#include "../SmartReader.h"
#include "WMICommunication.h"
#include "CMDCommunication.h"
#include <iostream>


SmartData SmartReader::ReadSMARTData(const Disk& _disk)
{
	WMICommunication wmi;
	wmi.WMIInit();
	wmi.CollectSMARTDataViaWMI(_disk);

	return wmi.GetSMARTData();
}

GeneralHealth::Health SmartReader::ReadDisksStatus(const Disk& _disk)
{
	CMDCommunication reader;
	return reader.CollectDiskStatus(_disk);
}

std::string SmartReader::ReadRawOutput(const Disk&)
{
	return {};
}

SmartTestStatus SmartReader::ReadTestStatus(const Disk&)
{
	return {};
}
