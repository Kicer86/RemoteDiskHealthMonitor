
#include "LinSmartAnalyzer.h"
#include "../SmartReader.h"

GeneralHealth::Health LinSmartAnalyzer::GetStatus(const Disk& _disk)
{
    return GeneralHealth::Health();
}


QString LinSmartAnalyzer::GetJSonData(const Disk& _disk)
{
    const auto smart = SmartReader().ReadSMARTData(_disk);

    return smart.toJSon();
}
