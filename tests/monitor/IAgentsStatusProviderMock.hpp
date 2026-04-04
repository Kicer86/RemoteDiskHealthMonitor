
#pragma once

#include <gmock/gmock.h>

#include "IAgentsStatusProvider.hpp"

class IAgentsStatusProviderMock: public IAgentsStatusProvider
{
public:
    MOCK_METHOD(void, observe, (const AgentInformation &), (override));
    MOCK_METHOD(void, unobserve, (const AgentInformation &), (override));
};
