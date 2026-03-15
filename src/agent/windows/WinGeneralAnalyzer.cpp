#include "WinGeneralAnalyzer.h"
#include "CMDCommunication.h"


RefreshPolicy WinGeneralAnalyzer::GetRefreshPolicy() const
{
    return {std::chrono::hours(1), true};
}


void WinGeneralAnalyzer::Refresh(const std::vector<Disk>& disks)
{
    CMDCommunication reader;
    for (const auto& disk : disks)
        m_cachedStatus[disk.GetDeviceId()] = reader.CollectDiskStatus(disk);
}


GeneralHealth::Health WinGeneralAnalyzer::GetStatus(const Disk& _disk)
{
    auto it = m_cachedStatus.find(_disk.GetDeviceId());
    if (it != m_cachedStatus.end())
        return it->second;
    return GeneralHealth::UNKNOWN;
}

nlohmann::json WinGeneralAnalyzer::GetRawData(const Disk& _disk)
{
    return nlohmann::json{{"type", "text"}, {"value", std::string()}};
}
