
#include <regex>

#include "agent/OutputParsersUtils.h"
#include "DmesgParser.h"
#include "IPartitionsManager.h"


namespace
{
    const std::vector<std::regex> ErrorPatterns = {
        std::regex("Buffer I/O error on device ([a-z0-9]*)")
    };
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
                const std::string physicalDev = paritionsManager.isPartition(dev)?
                                            paritionsManager.diskForPartition(dev):
                                            dev;

                const Disk disk(physicalDev);

                errors[disk].insert(line);
            }
        }

    return errors;
}
