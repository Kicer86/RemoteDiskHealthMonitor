#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

#include "common/GeneralHealth.h"


class HealthEnum
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Access to enum")

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
