
#pragma once

#include <chrono>
#include <memory>

#include <QNetworkAccessManager>
#include <QHash>
#include <QTimer>

#include <cpp_restapi/iconnection.hpp>
#include <cpp_restapi/isse_connection.hpp>

#include "IAgentsStatusProvider.hpp"


class AgentsStatusProvider: public IAgentsStatusProvider
{
    Q_OBJECT

public:
    AgentsStatusProvider(QObject* parent = nullptr);
    void observe(const AgentInformation &) override;
    void unobserve(const AgentInformation &) override;

private:
    struct AgentConnection {
        std::shared_ptr<cpp_restapi::IConnection> connection;
        std::shared_ptr<cpp_restapi::ISseConnection> sseConnection;
        std::chrono::steady_clock::time_point lastEventTime{};
        std::chrono::steady_clock::time_point subscribedAt{};
        std::chrono::milliseconds reconnectDelay{std::chrono::seconds{1}};
        bool connected = false;
    };

    static constexpr std::chrono::milliseconds MaxReconnectDelay{std::chrono::seconds{30}};
    static constexpr std::chrono::milliseconds WatchdogInterval{std::chrono::seconds{15}};
    static constexpr std::chrono::seconds WatchdogTimeout{45};
    static constexpr std::chrono::seconds FirstEventGrace{30};

    QNetworkAccessManager m_nam;
    QHash<AgentInformation, AgentConnection> m_connections;
    QTimer m_watchdog;

    void fetchInitialStatus(const AgentInformation& info);
    void fetchAgentInfo(const AgentInformation& info);
    void connectSse(const AgentInformation& info);
    void handleSseEvent(const AgentInformation& info, const cpp_restapi::SseEvent& event);
    void parseStatusJson(const AgentInformation& info, const QByteArray& json);
    void scheduleSseReconnect(const AgentInformation& info);
    void checkConnections();
};
