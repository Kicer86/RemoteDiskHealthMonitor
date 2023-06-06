#include "WinSmartAnalyzer.h"
#include "../SmartReader.h"

GeneralHealth::Health WinSmartAnalyzer::GetStatus(const Disk& _disk)
{
	return GeneralHealth::Health::UNKNOWN;
}

QString WinSmartAnalyzer::GetJSonData(const Disk& _disk)
{
	SmartReader reader;

	return reader.ReadSMARTData(_disk).toJSon();
}
