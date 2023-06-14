
#include "LinSmartAnalyzer.h"
#include "../SmartReader.h"
#include "../JSonUtils.hpp"

GeneralHealth::Health LinSmartAnalyzer::GetStatus(const Disk& _disk)
{
    return GeneralHealth::Health();
}


QString LinSmartAnalyzer::GetJSonData(const Disk& _disk)
{
    const auto smart = SmartReader().ReadSMARTData(_disk);

    return JSonUtils::SmartDataToJSon(smart);
}
