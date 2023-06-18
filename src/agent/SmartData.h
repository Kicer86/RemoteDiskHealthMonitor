#pragma once

#include <map>
#include <QString>
#include <magic_enum.hpp>

class SmartData
{

public:
    enum SmartAttribute
    {
        ReadErrorRate = 0x01,
        ThroughputPerformance = 0x02,
        SpinUpTime = 0x03,
        StartStopCount = 0x04,
        ReallocatedSectorsCount = 0x05,
        ReadChannelMargin = 0x06,
        SeekErrorRate = 0x07,
        SeekTimePerformance = 0x08,
        PowerOnHoursPOH = 0x09,
        SpinRetryCount = 0x0A,
        CalibrationRetryCount = 0x0B,
        PowerCycleCount = 0x0C,
        SoftReadErrorRate = 0x0D,
        WriteProtectMode = 0xA7,                    // Kingston
        SATAPhyErrorCount = 0xA8,                   // Kingston
        BadBlockRate = 0xA9,                        // Kingston
        BadBlkCtErlLat = 0xAA,                      // Kingston
        AvailableReservedSpace = 0xAA,
        SSDProgramFailCount = 0xAB,
        SSDEraseFailCount = 0xAC,
        SSDWearLevelingCount = 0xAD,
        UnexpectedPowerLossCount = 0xAE,
        PowerLossProtectionFailure = 0xAF,
        EraseFailCount = 0xB0,
        WearRangeDelta = 0xB1,
        UsedReservedBlockCount = 0xB2,
        UsedReservedBlockCountTotal = 0xB3,
        UnusedReservedBlockCountTotal = 0xB4,
        ProgramFailCountTotal_Flash = 0xB5,
        Non4KAlignedAccessCount_HDD = 0xB5,
        EraseFailCount_Samsung = 0xB6,
        SATADownshiftErrorCount = 0xB7,
        RuntimeBadBlock = 0xB7,
        EndtoEnderror = 0xB8,
        HeadStability = 0xB9,
        InducedOpVibrationDetection = 0xBA,
        ReportedUncorrectableErrors = 0xBB,
        CommandTimeout = 0xBC,
        HighFlyWrites = 0xBD,
        AirflowTemperatureWDC = 0xBE,
        TemperatureDifferencefrom100 = 0xBE,
        GSenseErrorRate = 0xBF,
        PoweroffRetractCount = 0xC0,
        LoadCycleCount = 0xC1,
        Temperature = 0xC2,
        HardwareECCRecovered = 0xC3,
        ReallocationEventCount = 0xC4,
        CurrentPendingSectorCount = 0xC5,
        UncorrectableSectorCount = 0xC6,
        UltraDMACRCErrorCount = 0xC7,
        MultiZoneErrorRate = 0xC8,
        WriteErrorRateFujitsu = 0xC8,
        OffTrackSoftReadErrorRate = 0xC9,
        DataAddressMarkerrors = 0xCA,
        RunOutCancel = 0xCB,
        SoftECCCorrection = 0xCC,
        ThermalAsperityRateTAR = 0xCD,
        FlyingHeight = 0xCE,
        SpinHighCurrent = 0xCF,
        SpinBuzz = 0xD0,
        OfflineSeekPerformance = 0xD1,
        VibrationDuringWrite = 0xD3,
        ShockDuringWrite = 0xD4,
        CRCErrorCount  = 0xDA,                      // Kingston
        DiskShift = 0xDC,
        GSenseErrorRateAlt = 0xDD,
        LoadedHours = 0xDE,
        LoadUnloadRetryCount = 0xDF,
        LoadFriction = 0xE0,
        LoadUnloadCycleCount = 0xE1,
        LoadInTime = 0xE2,
        TorqueAmplificationCount = 0xE3,
        PowerOffRetractCycle = 0xE4,
        GMRHeadAmplitude = 0xE6,
        DriveTemperature = 0xE7,
        EnduranceRemaining = 0xE8,
        AvailableReservedSpace_Intel = 0xE8,
        MediaWearoutIndicator = 0xE9,
        HeadFlyingHours = 0xF0,
        TransferErrorRateFujitsu = 0xF0,
        TotalLBAsWritten = 0xF1,
        TotalLBAsRead = 0xF2,
        TotalLBAsWrittenExpanded = 0xF3,
        TotalLBAsReadExpanded = 0xF4,
        MaxEraseCount = 0xF5,                       // Kingston
        CumulativeHostSectorsWritten = 0xF6,
        HostProgramPageCount = 0xF7,
        BackgroundProgramPageCount = 0xF8,
        NANDWrites = 0xF9,
        ReadErrorRetryRate = 0xFA,
        MinimumSparesRemaining = 0xFB,
        NewlyAddedBadFlashBlock = 0xFC,
        FreeFallProtection = 0xFE,
    };

    struct AttrData
    {
        int value;
        int worst;
        int rawVal;

        auto operator<=>(const AttrData &) const = default;
    };

    auto operator<=>(const SmartData &) const = default;

    void add(SmartAttribute, const AttrData &);
    const std::map<SmartAttribute, AttrData>& data() const;

    static std::optional<QString> GetAttrTypeName(const SmartAttribute& _uChar);

private:
    std::map<SmartAttribute, AttrData> m_smartData;
};


template <>
struct magic_enum::customize::enum_range<SmartData::SmartAttribute>
{
  static constexpr int min = 0;
  static constexpr int max = 255;
};
