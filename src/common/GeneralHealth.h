#pragma once

#include <cstdint>
#include <string>


namespace GeneralHealth
{
    enum Health : uint8_t
    {
        UNKNOWN = 0,
        GOOD,
        CHECK_STATUS,
        BAD
    };
}

std::string healthToString(GeneralHealth::Health health);
GeneralHealth::Health healthFromString(const std::string& str);
