#include "WinGeneralAnalyzer.h"
#include "CMDCommunication.h"


GeneralHealth::Health WinGeneralAnalyzer::GetStatus(const Disk& _disk)
{
    CMDCommunication reader;
    return reader.CollectDiskStatus(_disk);
}

nlohmann::json WinGeneralAnalyzer::GetRawData(const Disk& _disk)
{
    return nlohmann::json{{"type", "text"}, {"value", std::string()}};
}
