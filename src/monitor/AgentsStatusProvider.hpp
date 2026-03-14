
#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHash>

#include "IAgentsStatusProvider.hpp"


class AgentsStatusProvider: public IAgentsStatusProvider
{
    Q_OBJECT

public:
    AgentsStatusProvider(QObject* parent = nullptr);
    void observe(const AgentInformation &) override;

private:
    struct AgentConnection {
        QNetworkReply* sseReply = nullptr;
        QByteArray sseBuffer;
    };

    QNetworkAccessManager m_nam;
    QHash<AgentInformation, AgentConnection> m_connections;

    void fetchInitialStatus(const AgentInformation& info);
    void connectSse(const AgentInformation& info);
    void processSseData(const AgentInformation& info);
    void parseStatusJson(const AgentInformation& info, const QByteArray& json);
};
