#include "SmartHealthAnalyzer.h"
#include "VendorProfile.h"
#include "common/SmartData.h"

#include <algorithm>
#include <array>

namespace
{
    // ATA SMART attribute IDs considered critical for disk health.
    // Non-zero raw values for these indicate potential failure.
    // Matched by both ID and canonical name to avoid false positives
    // on NVMe synthetic IDs.
    struct CriticalAttr
    {
        uint8_t id;
        const char* canonicalName;
    };

    constexpr std::array<CriticalAttr, 5> criticalAttrs = {{
        {0x05, "Reallocated_Sector_Ct"},
        {0x0A, "Spin_Retry_Count"},
        {0xC4, "Reallocated_Event_Count"},
        {0xC5, "Current_Pending_Sector"},
        {0xC6, "Offline_Uncorrectable"},
    }};

    bool isCriticalAta(uint8_t id, const std::string& name)
    {
        return std::any_of(criticalAttrs.begin(), criticalAttrs.end(),
            [&](const CriticalAttr& ca) { return ca.id == id && name == ca.canonicalName; });
    }

    // NVMe health fields checked by name (IDs are synthetic).
    bool isCriticalNvme(const std::string& name)
    {
        return name == "Critical_Warning"
            || name == "Media_and_Data_Integrity_Errors";
    }

    bool isNvmePercentageUsed(const std::string& name)
    {
        return name == "Percentage_Used";
    }

    // Percentage of threshold at which we start warning.
    constexpr int proximityPercent = 15;

    // Percentage Used above this triggers CHECK_STATUS for NVMe.
    constexpr int nvmeWearWarningPercent = 90;
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
        // Layer 1: threshold breach → BAD (works for both ATA and NVMe Available Spare)
        if (attr.threshold > 0 && attr.value > 0 && attr.value <= attr.threshold)
        {
            return GeneralHealth::BAD;
        }

        // Layer 2a: ATA critical attributes — check interpreted raw value
        if (isCriticalAta(attr.id, attr.name))
        {
            const auto interpreted = profile.interpretRawValue(attr.id, attr.rawVal);
            if (interpreted > 0)
                worst = std::max(worst, GeneralHealth::CHECK_STATUS);
        }

        // Layer 2b: NVMe critical fields — non-zero raw means trouble
        if (isCriticalNvme(attr.name))
        {
            if (attr.rawVal > 0)
                worst = std::max(worst, GeneralHealth::BAD);
        }

        // Layer 2c: NVMe wear indicator
        if (isNvmePercentageUsed(attr.name))
        {
            if (attr.rawVal >= 100)
                worst = std::max(worst, GeneralHealth::BAD);
            else if (attr.rawVal >= nvmeWearWarningPercent)
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
