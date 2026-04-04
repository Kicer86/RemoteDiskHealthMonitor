#pragma once
#include "ISmartReader.h"

class SmartReader : public ISmartReader
{
public:
	SmartData ReadSMARTData(const Disk & _disk) override;
	GeneralHealth::Health ReadDisksStatus(const Disk & _disk) override;
	std::string ReadRawOutput(const Disk & _disk) override;
	SmartTestStatus ReadTestStatus(const Disk & _disk) override;
};
