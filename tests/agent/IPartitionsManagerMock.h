#pragma once

#include <gmock/gmock.h>

#include "linux/IPartitionsManager.h"

class IPartitionsManagerMock: public IPartitionsManager
{
public:
    MOCK_METHOD(bool, isPartition, (const std::string &), (const, override));
    MOCK_METHOD(std::string, diskForPartition, (const std::string &), (const, override));
};
