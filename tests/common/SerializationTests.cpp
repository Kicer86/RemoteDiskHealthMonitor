
#include <gtest/gtest.h>

#include "common/JsonSerialize.h"


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
