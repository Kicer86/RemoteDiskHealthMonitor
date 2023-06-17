#pragma once

#include <iostream>
#include <variant>
#include "common/GeneralHealth.h"
#include "agent/Disk.h"


class IProbe
{
public:
    virtual ~IProbe() = default;
    virtual GeneralHealth::Health GetStatus(const Disk& _disk) = 0;
    virtual QString GetJSonData(const Disk& _disk) = 0;
};
