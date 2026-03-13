
#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHash>
#include <QTimer>

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
        QNetworkReply* sseReply = nullptr;
        QByteArray sseBuffer;
        int reconnectDelayMs = 1000;
    };

    static constexpr int MaxReconnectDelayMs = 30000;

    QNetworkAccessManager m_nam;
    QHash<AgentInformation, AgentConnection> m_connections;

    void fetchInitialStatus(const AgentInformation& info);
    void connectSse(const AgentInformation& info);
    void processSseData(const AgentInformation& info);
    void parseStatusJson(const AgentInformation& info, const QByteArray& json);
    void scheduleSseReconnect(const AgentInformation& info);
};
