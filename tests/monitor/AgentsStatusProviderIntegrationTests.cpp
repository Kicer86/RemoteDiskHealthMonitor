#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QTest>
#include <QTimer>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "AgentsStatusProvider.hpp"
#include "common/JsonSerialize.h"
#include "common/constants.hpp"


namespace
{
    constexpr auto WaitTimeout = std::chrono::milliseconds{500};

    bool waitUntil(const std::function<bool()>& condition,
                   std::chrono::milliseconds timeout = WaitTimeout)
    {
        if (condition())
            return true;

        QEventLoop loop;
        QTimer pollTimer;
        pollTimer.setInterval(10);

        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);

        QObject::connect(&pollTimer, &QTimer::timeout, &loop, [&]() {
            if (condition())
                loop.quit();
        });
        QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);

        pollTimer.start();
        timeoutTimer.start(timeout);
        loop.exec();

        return condition();
    }

    DiskInfo disk(std::string name, GeneralHealth::Health health)
    {
        DiskSummary summary;
        summary.model = "Mock model";
        summary.vendor = "Mock vendor";
        summary.capacityBytes = 1024;
        summary.driveType = "SSD";

        DiskInfo info(std::move(name), health, {});
        info.SetSummary(summary);

        return info;
    }

    nlohmann::json statusJson(GeneralHealth::Health health,
                              const std::vector<DiskInfo>& disks,
                              std::string lastRefreshed)
    {
        return nlohmann::json{
            {"overallHealth", health},
            {"disks", disks},
            {"lastRefreshed", std::move(lastRefreshed)}
        };
    }

    nlohmann::json infoJson(std::string name, int protocol = static_cast<int>(VersionOfProtocol))
    {
        return nlohmann::json{
            {"name", std::move(name)},
            {"version", "test"},
            {"protocol", protocol}
        };
    }

    class MockAgentServer
    {
    public:
        MockAgentServer()
        {
            m_server.Get("/api/v1/info", [this](const httplib::Request&, httplib::Response& response) {
                ++m_infoRequests;
                response.set_content(handleInfoRequest().dump(), "application/json");
            });

            m_server.Get("/api/v1/events", [this](const httplib::Request&, httplib::Response& response) {
                ++m_sseSubscriptions;
                ++m_openSseSockets;

                auto lastSeenEventId = std::make_shared<int>(-1);
                auto initialSent = std::make_shared<bool>(false);

                response.set_header("Cache-Control", "no-cache");
                response.set_header("Connection", "keep-alive");
                response.set_chunked_content_provider(
                    "text/event-stream",
                    [this, lastSeenEventId, initialSent](size_t, httplib::DataSink& sink) {
                        std::unique_lock lock(m_mutex);

                        if (!*initialSent)
                        {
                            *initialSent = true;
                            *lastSeenEventId = m_eventId;
                            lock.unlock();
                            const std::string event = formatSseEvent(handleSseSubscription().dump());
                            sink.write(event.data(), event.size());
                            return true;
                        }

                        m_cv.wait_for(lock, std::chrono::milliseconds{100}, [&] {
                            return m_stopping || m_eventId != *lastSeenEventId;
                        });

                        if (m_stopping)
                            return false;

                        if (m_eventId == *lastSeenEventId)
                            return true;

                        *lastSeenEventId = m_eventId;
                        const std::string event = formatSseEvent(m_currentStatus.dump());
                        lock.unlock();
                        sink.write(event.data(), event.size());
                        return true;
                    },
                    [this](bool) {
                        --m_openSseSockets;
                    });
            });

            m_port = static_cast<quint16>(m_server.bind_to_any_port("127.0.0.1"));
            m_thread = std::thread([this] {
                m_server.listen_after_bind();
            });
            m_server.wait_until_ready();
        }

        ~MockAgentServer()
        {
            {
                std::lock_guard lock(m_mutex);
                m_stopping = true;
            }
            m_cv.notify_all();
            m_server.stop();
            if (m_thread.joinable())
                m_thread.join();
        }

        quint16 port() const
        {
            return m_port;
        }

        int infoRequests() const
        {
            return m_infoRequests.load();
        }

        int sseSubscriptions() const
        {
            return m_sseSubscriptions.load();
        }

        int openSseSockets() const
        {
            return m_openSseSockets.load();
        }

        void publishStatus(GeneralHealth::Health health,
                           const std::vector<DiskInfo>& disks,
                           std::string lastRefreshed)
        {
            {
                std::lock_guard lock(m_mutex);
                m_currentStatus = statusJson(health, disks, std::move(lastRefreshed));
                ++m_eventId;
            }
            m_cv.notify_all();
        }

        MOCK_METHOD(nlohmann::json, handleInfoRequest, ());
        MOCK_METHOD(nlohmann::json, handleSseSubscription, ());

    private:
        httplib::Server m_server;
        std::thread m_thread;
        quint16 m_port = 0;
        nlohmann::json m_currentStatus;
        std::atomic<int> m_infoRequests{0};
        std::atomic<int> m_sseSubscriptions{0};
        std::atomic<int> m_openSseSockets{0};
        std::mutex m_mutex;
        std::condition_variable m_cv;
        int m_eventId = 0;
        bool m_stopping = false;

        static std::string formatSseEvent(const std::string& data)
        {
            return "event: statusChanged\n" + std::string("data: ") + data + "\n\n";
        }
    };

    AgentInformation agentInfo(const QString& name, quint16 port)
    {
        return AgentInformation(name,
                                QHostAddress::LocalHost,
                                port,
                                AgentInformation::DetectionSource::Hardcoded);
    }

    struct AgentStatusProviderStatesHistory
    {
        std::vector<ConnectionState> connectionStates;
        std::vector<GeneralHealth::Health> statuses;
        std::vector<std::vector<DiskInfo>> diskCollections;
        QString lastRefreshed;
    };

    void connectProvider(AgentsStatusProvider& provider, const AgentInformation& info, AgentStatusProviderStatesHistory& statesHistory)
    {
        QObject::connect(&provider, &IAgentsStatusProvider::connectionStateChanged,
                        [&](const AgentInformation& changedAgent, ConnectionState state) {
                            if (changedAgent == info)
                                statesHistory.connectionStates.push_back(state);
                        });
        QObject::connect(&provider, &IAgentsStatusProvider::statusChanged,
                        [&](const AgentInformation& changedAgent, GeneralHealth::Health health) {
                            if (changedAgent == info)
                                statesHistory.statuses.push_back(health);
                        });
        QObject::connect(&provider, &IAgentsStatusProvider::diskCollectionChanged,
                        [&](const AgentInformation& changedAgent, const std::vector<DiskInfo>& disks) {
                            if (changedAgent == info)
                                statesHistory.diskCollections.push_back(disks);
                        });
        QObject::connect(&provider, &IAgentsStatusProvider::lastRefreshedChanged,
                        [&](const AgentInformation& changedAgent, const QString& timestamp) {
                            if (changedAgent == info)
                                statesHistory.lastRefreshed = timestamp;
                        });
    }
}


