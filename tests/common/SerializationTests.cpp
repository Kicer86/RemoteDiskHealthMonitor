
#include <gtest/gtest.h>

#include "common/JsonSerialize.h"


TEST(SerializationTest, DiskSummaryFull)
{
    DiskSummary summary;
    summary.model = "Samsung SSD 860 EVO 500GB";
    summary.vendor = "Samsung";
    summary.capacityBytes = 500107862016;
    summary.driveType = "SSD";
    summary.temperatureC = 34;
    summary.powerOnHours = 12345;
    summary.selfTestStatus = SmartTestStatus{
        .running = true,
        .percentRemaining = 40,
        .lastResult = "Completed without error"
    };

    nlohmann::json j = summary;
    auto summary2 = j.get<DiskSummary>();

    EXPECT_EQ(summary, summary2);
}


TEST(SerializationTest, DiskSummaryMinimal)
{
    DiskSummary summary;
    summary.model = "WDC WD10EZEX";
    summary.vendor = "WDC";
    summary.capacityBytes = 1000204886016;
    summary.driveType = "HDD";

    nlohmann::json j = summary;
    auto summary2 = j.get<DiskSummary>();

    EXPECT_EQ(summary, summary2);
    EXPECT_FALSE(j.contains("temperatureC"));
    EXPECT_FALSE(j.contains("powerOnHours"));
    EXPECT_FALSE(j.contains("selfTestStatus"));
}


TEST(SerializationTest, SmartTestStatusRoundTrip)
{
    SmartTestStatus status{
        .running = false,
        .percentRemaining = 0,
        .lastResult = "Completed without error"
    };

    nlohmann::json j = status;
    auto status2 = j.get<SmartTestStatus>();

    EXPECT_EQ(status, status2);
}


TEST(SerializationTest, DiskInfo)
{
    const DiskInfo di("disk #1", GeneralHealth::CHECK_STATUS, {
        ProbeStatus{.health = GeneralHealth::GOOD, .rawData = nlohmann::json{{"type", "text"}, {"value", "Some data"}}},
        ProbeStatus{.health = GeneralHealth::BAD, .rawData = nlohmann::json{{"type", "text"}, {"value", "Some other data"}}}
    });

    nlohmann::json j = di;
    DiskInfo di2 = j.get<DiskInfo>();

    EXPECT_EQ(di, di2);
}


TEST(SerializationTest, DiskInfoList)
{
    const std::vector<DiskInfo> di_vec = {
        DiskInfo("disk #1", GeneralHealth::CHECK_STATUS, {
            ProbeStatus{.health = GeneralHealth::GOOD, .rawData = nlohmann::json{{"type", "text"}, {"value", "Some data"}}},
            ProbeStatus{.health = GeneralHealth::BAD, .rawData = nlohmann::json{{"type", "text"}, {"value", "Some other data"}}}
        })
    };

    nlohmann::json j = di_vec;
    auto di_vec2 = j.get<std::vector<DiskInfo>>();

    EXPECT_EQ(di_vec, di_vec2);
}
