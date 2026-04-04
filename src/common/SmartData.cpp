#include "SmartData.h"

#include <map>

namespace
{
    const std::map<uint8_t, std::string> canonicalNames =
    {
        { 0x01, "Raw_Read_Error_Rate" },
        { 0x02, "Throughput_Performance" },
        { 0x03, "Spin_Up_Time" },
        { 0x04, "Start_Stop_Count" },
        { 0x05, "Reallocated_Sector_Ct" },
        { 0x06, "Read_Channel_Margin" },
        { 0x07, "Seek_Error_Rate" },
        { 0x08, "Seek_Time_Performance" },
        { 0x09, "Power_On_Hours" },
        { 0x0A, "Spin_Retry_Count" },
        { 0x0B, "Calibration_Retry_Count" },
        { 0x0C, "Power_Cycle_Count" },
        { 0x0D, "Soft_Read_Error_Rate" },
        { 0xB7, "SATA_Downshift_Error_Ct" },
        { 0xB8, "End-to-End_Error" },
        { 0xB9, "Head_Stability" },
        { 0xBA, "Induced_Op_Vibration_Det" },
        { 0xBB, "Reported_Uncorrect" },
        { 0xBC, "Command_Timeout" },
        { 0xBD, "High_Fly_Writes" },
        { 0xBE, "Airflow_Temperature_Cel" },
        { 0xBF, "G-Sense_Error_Rate" },
        { 0xC0, "Power-Off_Retract_Count" },
        { 0xC1, "Load_Cycle_Count" },
        { 0xC2, "Temperature_Celsius" },
        { 0xC3, "Hardware_ECC_Recovered" },
        { 0xC4, "Reallocated_Event_Count" },
        { 0xC5, "Current_Pending_Sector" },
        { 0xC6, "Offline_Uncorrectable" },
        { 0xC7, "UDMA_CRC_Error_Count" },
        { 0xC8, "Multi_Zone_Error_Rate" },
        { 0xC9, "Soft_Read_Error_Rate" },
        { 0xCA, "Data_Address_Mark_Errs" },
        { 0xCB, "Run_Out_Cancel" },
        { 0xCC, "Soft_ECC_Correction" },
        { 0xCD, "Thermal_Asperity_Rate" },
        { 0xCE, "Flying_Height" },
        { 0xCF, "Spin_High_Current" },
        { 0xD0, "Spin_Buzz" },
        { 0xD1, "Offline_Seek_Performnce" },
        { 0xD3, "Vibration_During_Write" },
        { 0xD4, "Shock_During_Write" },
        { 0xDC, "Disk_Shift" },
        { 0xDD, "G-Sense_Error_Rate_Alt" },
        { 0xDE, "Loaded_Hours" },
        { 0xDF, "Load_Unload_Retry_Ct" },
        { 0xE0, "Load_Friction" },
        { 0xE1, "Load_Unload_Cycle_Ct" },
        { 0xE2, "Load_In_Time" },
        { 0xE3, "Torque_Amplification_Ct" },
        { 0xE4, "Power-Off_Retract_Cycle" },
        { 0xE6, "GMR_Head_Amplitude" },
        { 0xE7, "Temperature_Celsius_Alt" },
        { 0xF0, "Head_Flying_Hours" },
        { 0xF1, "Total_LBAs_Written" },
        { 0xF2, "Total_LBAs_Read" },
        { 0xFA, "Read_Error_Retry_Rate" },
        { 0xFE, "Free_Fall_Sensor" },
    };
}

std::string SmartData::GetCanonicalName(uint8_t id)
{
    auto it = canonicalNames.find(id);
    if (it != canonicalNames.end())
        return it->second;

    return "Unknown_Attribute_" + std::to_string(id);
}
