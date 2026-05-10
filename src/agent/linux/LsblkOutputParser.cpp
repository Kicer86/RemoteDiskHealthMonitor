
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <optional>

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

    std::vector<std::string> splitByWhitespace(const std::string& str)
    {
        std::vector<std::string> parts;
        std::istringstream stream(str);
        std::string part;

        while (stream >> part)
            parts.push_back(part);

        return parts;
    }

    std::vector<std::string> splitString(const std::string& str, char delim)
    {
        std::vector<std::string> parts;
        std::istringstream stream(str);
        std::string part;

        while (std::getline(stream, part, delim))
            parts.push_back(part);

        return parts;
    }

    std::optional<RawLsblkEntry> parseLine(const std::string& line)
    {
        const auto cols = splitByWhitespace(line);
        if (cols.size() < 6)
            return std::nullopt;

        const auto major_minor = splitString(cols[1], ':');
        if (major_minor.size() != 2)
            return std::nullopt;

        try
        {
            return RawLsblkEntry{
                .name = cols[0],
                .type = cols[5],
                .size = std::stoull(cols[3]),
                .major = std::stoi(major_minor[0]),
                .minor = std::stoi(major_minor[1])
            };
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }

    bool isPartitionOfDisk(const std::string& partitionName,
                           const std::string& diskName)
    {
        if (partitionName.size() <= diskName.size() || partitionName.find(diskName) != 0)
            return false;

        const auto suffix = partitionName.substr(diskName.size());

        if (!suffix.empty() && std::isdigit(static_cast<unsigned char>(suffix.front())))
            return true;

        return suffix.size() > 1 &&
               suffix.front() == 'p' &&
               std::isdigit(static_cast<unsigned char>(suffix[1]));
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

        const auto parsedEntry = parseLine(line);
        if (!parsedEntry)
        {
            std::cerr << "Skipping malformed lsblk line: " << line << std::endl;
            continue;
        }

        const RawLsblkEntry& entry = *parsedEntry;

        if (entry.type == "disk")
            entries.push_back(fromRaw(entry));
        else if (entry.type == "part")
        {
            auto entryIt = std::find_if(entries.begin(), entries.end(), [part_name = entry.name](const auto& disk) {
                return isPartitionOfDisk(part_name, disk.name);
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
