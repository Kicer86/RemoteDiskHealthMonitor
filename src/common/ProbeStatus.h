
#pragma once

#include <nlohmann/json.hpp>

#include "common/GeneralHealth.h"


struct ProbeStatus
{
    GeneralHealth::Health health;
    nlohmann::json rawData;

    bool operator==(const ProbeStatus& other) const = default;
};
