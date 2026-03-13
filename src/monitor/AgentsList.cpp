
#include <chrono>
#include <iterator>
#include <QDebug>

#include <nlohmann/json.hpp>

#include "AgentsList.hpp"


using namespace std::placeholders;
using namespace std::chrono_literals;


AgentsList::AgentsList(IAgentsStatusProvider& statusProvider, QObject* p)
    : QAbstractListModel(p)
    , m_statusProvider(statusProvider)
{
    connect(&m_statusProvider, &IAgentsStatusProvider::statusChanged, this, &AgentsList::updateAgentHealth);
    connect(&m_statusProvider, &IAgentsStatusProvider::diskCollectionChanged, this, &AgentsList::updateAgentDiskInfoCollection);
}


void AgentsList::addAgent(const AgentInformation& info)
{
    auto it = std::find(m_agents.begin(), m_agents.end(), info);

    // we do not need duplicates
    if (it == m_agents.end())
    {
        beginInsertRows({}, m_agents.size(), m_agents.size());
        m_agents.append(info);
        endInsertRows();

        m_statusProvider.observe(info);
    }
}


void AgentsList::removeAgent(const AgentInformation& info)
{
    auto it = std::find(m_agents.begin(), m_agents.end(), info);

    if (it != m_agents.end())
    {
        const int pos = std::distance(m_agents.begin(), it);

       removeAgentAt(pos);
    }
}


void AgentsList::removeAgentAt(int position)
{
    const AgentInformation info = m_agents[position];

    beginRemoveRows({}, position, position);
    m_agents.removeAt(position);
    m_health.remove(info);
    m_diskInfoCollection.remove(info);
    endRemoveRows();

    m_statusProvider.unobserve(info);
}


const QVector<AgentInformation>& AgentsList::agents() const
{
    return m_agents;
}


int AgentsList::rowCount(const QModelIndex& parent) const
{
    return parent.isValid()? 0: m_agents.size();
}


QVariant AgentsList::data(const QModelIndex& index, int role) const
{
    QVariant result;

    if (index.column() == 0 && index.row() < m_agents.size())
    {
        const int row = index.row();

        if (role == AgentNameRole)
            result = m_agents[row].name();
        else if (role == AgentHealthRole)
        {
            auto it = m_health.find(m_agents[row]);

            result = it == m_health.end()? GeneralHealth::UNKNOWN: it.value();
        }
        else if (role == AgentDetectionTypeRole)
        {
            result = static_cast<int>(m_agents[row].detectionSource());
        }
        else if (role == AgentDiskInfoNamesRole)
        {
            QStringList names;
            auto it = m_diskInfoCollection.find(m_agents[row]);
            if (it != m_diskInfoCollection.end())
            {
                auto diskInfoVec = it.value();
                for (auto item : diskInfoVec)
                {
                    names.append(QString::fromStdString(item.GetName()));
                }
            }
            result = names;
        }
        else if (role == AgentDiskInfoDataRole)
        {
            QStringList diskJsonList;
            auto it = m_diskInfoCollection.find(m_agents[row]);
            if (it != m_diskInfoCollection.end())
            {
                const auto& diskInfoVec = it.value();
                for (const auto& disk : diskInfoVec)
                {
                    nlohmann::json diskJson;
                    diskJson["name"] = disk.GetName();
                    diskJson["health"] = disk.GetHealth();

                    nlohmann::json probesJson = nlohmann::json::array();
                    for (const auto& probe : disk.GetProbesStatuses())
                    {
                        nlohmann::json probeJson;
                        probeJson["health"] = probe.health;

                        if (std::holds_alternative<SmartData>(probe.rawData))
                        {
                            const auto& smart = std::get<SmartData>(probe.rawData);
                            probeJson["type"] = "smart";
                            nlohmann::json attrs = nlohmann::json::array();
                            for (const auto& [attr, data] : smart.smartData)
                            {
                                attrs.push_back({
                                    {"name", SmartData::GetAttrTypeName(attr)},
                                    {"value", data.value},
                                    {"worst", data.worst},
                                    {"rawVal", data.rawVal}
                                });
                            }
                            probeJson["attributes"] = attrs;
                        }
                        else
                        {
                            probeJson["type"] = "text";
                            probeJson["text"] = std::get<std::string>(probe.rawData);
                        }
                        probesJson.push_back(probeJson);
                    }
                    diskJson["probes"] = probesJson;
                    diskJsonList.append(QString::fromStdString(diskJson.dump()));
                }
            }
            result = diskJsonList;
        }
    }

    return result;
}


QHash<int, QByteArray> AgentsList::roleNames() const
{
    auto existingRoles = QAbstractListModel::roleNames();
    existingRoles.insert(AgentNameRole, "agentName");
    existingRoles.insert(AgentHealthRole, "agentHealth");
    existingRoles.insert(AgentDetectionTypeRole, "agentDetectionType");
    existingRoles.insert(AgentDiskInfoNamesRole, "agentDiskInfoNames");
    existingRoles.insert(AgentDiskInfoDataRole, "agentDiskInfoData");


    return existingRoles;
}


void AgentsList::updateAgentHealth(const AgentInformation& info, const GeneralHealth::Health& health)
{
    auto it = std::find(m_agents.begin(), m_agents.end(), info);

    if (it != m_agents.end())
    {
        m_health[info] = health;

        const int pos = std::distance(m_agents.begin(), it);
        const QModelIndex idx = index(pos, 0);

        emit dataChanged(idx, idx, {AgentHealthRole});
    }
}

void AgentsList::updateAgentDiskInfoCollection(const AgentInformation& _info, const std::vector<DiskInfo>& _diskInfoCollection)
{
    auto it = std::find(m_agents.begin(), m_agents.end(), _info);

    if (it != m_agents.end())
    {
        m_diskInfoCollection[_info] = _diskInfoCollection;

        const int pos = std::distance(m_agents.begin(), it);
        const QModelIndex idx = index(pos, 0);

        emit dataChanged(idx, idx, { AgentDiskInfoNamesRole });
        emit dataChanged(idx, idx, { AgentDiskInfoDataRole });
    }
}
