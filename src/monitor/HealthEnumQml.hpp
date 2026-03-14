#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

#include "common/GeneralHealth.h"


namespace HealthEnum
{
    Q_NAMESPACE
    QML_ELEMENT

    enum Health
    {
        UNKNOWN      = static_cast<int>(GeneralHealth::UNKNOWN),
        GOOD         = static_cast<int>(GeneralHealth::GOOD),
        CHECK_STATUS = static_cast<int>(GeneralHealth::CHECK_STATUS),
        BAD          = static_cast<int>(GeneralHealth::BAD),
    };

    Q_ENUM_NS(Health)
}
