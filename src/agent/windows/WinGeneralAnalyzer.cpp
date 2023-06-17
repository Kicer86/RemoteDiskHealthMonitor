#include "WinGeneralAnalyzer.h"
#include "CMDCommunication.h"


GeneralHealth::Health WinGeneralAnalyzer::GetStatus(const Disk& _disk)
{
    CMDCommunication reader;

    GeneralHealth res = reader.CollectDiskStatus( _disk );

    return static_cast<GeneralHealth::Health>(res.GetStatus());
}

QString WinGeneralAnalyzer::GetJSonData(const Disk& _disk)
{
    return {};
}
