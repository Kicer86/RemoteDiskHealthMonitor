
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cassert>

#include "agent/OutputParsersUtils.h"
#include "LsblkOutputParser.h"


namespace
{
    struct RawLsblkEntry
    {
        std::string name;
        std::string type;
        std::uint64_t size;
        int major;
        int minor;
    };

    std::vector<std::string> splitString(const std::string& str, char delim)
    {
        std::vector<std::string> parts;
        std::istringstream stream(str);
        std::string part;
        while (std::getline(stream, part, delim))
            parts.push_back(part);
        return parts;
    }

    RawLsblkEntry parseLine(const std::string& line)
    {
        const auto cols = splitString(line, ' ');
        assert(cols.size() >= 6);

        const auto major_minor = splitString(cols[1], ':');
        assert(major_minor.size() == 2);

        const RawLsblkEntry rawEntry{
            .name = cols[0],
            .type = cols[5],
            .size = std::stoull(cols[3]),
            .major = std::stoi(major_minor[0]),
            .minor = std::stoi(major_minor[1])
        };

        return rawEntry;
    }

    LsblkOutputParser::LsblkEntry fromRaw(const RawLsblkEntry& raw)
    {
        const LsblkOutputParser::LsblkEntry entry {
            .name = raw.name,
            .type = raw.type,
            .size = raw.size,
            .partitions = {},
            .major = raw.major,
            .minor = raw.minor
        };

        return entry;
    }
}


std::vector<LsblkOutputParser::LsblkEntry> LsblkOutputParser::parse(const std::string& output)
{
    std::vector<LsblkEntry> entries;

    auto lines = ParsersUtils::clean(output);

    if (!lines.empty())
        lines.erase(lines.begin());            // drop header

    for (const auto& line : lines)
    {
        if (line.empty()) continue;

        const RawLsblkEntry entry = parseLine(line);

        if (entry.type == "disk")
            entries.push_back(fromRaw(entry));
        else if (entry.type == "part")
        {
            auto entryIt = std::find_if(entries.begin(), entries.end(), [part_name = entry.name](const auto& disk) {
                return part_name.find(disk.name) == 0;
            });

            if (entryIt == entries.end())
                std::cerr << "Partition " << entry.name << " does not match any disk" << std::endl;
            else
                entryIt->partitions.insert(entry.name);
        }
        else if (entry.type.substr(0, 4) == "raid")
            entries.push_back(fromRaw(entry));
    }

    return entries;
}