TEST(AgentsStatusProviderIntegrationTest, readsInitialAgentStateFromFirstSseEvent)
{
    MockAgentServer mockAgent;
    EXPECT_CALL(mockAgent, handleInfoRequest())
        .WillOnce(testing::Return(infoJson("alpha")));
    EXPECT_CALL(mockAgent, handleSseSubscription())
        .WillOnce(testing::Return(statusJson(GeneralHealth::GOOD,
                                             {disk("sda", GeneralHealth::GOOD)},
                                             "2026-05-04T10:00:00Z")));

    AgentsStatusProvider provider;
    const AgentInformation info = agentInfo("alpha", mockAgent.port());
    AgentStatusProviderStatesHistory history;

    connectProvider(provider, info, history);
    provider.observe(info);

    ASSERT_TRUE(waitUntil([&] { return !history.statuses.empty() && !history.diskCollections.empty(); }));
    ASSERT_TRUE(waitUntil([&] { return mockAgent.infoRequests() == 1 && mockAgent.sseSubscriptions() == 1; }));

    EXPECT_EQ(history.connectionStates.front(), ConnectionState::Connecting);
    EXPECT_EQ(history.connectionStates.back(), ConnectionState::Connected);
    EXPECT_EQ(history.statuses.back(), GeneralHealth::GOOD);
    ASSERT_EQ(history.diskCollections.back().size(), 1);
    EXPECT_EQ(history.diskCollections.back().front().GetName(), "sda");
    EXPECT_EQ(history.lastRefreshed, "2026-05-04T10:00:00Z");
}


