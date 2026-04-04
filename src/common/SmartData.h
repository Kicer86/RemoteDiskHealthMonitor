#pragma once

#include <cstdint>
#include <string>
#include <vector>

class SmartData
{
public:
    struct Attribute
    {
        uint8_t id = 0;
        std::string name;
        int value = 0;
        int worst = 0;
        int threshold = 0;
        int64_t rawVal = 0;

        bool operator==(const Attribute&) const = default;
    };

    std::vector<Attribute> attributes;

    bool operator==(const SmartData&) const = default;

    static std::string GetCanonicalName(uint8_t id);
};
