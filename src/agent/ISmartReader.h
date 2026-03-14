#pragma once

#include <string>

#include "common/GeneralHealth.h"
#include "common/DiskSummary.h"
#include "agent/Disk.h"
#include "common/SmartData.h"


class ISmartReader
{
public:
	virtual ~ISmartReader() = default;
	virtual SmartData ReadSMARTData(const Disk & _disk) = 0;
	virtual GeneralHealth::Health ReadDisksStatus(const Disk & _disk) = 0;
	virtual std::string ReadRawOutput(const Disk & _disk) = 0;
	virtual SmartTestStatus ReadTestStatus(const Disk & _disk) = 0;
};
