
#include "LinSmartAnalyzer.h"
#include "../SmartReader.h"
#include "common/SmartData.h"

GeneralHealth::Health LinSmartAnalyzer::GetStatus(const Disk& _disk)
{
    return GeneralHealth::Health();
}


nlohmann::json LinSmartAnalyzer::GetRawData(const Disk& _disk)
{
    const auto smart = SmartReader().ReadSMARTData(_disk);

    nlohmann::json attrs = nlohmann::json::array();
    for (const auto& [attr, data] : smart.smartData)
    {
        attrs.push_back({
            {"name", SmartData::GetAttrTypeName(attr)},
            {"value", data.value},
            {"worst", data.worst},
            {"rawVal", data.rawVal}
        });
    }

    return nlohmann::json{{"type", "smart"}, {"attributes", attrs}};
}
