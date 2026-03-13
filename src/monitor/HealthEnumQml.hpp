#pragma once

#include <QObject>

#include "common/GeneralHealth.h"


class HealthEnum
{
    Q_GADGET

public:
    enum Health
    {
        UNKNOWN      = static_cast<int>(GeneralHealth::UNKNOWN),
        GOOD         = static_cast<int>(GeneralHealth::GOOD),
        CHECK_STATUS = static_cast<int>(GeneralHealth::CHECK_STATUS),
        BAD          = static_cast<int>(GeneralHealth::BAD),
    };

    Q_ENUM(Health)
};
