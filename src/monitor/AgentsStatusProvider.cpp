
#include <iostream>

#include <QAbstractSocket>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QUrl>

#include <cpp_restapi/create_qt_connection.hpp>

#include "AgentsStatusProvider.hpp"
#include "common/JsonSerialize.h"
#include "common/constants.hpp"


namespace
{
    QString formatHost(const QHostAddress& host)
    {
        const QString s = host.toString();
        return host.protocol() == QAbstractSocket::IPv6Protocol
            ? QStringLiteral("[%1]").arg(s)
            : s;
    }
}


AgentsStatusProvider::AgentsStatusProvider(QObject* parent)
    : IAgentsStatusProvider()
{
    Q_UNUSED(parent);
    m_watchdog.setInterval(WatchdogInterval);
    connect(&m_watchdog, &QTimer::timeout, this, &AgentsStatusProvider::checkConnections);
    m_watchdog.start();
}


void AgentsStatusProvider::observe(const AgentInformation& info)
{
    if (m_connections.contains(info))
        return;

    const std::string address = QStringLiteral("http://%1:%2")
        .arg(formatHost(info.host()))
        .arg(info.port())
        .toStdString();

    AgentConnection agentConn;
    agentConn.connection = cpp_restapi::createQtConnection(m_nam, address, {});

    m_connections.insert(info, std::move(agentConn));

    emit connectionStateChanged(info, ConnectionState::Connecting);

    // SSE connect triggers agent-side conditional refresh;
    // the first SSE event carries initial status data
    connectSse(info);
    fetchAgentInfo(info);
}


void AgentsStatusProvider::unobserve(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    if (it->sseConnection)
        it->sseConnection->close();

    m_connections.erase(it);
}


void AgentsStatusProvider::parseStatusJson(const AgentInformation& info, std::string_view json)
{
    try
    {
        nlohmann::json j = nlohmann::json::parse(json);

        if (j.contains("overallHealth"))
        {
            GeneralHealth::Health health = j.at("overallHealth").get<GeneralHealth::Health>();
            emit statusChanged(info, health);
        }

        if (j.contains("disks"))
        {
            std::vector<DiskInfo> disks = j.at("disks").get<std::vector<DiskInfo>>();
            emit diskCollectionChanged(info, disks);
        }

        if (j.contains("lastRefreshed"))
        {
            QString ts = QString::fromStdString(j.at("lastRefreshed").get<std::string>());
            emit lastRefreshedChanged(info, ts);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error for agent " << info.name().toStdString()
                  << ": " << e.what() << "\n";
    }
}


void AgentsStatusProvider::fetchAgentInfo(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end() || !it->connection)
        return;

    QPointer<AgentsStatusProvider> self(this);
    it->connection->fetch("api/v1/info",
        [self, info](cpp_restapi::Response response)
        {
            if (!self)
                return;

            try
            {
                nlohmann::json j = nlohmann::json::parse(response.body);
                if (j.contains("protocol"))
                {
                    const int agentVersion = j.at("protocol").get<int>();
                    const int monitorVersion = static_cast<int>(VersionOfProtocol);
                    if (agentVersion != monitorVersion)
                        emit self->protocolMismatch(info, agentVersion, monitorVersion);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Failed to parse agent info from " << info.name().toStdString()
                          << ": " << e.what() << "\n";
            }
        },
        [info](cpp_restapi::HttpError error)
        {
            std::cerr << "Failed to fetch agent info from " << info.name().toStdString()
                      << ": " << error.message << "\n";
        });
}


void AgentsStatusProvider::connectSse(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end() || !it->connection)
        return;

    it->subscribedAt = std::chrono::steady_clock::now();

    it->sseConnection = it->connection->subscribe("api/v1/events",
        [self = QPointer<AgentsStatusProvider>(this), info](const cpp_restapi::SseEvent& event) {
            if (!self)
                return;
            self->handleSseEvent(info, event);
        });
}


void AgentsStatusProvider::handleSseEvent(const AgentInformation& info, const cpp_restapi::SseEvent& event)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    it->reconnectDelay = std::chrono::milliseconds{1000};
    it->lastEventTime = std::chrono::steady_clock::now();
    it->connected = true;

    emit connectionStateChanged(info, ConnectionState::Connected);

    parseStatusJson(info, event.data);
}


void AgentsStatusProvider::scheduleSseReconnect(const AgentInformation& info)
{
    auto it = m_connections.find(info);
    if (it == m_connections.end())
        return;

    const std::chrono::milliseconds delay = it->reconnectDelay;
    it->reconnectDelay = std::min(it->reconnectDelay * 2, MaxReconnectDelay);

    QTimer::singleShot(delay, this, [self = QPointer<AgentsStatusProvider>(this), info]() {
        if (self && self->m_connections.contains(info))
            self->connectSse(info);
    });
}


void AgentsStatusProvider::checkConnections()
{
    const auto now = std::chrono::steady_clock::now();

    for (auto&& [info, conn] : m_connections.asKeyValueRange())
    {
        const bool eventTimedOut =
            conn.connected && (now - conn.lastEventTime) > WatchdogTimeout;
        const bool firstEventNeverArrived =
            !conn.connected && conn.sseConnection
            && conn.subscribedAt != std::chrono::steady_clock::time_point{}
            && (now - conn.subscribedAt) > FirstEventGrace;

        if (eventTimedOut || firstEventNeverArrived)
        {
            conn.connected = false;

            if (conn.sseConnection)
                conn.sseConnection->close();

            emit connectionStateChanged(info, ConnectionState::Disconnected);
            scheduleSseReconnect(info);
        }
    }
}
