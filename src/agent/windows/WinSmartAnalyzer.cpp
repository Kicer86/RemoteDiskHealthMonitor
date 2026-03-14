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
