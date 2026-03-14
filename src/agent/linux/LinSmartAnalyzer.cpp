
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
    for (const auto& attr : smart.attributes)
    {
        attrs.push_back({
            {"id", attr.id},
            {"name", attr.name},
            {"value", attr.value},
            {"worst", attr.worst},
            {"threshold", attr.threshold},
            {"rawVal", attr.rawVal}
        });
    }

    return nlohmann::json{{"type", "smart"}, {"attributes", attrs}};
}
