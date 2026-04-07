
#pragma once

#include <memory>
#include <set>
#include <string>
#include <map>

#include "../IProbe.h"


class IPartitionsManager;

class LinGeneralAnalyzer : public IProbe
{
public:
    LinGeneralAnalyzer(std::shared_ptr<IPartitionsManager>);

    RefreshPolicy GetRefreshPolicy() const override;
    void Refresh(const std::vector<Disk>& disks) override;

    GeneralHealth::Health GetStatus(const Disk& disk) const override;
    nlohmann::json GetRawData(const Disk& disk) const override;

private:
    std::map<Disk, std::set<std::string>> m_errors;
    std::shared_ptr<IPartitionsManager> m_partitionsManager;
    bool m_useJournalctl = false;
};
