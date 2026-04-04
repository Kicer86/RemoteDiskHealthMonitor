
#pragma once

#include <string>

#include "../IDiskCollector.h"
#include "IPartitionsManager.h"
#include "LsblkOutputParser.h"


class LinuxDiskCollector: public IDiskCollector, public IPartitionsManager
{
public:
    LinuxDiskCollector(const std::vector<LsblkOutputParser::LsblkEntry>& lsblkEntries);

    std::vector<Disk> GetDisksList() override;

    bool isPartition(const std::string& deviceName) const override;
    std::string diskForPartition(const std::string& deviceName) const override;

private:
    std::vector<LsblkOutputParser::LsblkEntry> m_lsblkEntries;
};
