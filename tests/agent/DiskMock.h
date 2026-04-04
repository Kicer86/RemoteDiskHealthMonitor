#pragma once

#include <gmock/gmock.h>
#include "Disk.h"

class DiskMock : public Disk
{
public:
    DiskMock()
    {
        m_deviceId = "mock_deviceId";
    }

    DiskMock(const std::string& deviceId, const std::string& model)
        : Disk(deviceId, model)
    {
    }
};
