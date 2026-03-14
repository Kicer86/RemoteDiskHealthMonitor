#include "SmartHealthAnalyzer.h"
#include "VendorProfile.h"
#include "common/SmartData.h"

#include <algorithm>
#include <array>

namespace
{
    // SMART attribute IDs considered critical for disk health.
    // Non-zero raw values for these indicate potential failure.
    constexpr std::array criticalAttrIds = {
        uint8_t(0x05),  // Reallocated_Sector_Ct
        uint8_t(0x0A),  // Spin_Retry_Count
        uint8_t(0xC4),  // Reallocated_Event_Count
        uint8_t(0xC5),  // Current_Pending_Sector
        uint8_t(0xC6),  // Offline_Uncorrectable
    };

    bool isCritical(uint8_t id)
    {
        return std::find(criticalAttrIds.begin(), criticalAttrIds.end(), id) != criticalAttrIds.end();
    }

    // Percentage of threshold at which we start warning.
    constexpr int proximityPercent = 15;
}


SmartHealthAnalyzer::SmartHealthAnalyzer(std::unique_ptr<ISmartReader> reader)
    : m_reader(std::move(reader))
{
}

SmartHealthAnalyzer::~SmartHealthAnalyzer() = default;


const IVendorProfile& SmartHealthAnalyzer::profileFor(const std::string& vendor)
{
    static const GenericProfile generic;
    static const SamsungProfile samsung;
    static const SeagateProfile seagate;

    if (vendor == "Samsung")
        return samsung;
    if (vendor == "Seagate")
        return seagate;

    return generic;
}


GeneralHealth::Health SmartHealthAnalyzer::GetStatus(const Disk& disk)
{
    const auto smart = m_reader->ReadSMARTData(disk);
    const auto& profile = profileFor(disk.GetVendor());

    auto worst = GeneralHealth::GOOD;

    for (const auto& attr : smart.attributes)
    {
        // Layer 1: threshold breach → BAD
        if (attr.threshold > 0 && attr.value > 0 && attr.value <= attr.threshold)
        {
            return GeneralHealth::BAD;
        }

        // Layer 2: critical attributes — check interpreted raw value
        if (isCritical(attr.id))
        {
            const auto interpreted = profile.interpretRawValue(attr.id, attr.rawVal);
            if (interpreted > 0)
                worst = std::max(worst, GeneralHealth::CHECK_STATUS);
        }

        // Layer 3: proximity to threshold → CHECK_STATUS
        if (attr.threshold > 0 && attr.value > attr.threshold)
        {
            const int margin = (attr.threshold * proximityPercent) / 100;
            if (attr.value <= attr.threshold + std::max(margin, 1))
                worst = std::max(worst, GeneralHealth::CHECK_STATUS);
        }
    }

    return worst;
}


nlohmann::json SmartHealthAnalyzer::GetRawData(const Disk& disk)
{
    const auto smart = m_reader->ReadSMARTData(disk);

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
