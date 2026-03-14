
#include <gmock/gmock.h>

#include "common/ProbeStatus.h"
#include "common/JsonSerialize.h"


TEST(ProbeStatusTest, serializationOfStringRawData)
{
    ProbeStatus status;
    status.health = GeneralHealth::GOOD;
    status.rawData = nlohmann::json{{"type", "text"}, {"value", "test data"}};

    nlohmann::json j = status;
    ProbeStatus status2 = j.get<ProbeStatus>();

    EXPECT_EQ(status.health, status2.health);
    EXPECT_EQ(status.rawData, status2.rawData);
}



TEST(ProbeStatusTest, serializationOfSmartRawData)
{
    SmartData smart{ .attributes = {
        {0xF1, "Total_LBAs_Written", 1, 2, 0, 3},
        {0xBD, "High_Fly_Writes", 8, 9, 0, 4}
    }};

    nlohmann::json smartJson;
    to_json(smartJson, smart);
    smartJson["type"] = "smart";

    ProbeStatus status;
    status.health = GeneralHealth::BAD;
    status.rawData = smartJson;

    nlohmann::json j = status;
    ProbeStatus status2 = j.get<ProbeStatus>();

    EXPECT_EQ(status.health, status2.health);
    EXPECT_EQ(status.rawData, status2.rawData);
}
