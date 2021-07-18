
#include <gmock/gmock.h>

#include "linux/SmartCtlOutputParser.h"

using testing::UnorderedElementsAre;

TEST(SmartCtlOutputParserTest, fullOutput)
{
    const auto result = SmartCtlOutputParser::parse(
        R"(
            smartctl 7.1 2019-12-30 r5022 [x86_64-linux-5.4.85-1-lts] (local build)
            Copyright (C) 2002-19, Bruce Allen, Christian Franke, www.smartmontools.org

            === START OF INFORMATION SECTION ===
            Model Family:     Western Digital Blue
            Device Model:     WDC WD10EZEX-35WN4A0
            Serial Number:    WCC6Y5ZE7F40
            LU WWN Device Id: 5 0014ee 20e424ca2
            Firmware Version: 01.01A01
            User Capacity:    1 000 204 886 016 bytes [1,00 TB]
            Sector Sizes:     512 bytes logical, 4096 bytes physical
            Rotation Rate:    7200 rpm
            Form Factor:      3.5 inches
            Device is:        In smartctl database [for details use: -P show]
            ATA Version is:   ACS-3 T13/2161-D revision 3b
            SATA Version is:  SATA 3.1, 6.0 Gb/s (current: 6.0 Gb/s)
            Local Time is:    Thu Dec 31 16:29:02 2020 CET
            SMART support is: Available - device has SMART capability.
            SMART support is: Enabled

            === START OF READ SMART DATA SECTION ===
            SMART overall-health self-assessment test result: PASSED

            General SMART Values:
            Offline data collection status:  (0x84) Offline data collection activity
                                                    was suspended by an interrupting command from host.
                                                    Auto Offline Data Collection: Enabled.
            Self-test execution status:      (   0) The previous self-test routine completed
                                                    without error or no self-test has ever
                                                    been run.
            Total time to complete Offline
            data collection:                (12180) seconds.
            Offline data collection
            capabilities:                    (0x7b) SMART execute Offline immediate.
                                                    Auto Offline data collection on/off support.
                                                    Suspend Offline collection upon new
                                                    command.
                                                    Offline surface scan supported.
                                                    Self-test supported.
                                                    Conveyance Self-test supported.
                                                    Selective Self-test supported.
            SMART capabilities:            (0x0003) Saves SMART data before entering
                                                    power-saving mode.
                                                    Supports SMART auto save timer.
            Error logging capability:        (0x01) Error logging supported.
                                                    General Purpose Logging supported.
            Short self-test routine
            recommended polling time:        (   2) minutes.
            Extended self-test routine
            recommended polling time:        ( 126) minutes.
            Conveyance self-test routine
            recommended polling time:        (   5) minutes.
            SCT capabilities:              (0x3035) SCT Status supported.
                                                    SCT Feature Control supported.
                                                    SCT Data Table supported.

            SMART Attributes Data Structure revision number: 16
            Vendor Specific SMART Attributes with Thresholds:
            ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
            1 Raw_Read_Error_Rate     0x002f   200   200   051    Pre-fail  Always       -       0
            3 Spin_Up_Time            0x0027   171   170   021    Pre-fail  Always       -       2408
            4 Start_Stop_Count        0x0032   099   099   000    Old_age   Always       -       1140
            5 Reallocated_Sector_Ct   0x0033   200   200   140    Pre-fail  Always       -       0
            7 Seek_Error_Rate         0x002e   200   200   000    Old_age   Always       -       0
            9 Power_On_Hours          0x0032   084   084   000    Old_age   Always       -       12128
            10 Spin_Retry_Count        0x0032   100   100   000    Old_age   Always       -       0
            11 Calibration_Retry_Count 0x0032   100   100   000    Old_age   Always       -       0
            12 Power_Cycle_Count       0x0032   099   099   000    Old_age   Always       -       1138
            192 Power-Off_Retract_Count 0x0032   200   200   000    Old_age   Always       -       30
            193 Load_Cycle_Count        0x0032   200   200   000    Old_age   Always       -       1295
            194 Temperature_Celsius     0x0022   111   098   000    Old_age   Always       -       32
            196 Reallocated_Event_Count 0x0032   200   200   000    Old_age   Always       -       0
            197 Current_Pending_Sector  0x0032   200   200   000    Old_age   Always       -       0
            198 Offline_Uncorrectable   0x0030   200   200   000    Old_age   Offline      -       0
            199 UDMA_CRC_Error_Count    0x0032   200   200   000    Old_age   Always       -       0
            200 Multi_Zone_Error_Rate   0x0008   200   200   000    Old_age   Offline      -       0

            SMART Error Log Version: 1
            No Errors Logged

            SMART Self-test log structure revision number 1
            Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
            # 1  Extended offline    Completed without error       00%      2161         -

            SMART Selective self-test log data structure revision number 1
            SPAN  MIN_LBA  MAX_LBA  CURRENT_TEST_STATUS
                1        0        0  Not_testing
                2        0        0  Not_testing
                3        0        0  Not_testing
                4        0        0  Not_testing
                5        0        0  Not_testing
            Selective self-test flags (0x0):
            After scanning selected spans, do NOT read-scan remainder of disk.
            If Selective self-test is pending on power-up, resume after 0 minute delay.
        )"
    );

    EXPECT_THAT(result.smartData, UnorderedElementsAre(
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::ReadErrorRate,             SmartData::AttrData{200, 200, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::SpinUpTime,                SmartData::AttrData{171, 170, 2408 } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::StartStopCount,            SmartData::AttrData{ 99,  99, 1140 } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::ReallocatedSectorsCount,   SmartData::AttrData{200, 200, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::SeekErrorRate,             SmartData::AttrData{200, 200, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::PowerOnHoursPOH,           SmartData::AttrData{ 84,  84, 12128} },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::SpinRetryCount,            SmartData::AttrData{100, 100, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::CalibrationRetryCount,     SmartData::AttrData{100, 100, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::PowerCycleCount,           SmartData::AttrData{ 99,  99, 1138 } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::PoweroffRetractCount,      SmartData::AttrData{200, 200, 30   } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::LoadCycleCount,            SmartData::AttrData{200, 200, 1295 } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::Temperature,               SmartData::AttrData{111,  98, 32   } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::ReallocationEventCount,    SmartData::AttrData{200, 200, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::CurrentPendingSectorCount, SmartData::AttrData{200, 200, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::UncorrectableSectorCount,  SmartData::AttrData{200, 200, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::UltraDMACRCErrorCount,     SmartData::AttrData{200, 200, 0    } },
        std::pair<SmartData::SmartAttribute, SmartData::AttrData>{SmartData::MultiZoneErrorRate,        SmartData::AttrData{200, 200, 0    } }
    ));
}
