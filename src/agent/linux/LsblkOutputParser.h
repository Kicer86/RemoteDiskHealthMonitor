
#pragma once

#include <set>
#include <string>
#include <vector>
#include <tuple>
#include <cstdint>

class LsblkOutputParser
{
public:
    struct LsblkEntry
    {
        std::string name;
        std::string type;
        std::uint64_t size;
        std::set<std::string> partitions;
        int major;
        int minor;

        friend auto operator==(const LsblkEntry& lhs, const LsblkEntry& rhs)
        {
            return std::tie(lhs.name, lhs.type, lhs.size, lhs.partitions, lhs.major, lhs.minor) ==
                   std::tie(rhs.name, rhs.type, rhs.size, rhs.partitions, rhs.major, rhs.minor);
        }
    };

    static std::vector<LsblkEntry> parse(const std::string& output);
};
