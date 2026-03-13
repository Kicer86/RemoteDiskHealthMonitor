
#pragma once

#include <QObject>
#include <vector>

#include "AgentInformation.hpp"
#include "common/GeneralHealth.h"
#include "common/DiskInfo.h"


enum class ConnectionState
{
    Connecting,
    Connected,
    Disconnected,
    Error,
};


class IAgentsStatusProvider: public QObject
{
        Q_OBJECT

    public:
        virtual ~IAgentsStatusProvider() = default;

        virtual void observe(const AgentInformation &) = 0;
        virtual void unobserve(const AgentInformation &) = 0;

    signals:
        void statusChanged(const AgentInformation &, const GeneralHealth::Health &);
        void diskCollectionChanged(const AgentInformation&, const std::vector<DiskInfo> &);
        void connectionStateChanged(const AgentInformation &, ConnectionState);
        void protocolMismatch(const AgentInformation &, int agentVersion, int monitorVersion);

};
