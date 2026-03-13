
#include <regex>

#include "common/OutputParsersUtils.h"
#include "DmesgParser.h"
#include "IPartitionsManager.h"


namespace
{
    const std::vector<std::string> ErrorPatterns = {
        "Buffer I/O error on device ([a-z0-9]*)"
    };
}



std::map<Disk, std::set<std::string>> DmesgParser::parse(const std::string& output, const IPartitionsManager& paritionsManager)
{
    std::map<Disk, std::set<std::string>> errors;

    const auto lines = ParsersUtils::clean(output);

    for(const auto& line: lines)
        for(const auto& errorPattern: ErrorPatterns)
        {
            std::regex errorRegex(errorPattern);
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
