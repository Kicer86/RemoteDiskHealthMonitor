
#include <gmock/gmock.h>

#include "common/ProbeStatus.h"
#include "common/JsonSerialize.h"


TEST(ProbeStatusTest, serializationOfStringRawData)
{
    ProbeStatus status;
    status.health = GeneralHealth::GOOD;
    status.rawData = std::string("test data");

    nlohmann::json j = status;
    ProbeStatus status2 = j.get<ProbeStatus>();

    EXPECT_EQ(status.health, status2.health);
    EXPECT_EQ(std::get<std::string>(status.rawData), std::get<std::string>(status2.rawData));
}



TEST(ProbeStatusTest, serializationOfSmartRawData)
{
    ProbeStatus status;
    status.health = GeneralHealth::BAD;
    status.rawData = SmartData{ .smartData =
        {
            {SmartData::TotalLBAsWritten, {1,2,3}},
            {SmartData::HighFlyWrites, {8,9,4}}
        }
    };

    nlohmann::json j = status;
    ProbeStatus status2 = j.get<ProbeStatus>();

    EXPECT_EQ(status.health, status2.health);
    EXPECT_EQ(std::get<SmartData>(status.rawData), std::get<SmartData>(status2.rawData));
}
