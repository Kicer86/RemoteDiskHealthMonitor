
#include "SmartData.h"

namespace
{
    const std::map<SmartData::SmartAttribute, QString> dictionary =
    {
        {	SmartData::ReadErrorRate,                 "ReadErrorRate"	}	,
        {	SmartData::ThroughputPerformance,         "ThroughputPerformance"	}	,
        {	SmartData::SpinUpTime,                    "SpinUpTime"	}	,
        {	SmartData::StartStopCount,                "StartStopCount"	}	,
        {	SmartData::ReallocatedSectorsCount,       "ReallocatedSectorsCount"	}	,
        {	SmartData::ReadChannelMargin,             "ReadChannelMargin"	}	,
        {	SmartData::SeekErrorRate,                 "SeekErrorRate"	}	,
        {	SmartData::SeekTimePerformance,           "SeekTimePerformance"	}	,
        {	SmartData::PowerOnHoursPOH,               "PowerOnHoursPOH"	}	,
        {	SmartData::SpinRetryCount,                "SpinRetryCount"	}	,
        {	SmartData::CalibrationRetryCount,         "CalibrationRetryCount"	}	,
        {	SmartData::PowerCycleCount,               "PowerCycleCount"	}	,
        {	SmartData::SoftReadErrorRate,             "SoftReadErrorRate"	}	,
        {	SmartData::SATADownshiftErrorCount,       "SATADownshiftErrorCount"	}	,
        {	SmartData::EndtoEnderror,                 "EndtoEnderror"	}	,
        {	SmartData::HeadStability,                 "HeadStability"	}	,
        {	SmartData::InducedOpVibrationDetection,   "InducedOpVibrationDetection"	}	,
        {	SmartData::ReportedUncorrectableErrors,   "ReportedUncorrectableErrors"	}	,
        {	SmartData::CommandTimeout,                "CommandTimeout"	}	,
        {	SmartData::HighFlyWrites,                 "HighFlyWrites"	}	,
        {	SmartData::AirflowTemperatureWDC,         "AirflowTemperatureWDC"	}	,
        {	SmartData::TemperatureDifferencefrom100,  "TemperatureDifferencefrom100"	}	,
        {	SmartData::GSenseErrorRate,               "GSenseErrorRate"	}	,
        {	SmartData::PoweroffRetractCount,          "PoweroffRetractCount"	}	,
        {	SmartData::LoadCycleCount,                "LoadCycleCount"	}	,
        {	SmartData::Temperature,                   "Temperature"	}	,
        {	SmartData::HardwareECCRecovered,          "HardwareECCRecovered"	}	,
        {	SmartData::ReallocationEventCount,        "ReallocationEventCount"	}	,
        {	SmartData::CurrentPendingSectorCount,     "CurrentPendingSectorCount"	}	,
        {	SmartData::UncorrectableSectorCount,      "UncorrectableSectorCount"	}	,
        {	SmartData::UltraDMACRCErrorCount,         "UltraDMACRCErrorCount"	}	,
        {	SmartData::MultiZoneErrorRate,            "MultiZoneErrorRate"	}	,
        {	SmartData::WriteErrorRateFujitsu,         "WriteErrorRateFujitsu"	}	,
        {	SmartData::OffTrackSoftReadErrorRate,     "OffTrackSoftReadErrorRate"	}	,
        {	SmartData::DataAddressMarkerrors,         "DataAddressMarkerrors"	}	,
        {	SmartData::RunOutCancel,                  "RunOutCancel"	}	,
        {	SmartData::SoftECCCorrection,             "SoftECCCorrection"	}	,
        {	SmartData::ThermalAsperityRateTAR,        "ThermalAsperityRateTAR"	}	,
        {	SmartData::FlyingHeight,                  "FlyingHeight"	}	,
        {	SmartData::SpinHighCurrent,               "SpinHighCurrent"	}	,
        {	SmartData::SpinBuzz,                      "SpinBuzz"	}	,
        {	SmartData::OfflineSeekPerformance,        "OfflineSeekPerformance"	}	,
        {	SmartData::VibrationDuringWrite,          "VibrationDuringWrite"	}	,
        {	SmartData::ShockDuringWrite,              "ShockDuringWrite"	}	,
        {	SmartData::DiskShift,                     "DiskShift"	}	,
        {	SmartData::GSenseErrorRateAlt,            "GSenseErrorRateAlt"	}	,
        {	SmartData::LoadedHours,                   "LoadedHours"	}	,
        {	SmartData::LoadUnloadRetryCount,          "LoadUnloadRetryCount"	}	,
        {	SmartData::LoadFriction,                  "LoadFriction"	}	,
        {	SmartData::LoadUnloadCycleCount,          "LoadUnloadCycleCount"	}	,
        {	SmartData::LoadInTime,                    "LoadInTime"	}	,
        {	SmartData::TorqueAmplificationCount,      "TorqueAmplificationCount"	}	,
        {	SmartData::PowerOffRetractCycle,          "PowerOffRetractCycle"	}	,
        {	SmartData::GMRHeadAmplitude,              "GMRHeadAmplitude"	}	,
        {	SmartData::DriveTemperature,              "DriveTemperature"	}	,
        {	SmartData::HeadFlyingHours,               "HeadFlyingHours"	}	,
        {	SmartData::TransferErrorRateFujitsu,      "TransferErrorRateFujitsu"	}	,
        {	SmartData::TotalLBAsWritten,              "TotalLBAsWritten"	}	,
        {	SmartData::TotalLBAsRead,                 "TotalLBAsRead"	}	,
        {	SmartData::ReadErrorRetryRate,            "ReadErrorRetryRate"	}	,
        {	SmartData::FreeFallProtection,            "FreeFallProtection"	}
    };
}


void SmartData::add(SmartData::SmartAttribute attr, const SmartData::AttrData& value)
{
    m_smartData.emplace(attr, value);
}


const std::map<SmartData::SmartAttribute, SmartData::AttrData>& SmartData::data() const
{
    return m_smartData;
}


QString SmartData::GetAttrTypeName(const SmartAttribute& _uChar)
{
    auto it = dictionary.find(_uChar);
    if(it != dictionary.end())
    {
        return it->second;
    }
    else
    {
        return "Unknown Attribute";
    }
}
