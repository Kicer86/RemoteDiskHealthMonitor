
#include <gmock/gmock.h>

#include "linux/SmartCtlOutputParser.h"

using testing::UnorderedElementsAre;


TEST(SmartCtlOutputParserTest, parseTestStatusCompleted)
{
    const auto status = SmartCtlOutputParser::parseTestStatus(
        R"(
            Self-test execution status:      (   0) The previous self-test routine completed
                                                    without error or no self-test has ever
                                                    been run.

            SMART Self-test log structure revision number 1
            Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
            # 1  Extended offline    Completed without error       00%      2161         -
        )"
    );

    EXPECT_FALSE(status.running);
    EXPECT_EQ(0, status.percentRemaining);
    EXPECT_EQ("Completed without error", status.lastResult);
}


TEST(SmartCtlOutputParserTest, parseTestStatusRunning)
{
    const auto status = SmartCtlOutputParser::parseTestStatus(
        R"(
            Self-test execution status:      ( 249) Self-test routine in progress...
                                                    90% of test remaining.

            SMART Self-test log structure revision number 1
            Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
            # 1  Extended offline    Completed without error       00%      2161         -
        )"
    );

    EXPECT_TRUE(status.running);
    EXPECT_EQ(90, status.percentRemaining);
    EXPECT_EQ("Completed without error", status.lastResult);
}


TEST(SmartCtlOutputParserTest, parseTestStatusNoTestsRun)
{
    const auto status = SmartCtlOutputParser::parseTestStatus(
        R"(
            Self-test execution status:      (   0) The previous self-test routine completed
                                                    without error or no self-test has ever
                                                    been run.

            SMART Self-test log structure revision number 1
            No self-tests have been logged.  [To run self-tests, use: smartctl -t]
        )"
    );

    EXPECT_FALSE(status.running);
    EXPECT_EQ(0, status.percentRemaining);
    EXPECT_TRUE(status.lastResult.empty());
}


TEST(SmartCtlOutputParserTest, parseTestStatusReadFailure)
{
    const auto status = SmartCtlOutputParser::parseTestStatus(
        R"(
            Self-test execution status:      (   0) The previous self-test routine completed
                                                    without error or no self-test has ever
                                                    been run.

            SMART Self-test log structure revision number 1
            Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
            # 1  Short offline       Completed: read failure       90%      4567         1234567
        )"
    );

    EXPECT_FALSE(status.running);
    EXPECT_EQ("Completed: read failure", status.lastResult);
}


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

    EXPECT_THAT(result.attributes, UnorderedElementsAre(
        SmartData::Attribute{  1, "Raw_Read_Error_Rate",     200, 200, 51,  0     },
        SmartData::Attribute{  3, "Spin_Up_Time",            171, 170, 21,  2408  },
        SmartData::Attribute{  4, "Start_Stop_Count",         99,  99,  0,  1140  },
        SmartData::Attribute{  5, "Reallocated_Sector_Ct",   200, 200, 140, 0     },
        SmartData::Attribute{  7, "Seek_Error_Rate",         200, 200,  0,  0     },
        SmartData::Attribute{  9, "Power_On_Hours",           84,  84,  0,  12128 },
        SmartData::Attribute{ 10, "Spin_Retry_Count",        100, 100,  0,  0     },
        SmartData::Attribute{ 11, "Calibration_Retry_Count", 100, 100,  0,  0     },
        SmartData::Attribute{ 12, "Power_Cycle_Count",        99,  99,  0,  1138  },
        SmartData::Attribute{192, "Power-Off_Retract_Count", 200, 200,  0,  30    },
        SmartData::Attribute{193, "Load_Cycle_Count",        200, 200,  0,  1295  },
        SmartData::Attribute{194, "Temperature_Celsius",     111,  98,  0,  32    },
        SmartData::Attribute{196, "Reallocated_Event_Count", 200, 200,  0,  0     },
        SmartData::Attribute{197, "Current_Pending_Sector",  200, 200,  0,  0     },
        SmartData::Attribute{198, "Offline_Uncorrectable",   200, 200,  0,  0     },
        SmartData::Attribute{199, "UDMA_CRC_Error_Count",    200, 200,  0,  0     },
        SmartData::Attribute{200, "Multi_Zone_Error_Rate",   200, 200,  0,  0     }
    ));
}

TEST(SmartCtlOutputParserTest, handlesLargeRawValuesAndMultiTokenFields)
{
    const auto result = SmartCtlOutputParser::parse(
        R"(
            Vendor Specific SMART Attributes with Thresholds:
            ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
            165 Block_Erase_Count       0x0032   100   100   000    Old_age   Always       -       481240550006
            194 Temperature_Celsius     0x0022   076   043   000    Old_age   Always       -       24 (Min/Max 16/49)
            230 Media_Wearout_Indicator 0x0032   100   100   000    Old_age   Always       -       0x0523023c0523
        )"
    );

    EXPECT_THAT(result.attributes, UnorderedElementsAre(
        SmartData::Attribute{165, "Block_Erase_Count",       100, 100, 0, 481240550006LL},
        SmartData::Attribute{194, "Temperature_Celsius",      76,  43, 0, 24           },
        SmartData::Attribute{230, "Media_Wearout_Indicator", 100, 100, 0, 0x0523023c0523LL}
    ));
}

