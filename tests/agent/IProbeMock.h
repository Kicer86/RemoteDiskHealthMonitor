#pragma once

#include <gmock/gmock.h>

#include "IProbe.h"

class IProbeMock : public IProbe
{
public:
    MOCK_METHOD(RefreshPolicy, GetRefreshPolicy, (), (const, override));
    MOCK_METHOD(void, Refresh, (const std::vector<Disk>&), (override));
    MOCK_METHOD(GeneralHealth::Health, GetStatus, (const Disk&), (const, override));
    MOCK_METHOD(nlohmann::json, GetRawData, (const Disk&), (const, override));
};