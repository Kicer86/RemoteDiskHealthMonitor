
#pragma once

#include <QRemoteObjectHost>

#include "common/GeneralHealth.h"
#include "common/constants.hpp"
#include "rep_AgentStatus_source.h"
#include "common/DiskInfo.h"


class Server : public AgentStatusSource
{
    Q_OBJECT

public:
    Server(QObject* parent = nullptr);
    bool Init();

    const QString& ip() const;
    quint16 port() const;

    void setOverallStatus(GeneralHealth::Health overallStatus) override;
    void setDiskInfoCollection(QJsonDocument diskInfoCollection) override;
    GeneralHealth::Health overallStatus() const override;
    QJsonDocument diskInfoCollection() const override;
    void refresh() override;

private:
    void CollectInfoAboutDiscs();

    GeneralHealth::Health m_health;
    QJsonDocument m_diskInfoCollection;

    QRemoteObjectHost m_ROHost;
    QString m_ip;
};
