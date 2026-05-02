#pragma once

#include <map>
#include <string>

#include "../IProbe.h"

class WinGeneralAnalyzer : public IProbe
{
public:
    RefreshPolicy GetRefreshPolicy() const override;
    void Refresh(const std::vector<Disk>& disks) override;

    GeneralHealth::Health GetStatus(const Disk& _disk) const override;
    nlohmann::json GetRawData(const Disk& _disk) const override;

private:
    std::map<std::string, GeneralHealth::Health> m_cachedStatus;
};
