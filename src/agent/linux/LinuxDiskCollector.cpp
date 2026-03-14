
#include "LinuxDiskCollector.h"
#include "LsblkOutputParser.h"

#include <fstream>
#include <algorithm>


namespace
{
    std::string detectDriveType(const std::string& name)
    {
        if (name.find("nvme") == 0)
            return "NVMe";

        std::ifstream rotFile("/sys/block/" + name + "/queue/rotational");
        if (rotFile.is_open())
        {
            int rotational = -1;
            rotFile >> rotational;
            if (rotational == 1)
                return "HDD";
            if (rotational == 0)
                return "SSD";
        }

        return {};
    }
}


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

        const auto driveType = detectDriveType(entry.name);
        disks.emplace_back(entry.name, model, entry.size, driveType);
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

