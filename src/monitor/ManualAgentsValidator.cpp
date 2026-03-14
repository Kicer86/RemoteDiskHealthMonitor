
#include "ManualAgentsValidator.hpp"


void ManualAgentsValidator::addNewAgent(const QString& name, const QString& ip, const QString& port)
{
    if (name.trimmed().isEmpty())
    {
        emit validationFailed(tr("Agent name cannot be empty"));
        return;
    }

    const auto segments = ip.split('.');
    bool ipValid = segments.size() == 4;
    if (ipValid)
    {
        for (const auto& segment : segments)
        {
            bool ok = false;
            const int number = segment.toInt(&ok);
            if (!ok || number < 0 || number > 255)
            {
                ipValid = false;
                break;
            }
        }
    }

    if (!ipValid)
    {
        emit validationFailed(tr("Invalid IP address"));
        return;
    }

    bool portOk = false;
    const int portNum = port.toInt(&portOk);
    if (!portOk || portNum < 1 || portNum > 65535)
    {
        emit validationFailed(tr("Port must be between 1 and 65535"));
        return;
    }

    const AgentInformation info(name.trimmed(), QHostAddress(ip), portNum, AgentInformation::DetectionSource::Hardcoded);
    emit agentDiscovered(info);
}