TEST(AgentsStatusProviderIntegrationTest, reactsToSseStatusNotifications)
{
    MockAgentServer mockAgent;
    EXPECT_CALL(mockAgent, handleInfoRequest())
        .WillOnce(testing::Return(infoJson("alpha")));
    EXPECT_CALL(mockAgent, handleSseSubscription())
        .WillOnce(testing::Return(statusJson(GeneralHealth::GOOD,
                                             {disk("sda", GeneralHealth::GOOD)},
                                             "2026-05-04T10:00:00Z")));

    AgentsStatusProvider provider;
    const AgentInformation info = agentInfo("alpha", mockAgent.port());
    AgentStatusProviderStatesHistory history;

    connectProvider(provider, info, history);
    provider.observe(info);

    ASSERT_TRUE(waitUntil([&] { return history.statuses.size() == 1 && !history.diskCollections.empty(); }));

    mockAgent.publishStatus(GeneralHealth::BAD,
                            {disk("sdb", GeneralHealth::BAD), disk("nvme0n1", GeneralHealth::GOOD)},
                            "2026-05-04T10:05:00Z");

    ASSERT_TRUE(waitUntil([&] { return history.statuses.size() >= 2 && history.diskCollections.back().size() == 2; }));

    EXPECT_EQ(history.statuses.back(), GeneralHealth::BAD);
    EXPECT_EQ(history.diskCollections.back().at(0).GetName(), "sdb");
    EXPECT_EQ(history.diskCollections.back().at(1).GetName(), "nvme0n1");
    EXPECT_EQ(history.lastRefreshed, "2026-05-04T10:05:00Z");
}


TEST(AgentsStatusProviderIntegrationTest, keepsMultipleAgentsSeparated)
{
    MockAgentServer alpha;
    MockAgentServer beta;
    EXPECT_CALL(alpha, handleInfoRequest())
        .WillOnce(testing::Return(infoJson("alpha")));
    EXPECT_CALL(alpha, handleSseSubscription())
        .WillOnce(testing::Return(statusJson(GeneralHealth::GOOD,
                                             {disk("sda", GeneralHealth::GOOD)},
                                             "2026-05-04T10:00:00Z")));
    EXPECT_CALL(beta, handleInfoRequest())
        .WillOnce(testing::Return(infoJson("beta")));
    EXPECT_CALL(beta, handleSseSubscription())
        .WillOnce(testing::Return(statusJson(GeneralHealth::BAD,
                                             {disk("sdb", GeneralHealth::BAD)},
                                             "2026-05-04T10:00:00Z")));

    AgentsStatusProvider provider;

    const AgentInformation alphaInfo = agentInfo("alpha", alpha.port());
    const AgentInformation betaInfo = agentInfo("beta", beta.port());

    QHash<AgentInformation, GeneralHealth::Health> statuses;
    QHash<AgentInformation, QStringList> diskNames;

    QObject::connect(&provider, &IAgentsStatusProvider::statusChanged,
                     [&](const AgentInformation& agent, GeneralHealth::Health health) {
                         statuses[agent] = health;
                     });
    QObject::connect(&provider, &IAgentsStatusProvider::diskCollectionChanged,
                     [&](const AgentInformation& agent, const std::vector<DiskInfo>& disks) {
                         QStringList names;
                         for (const auto& diskInfo : disks)
                             names.append(QString::fromStdString(diskInfo.GetName()));
                         diskNames[agent] = names;
                     });

    provider.observe(alphaInfo);
    provider.observe(betaInfo);

    ASSERT_TRUE(waitUntil([&] {
        return statuses.contains(alphaInfo) && statuses.contains(betaInfo)
            && diskNames.contains(alphaInfo) && diskNames.contains(betaInfo);
    }));

    EXPECT_EQ(statuses[alphaInfo], GeneralHealth::GOOD);
    EXPECT_EQ(statuses[betaInfo], GeneralHealth::BAD);
    EXPECT_EQ(diskNames[alphaInfo], QStringList{"sda"});
    EXPECT_EQ(diskNames[betaInfo], QStringList{"sdb"});

    beta.publishStatus(GeneralHealth::CHECK_STATUS,
                       {disk("sdc", GeneralHealth::CHECK_STATUS)},
                       "2026-05-04T10:10:00Z");

    ASSERT_TRUE(waitUntil([&] {
        return statuses[betaInfo] == GeneralHealth::CHECK_STATUS
            && diskNames[betaInfo] == QStringList{"sdc"};
    }));

    EXPECT_EQ(statuses[alphaInfo], GeneralHealth::GOOD);
    EXPECT_EQ(diskNames[alphaInfo], QStringList{"sda"});
}


