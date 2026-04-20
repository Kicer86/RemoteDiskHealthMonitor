
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
        int reconnectDelayMs = 1000;
        bool connected = false;
    };

    static constexpr int MaxReconnectDelayMs = 30000;
    static constexpr std::chrono::milliseconds WatchdogInterval{15000};
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