TEST(SmartCtlOutputParserTest, handlesDashThresholdAsZero)
{
    const auto result = SmartCtlOutputParser::parse(
        R"(
            Vendor Specific SMART Attributes with Thresholds:
            ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
              5 Reallocated_Sector_Ct   0x0032   100   100   ---    Old_age   Always       -       0
              9 Power_On_Hours          0x0032   100   100   ---    Old_age   Always       -       12595
            232 Available_Reservd_Space 0x0033   100   100   004    Pre-fail  Always       -       100
        )"
    );

    EXPECT_THAT(result.attributes, UnorderedElementsAre(
        SmartData::Attribute{  5, "Reallocated_Sector_Ct",   100, 100, 0, 0    },
        SmartData::Attribute{  9, "Power_On_Hours",          100, 100, 0, 12595},
        SmartData::Attribute{232, "Available_Reservd_Space", 100, 100, 4, 100  }
    ));
}

TEST(SmartCtlOutputParserTest, parsesNvmeHealthOutput)
{
    const auto result = SmartCtlOutputParser::parse(
        R"(
            smartctl 7.5 2025-04-30 r5714 [x86_64-linux-6.18.16-1-lts] (local build)
            Copyright (C) 2002-25, Bruce Allen, Christian Franke, www.smartmontools.org

            === START OF INFORMATION SECTION ===
            Model Number:                       WD_BLACK SN850X 4000GB
            Serial Number:                      240258445614

            === START OF SMART DATA SECTION ===
            SMART overall-health self-assessment test result: PASSED

            SMART/Health Information (NVMe Log 0x02, NSID 0xffffffff)
            Critical Warning:                   0x00
            Temperature:                        50 Celsius
            Available Spare:                    100%
            Available Spare Threshold:          10%
            Percentage Used:                    0%
            Data Units Read:                    104 824 560 [53,6 TB]
            Data Units Written:                 80 482 532 [41,2 TB]
            Host Read Commands:                 233 072 100
            Host Write Commands:                781 541 720
            Controller Busy Time:               864
            Power Cycles:                       601
            Power On Hours:                     4 542
            Unsafe Shutdowns:                   74
            Media and Data Integrity Errors:    0
            Error Information Log Entries:      0
            Warning  Comp. Temperature Time:    0
            Critical Comp. Temperature Time:    0

            Error Information (NVMe Log 0x01, 16 of 256 entries)
            No Errors Logged
        )"
    );

    ASSERT_EQ(17u, result.attributes.size());

    // Check key fields by name
    auto findByName = [&](const std::string& name) -> const SmartData::Attribute* {
        for (const auto& a : result.attributes)
            if (a.name == name) return &a;
        return nullptr;
    };

    auto* critWarn = findByName("Critical_Warning");
    ASSERT_NE(nullptr, critWarn);
    EXPECT_EQ(0, critWarn->rawVal);

    auto* temp = findByName("Temperature");
    ASSERT_NE(nullptr, temp);
    EXPECT_EQ(50, temp->rawVal);

    auto* spare = findByName("Available_Spare");
    ASSERT_NE(nullptr, spare);
    EXPECT_EQ(100, spare->rawVal);
    EXPECT_EQ(100, spare->value);
    EXPECT_EQ(10, spare->threshold);  // threshold from Available Spare Threshold

    auto* spareThresh = findByName("Available_Spare_Threshold");
    ASSERT_NE(nullptr, spareThresh);
    EXPECT_EQ(10, spareThresh->rawVal);

    auto* pctUsed = findByName("Percentage_Used");
    ASSERT_NE(nullptr, pctUsed);
    EXPECT_EQ(0, pctUsed->rawVal);

    auto* unitsRead = findByName("Data_Units_Read");
    ASSERT_NE(nullptr, unitsRead);
    EXPECT_EQ(104824560LL, unitsRead->rawVal);

    auto* unitsWritten = findByName("Data_Units_Written");
    ASSERT_NE(nullptr, unitsWritten);
    EXPECT_EQ(80482532LL, unitsWritten->rawVal);

    auto* powerCycles = findByName("Power_Cycles");
    ASSERT_NE(nullptr, powerCycles);
    EXPECT_EQ(601, powerCycles->rawVal);

    auto* powerOnHours = findByName("Power_On_Hours");
    ASSERT_NE(nullptr, powerOnHours);
    EXPECT_EQ(4542, powerOnHours->rawVal);

    auto* unsafeShut = findByName("Unsafe_Shutdowns");
    ASSERT_NE(nullptr, unsafeShut);
    EXPECT_EQ(74, unsafeShut->rawVal);

    auto* mediaErrors = findByName("Media_and_Data_Integrity_Errors");
    ASSERT_NE(nullptr, mediaErrors);
    EXPECT_EQ(0, mediaErrors->rawVal);
}

TEST(SmartCtlOutputParserTest, nvmeWithOnlyHealthSection)
{
    const auto result = SmartCtlOutputParser::parse(
        R"(
            SMART/Health Information (NVMe Log 0x02, NSID 0xffffffff)
            Critical Warning:                   0x00
            Temperature:                        35 Celsius
            Available Spare:                    85%
            Available Spare Threshold:          5%
            Percentage Used:                    12%
        )"
    );

    ASSERT_EQ(5u, result.attributes.size());

    auto findByName = [&](const std::string& name) -> const SmartData::Attribute* {
        for (const auto& a : result.attributes)
            if (a.name == name) return &a;
        return nullptr;
    };

    auto* spare = findByName("Available_Spare");
    ASSERT_NE(nullptr, spare);
    EXPECT_EQ(85, spare->value);
    EXPECT_EQ(5, spare->threshold);

    auto* pctUsed = findByName("Percentage_Used");
    ASSERT_NE(nullptr, pctUsed);
    EXPECT_EQ(12, pctUsed->rawVal);
}
