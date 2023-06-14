#include "WinSmartAnalyzer.h"
#include "../SmartReader.h"
#include "../JSonUtils.hpp"

GeneralHealth::Health WinSmartAnalyzer::GetStatus(const Disk& _disk)
{
	return GeneralHealth::Health::UNKNOWN;
}

QString WinSmartAnalyzer::GetJSonData(const Disk& _disk)
{
	SmartReader reader;

	return JSonUtils::SmartDataToJSon(reader.ReadSMARTData(_disk));
}
