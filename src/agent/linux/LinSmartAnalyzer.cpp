
#include "LinSmartAnalyzer.h"
#include "../SmartReader.h"

GeneralHealth::Health LinSmartAnalyzer::GetStatus(const Disk& _disk)
{
    return GeneralHealth::Health();
}


IProbe::RawData LinSmartAnalyzer::GetRawData(const Disk& _disk)
{
    const auto smart = SmartReader().ReadSMARTData(_disk);

    return smart;
}
