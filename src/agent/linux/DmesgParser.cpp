
#include <regex>

#include "agent/OutputParsersUtils.h"
#include "DmesgParser.h"
#include "IPartitionsManager.h"


namespace
{
    const std::vector<std::regex> ErrorPatterns = {
        std::regex(R"(Buffer I/O error on (?:dev(?:ice)? )?([A-Za-z0-9_.-]+))"),
        std::regex(R"((?:I/O error|critical medium error|medium error|uncorrectable error).*dev ([A-Za-z0-9_.-]+))"),
        std::regex(R"(EXT[234]-fs error.*\(device ([A-Za-z0-9_.-]+)\))")
    };

    std::string physicalDeviceFor(const std::string& deviceName,
                                  const IPartitionsManager& partitionsManager)
    {
        if (partitionsManager.isPartition(deviceName))
        {
            const auto physicalDevice = partitionsManager.diskForPartition(deviceName);
            return physicalDevice.empty() ? deviceName : physicalDevice;
        }
        else
            return deviceName;
    }
}


std::map<Disk, std::set<std::string>> DmesgParser::parse(const std::string& output, const IPartitionsManager& paritionsManager)
{
    std::map<Disk, std::set<std::string>> errors;

    const auto lines = ParsersUtils::clean(output);

    for(const auto& line: lines)
        for(const auto& errorRegex: ErrorPatterns)
        {
            std::smatch errorMatch;

            if (std::regex_search(line, errorMatch, errorRegex))
            {
                const std::string dev = errorMatch[1].str();
                const std::string physicalDev = physicalDeviceFor(dev, paritionsManager);

                const Disk disk(physicalDev);

                errors[disk].insert(line);
            }
        }

    return errors;
}
