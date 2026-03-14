#include "DiskInfo.h"



DiskInfo::DiskInfo()
{
}

DiskInfo::DiskInfo(std::string _name, const GeneralHealth::Health& _health, const std::vector<ProbeStatus>& _statuses)
    : m_name(std::move(_name)), m_health(_health), m_statuses(_statuses)
{
}

void DiskInfo::SetName(const std::string& _name)
{
    m_name = _name;
}

void DiskInfo::SetProbesStatuses(const std::vector<ProbeStatus>& statuses)
{
    m_statuses = statuses;
}


const std::string& DiskInfo::GetName() const
{
    return m_name;
};

void DiskInfo::SetHealth(const GeneralHealth::Health& _health)
{
    m_health = _health;
}

GeneralHealth::Health DiskInfo::GetHealth() const
{
    return m_health;
}

const std::vector<ProbeStatus> & DiskInfo::GetProbesStatuses() const
{
    return m_statuses;
}

void DiskInfo::SetSummary(const DiskSummary& summary)
{
    m_summary = summary;
}

const DiskSummary& DiskInfo::GetSummary() const
{
    return m_summary;
}
