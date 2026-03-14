#pragma once

#include <cstdint>
#include <string>

// Interprets raw SMART attribute values in a vendor-specific way.
// Some vendors (Samsung, Seagate) pack additional internal counters into
// the 6-byte raw value field, making face-value interpretation misleading.
class IVendorProfile
{
public:
    virtual ~IVendorProfile() = default;

    // Extract the meaningful error/event count from a raw attribute value.
    // For most vendors this is identity; Samsung/Seagate override for specific IDs.
    virtual int64_t interpretRawValue(uint8_t attrId, int64_t rawVal) const = 0;
};


// Default: raw value taken at face value.
class GenericProfile : public IVendorProfile
{
public:
    int64_t interpretRawValue(uint8_t, int64_t rawVal) const override
    {
        return rawVal;
    }
};


// Samsung SSDs/HDDs pack internal counters into upper bytes of some attributes.
// ID 1 (Raw_Read_Error_Rate): lower 32 bits = actual error count.
// ID 7 (Seek_Error_Rate): lower 32 bits = actual count (on Samsung HDDs).
class SamsungProfile : public IVendorProfile
{
public:
    int64_t interpretRawValue(uint8_t attrId, int64_t rawVal) const override
    {
        if (attrId == 0x01 || attrId == 0x07)
            return rawVal & 0xFFFFFFFF;

        return rawVal;
    }
};


// Seagate drives use packed encoding for several attributes.
// ID 1 (Raw_Read_Error_Rate): lower 32 bits = actual error count.
// ID 7 (Seek_Error_Rate): lower 32 bits = actual error count.
// ID 195 (Hardware_ECC_Recovered): lower 32 bits = actual count.
class SeagateProfile : public IVendorProfile
{
public:
    int64_t interpretRawValue(uint8_t attrId, int64_t rawVal) const override
    {
        if (attrId == 0x01 || attrId == 0x07 || attrId == 0xC3)
            return rawVal & 0xFFFFFFFF;

        return rawVal;
    }
};

