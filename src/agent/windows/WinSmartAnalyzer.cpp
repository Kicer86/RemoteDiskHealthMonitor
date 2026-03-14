#include "WinSmartAnalyzer.h"
#include "../SmartReader.h"
#include "common/SmartData.h"

GeneralHealth::Health WinSmartAnalyzer::GetStatus(const Disk& _disk)
{
	return GeneralHealth::Health::UNKNOWN;
}

nlohmann::json WinSmartAnalyzer::GetRawData(const Disk& _disk)
{
	SmartReader reader;
	const auto smart = reader.ReadSMARTData(_disk);

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
