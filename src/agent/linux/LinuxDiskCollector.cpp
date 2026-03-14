
#include "LinuxDiskCollector.h"
#include "LsblkOutputParser.h"


LinuxDiskCollector::LinuxDiskCollector(const std::vector<LsblkOutputParser::LsblkEntry>& lsblkEntries)
    : m_lsblkEntries(lsblkEntries)
{

}


std::vector<Disk> LinuxDiskCollector::GetDisksList()
{
    std::vector<Disk> disks;

    for (const auto& entry: m_lsblkEntries)
    {
        const Disk disk(entry.name);

        disks.push_back(disk);
    }

    return disks;
}


bool LinuxDiskCollector::isPartition(const std::string& deviceName) const
{
    for(const auto& entry: m_lsblkEntries)
        for(const auto& partitionDevice: entry.partitions)
            if (partitionDevice == deviceName)
                return true;

    return false;
}


std::string LinuxDiskCollector::diskForPartition(const std::string& deviceName) const
{
    for(const auto& entry: m_lsblkEntries)
        for(const auto& partitionDevice: entry.partitions)
            if (partitionDevice == deviceName)
                return entry.name;

    return {};
}

