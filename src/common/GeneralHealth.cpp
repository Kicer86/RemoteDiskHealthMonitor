#include "GeneralHealth.h"


std::string healthToString(GeneralHealth::Health health)
{
    switch (health)
    {
        case GeneralHealth::UNKNOWN:      return "UNKNOWN";
        case GeneralHealth::GOOD:         return "GOOD";
        case GeneralHealth::CHECK_STATUS: return "CHECK_STATUS";
        case GeneralHealth::BAD:          return "BAD";
    }
    return "UNKNOWN";
}


GeneralHealth::Health healthFromString(const std::string& str)
{
    if (str == "GOOD")         return GeneralHealth::GOOD;
    if (str == "CHECK_STATUS") return GeneralHealth::CHECK_STATUS;
    if (str == "BAD")          return GeneralHealth::BAD;
    return GeneralHealth::UNKNOWN;
}
