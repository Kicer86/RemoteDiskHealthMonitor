#pragma once

#include <gmock/gmock.h>

#include "IProbe.h"

class IProbeMock : public IProbe
{
public:
    MOCK_METHOD(GeneralHealth::Health, GetStatus, (const Disk&), (override));
    MOCK_METHOD(nlohmann::json, GetRawData, (const Disk&), (override));
};