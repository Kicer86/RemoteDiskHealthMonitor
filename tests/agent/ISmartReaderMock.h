#pragma once

#include <gmock/gmock.h>

#include "ISmartReader.h"

class ISmartReaderMock : public ISmartReader
{
public:
    MOCK_METHOD(SmartData, ReadSMARTData, (const Disk&), (override));
    MOCK_METHOD(GeneralHealth::Health, ReadDisksStatus, (const Disk&), (override));
};
