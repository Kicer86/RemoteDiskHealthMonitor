
#pragma once

#include "agent/IProbe.h"


struct ProbeStatus
{
    GeneralHealth::Health health;
    QString jsonData;

    auto operator<=>(const ProbeStatus &) const = default;
};
