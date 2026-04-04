#pragma once

#include <nlohmann/json.hpp>

#include "common/GeneralHealth.h"
#include "agent/Disk.h"


class IProbe
{
public:
    virtual ~IProbe() = default;
    virtual GeneralHealth::Health GetStatus(const Disk& _disk) = 0;
    virtual nlohmann::json GetRawData(const Disk& _disk) = 0;
};