TEST(AgentsStatusProviderIntegrationTest, reportsProtocolMismatchFromInfoEndpoint)
{
    const int unsupportedProtocol = static_cast<int>(VersionOfProtocol) + 1;
    MockAgentServer mockAgent;
    EXPECT_CALL(mockAgent, handleInfoRequest())
        .WillOnce(testing::Return(infoJson("alpha", unsupportedProtocol)));
    EXPECT_CALL(mockAgent, handleSseSubscription())
        .WillOnce(testing::Return(statusJson(GeneralHealth::GOOD,
                                             {disk("sda", GeneralHealth::GOOD)},
                                             "2026-05-04T10:00:00Z")));

    AgentsStatusProvider provider;
    const AgentInformation info = agentInfo("alpha", mockAgent.port());

    int reportedAgentVersion = 0;
    int reportedMonitorVersion = 0;

    QObject::connect(&provider, &IAgentsStatusProvider::protocolMismatch,
                     [&](const AgentInformation& changedAgent, int agentVersion, int monitorVersion) {
                         if (changedAgent == info)
                         {
                             reportedAgentVersion = agentVersion;
                             reportedMonitorVersion = monitorVersion;
                         }
                     });

    provider.observe(info);

    ASSERT_TRUE(waitUntil([&] { return reportedAgentVersion != 0; }));

    EXPECT_EQ(reportedAgentVersion, unsupportedProtocol);
    EXPECT_EQ(reportedMonitorVersion, static_cast<int>(VersionOfProtocol));
}


TEST(AgentsStatusProviderIntegrationTest, unobserveClosesSseAndIgnoresLaterAgentNotifications)
{
    MockAgentServer mockAgent;
    EXPECT_CALL(mockAgent, handleInfoRequest())
        .WillOnce(testing::Return(infoJson("alpha")));
    EXPECT_CALL(mockAgent, handleSseSubscription())
        .WillOnce(testing::Return(statusJson(GeneralHealth::GOOD,
                                             {disk("sda", GeneralHealth::GOOD)},
                                             "2026-05-04T10:00:00Z")));

    AgentsStatusProvider provider;
    const AgentInformation info = agentInfo("alpha", mockAgent.port());

    std::vector<GeneralHealth::Health> statuses;

    QObject::connect(&provider, &IAgentsStatusProvider::statusChanged,
                     [&](const AgentInformation& changedAgent, GeneralHealth::Health health) {
                         if (changedAgent == info)
                             statuses.push_back(health);
                     });

    provider.observe(info);

    ASSERT_TRUE(waitUntil([&] { return statuses.size() == 1 && mockAgent.openSseSockets() == 1; }));

    provider.unobserve(info);

    ASSERT_TRUE(waitUntil([&] { return mockAgent.openSseSockets() == 0; }));

    mockAgent.publishStatus(GeneralHealth::BAD, {disk("sdb", GeneralHealth::BAD)}, "2026-05-04T10:15:00Z");
    QTest::qWait(100);

    ASSERT_EQ(statuses.size(), 1);
    EXPECT_EQ(statuses.back(), GeneralHealth::GOOD);
}


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
