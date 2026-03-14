
#include "LinuxDiskCollector.h"
#include "LsblkOutputParser.h"

#include <fstream>
#include <algorithm>


LinuxDiskCollector::LinuxDiskCollector(const std::vector<LsblkOutputParser::LsblkEntry>& lsblkEntries)
    : m_lsblkEntries(lsblkEntries)
{

}


std::vector<Disk> LinuxDiskCollector::GetDisksList()
{
    std::vector<Disk> disks;

    for (const auto& entry: m_lsblkEntries)
    {
        std::string model;
        std::ifstream modelFile("/sys/block/" + entry.name + "/device/model");
        if (modelFile.is_open())
        {
            std::getline(modelFile, model);
            // trim trailing whitespace
            while (!model.empty() && (model.back() == ' ' || model.back() == '\t'))
                model.pop_back();
        }

        disks.emplace_back(entry.name, model);
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

