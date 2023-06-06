
#include <QIODevice>

#include <gmock/gmock.h>

#include "ProbeStatus.h"
#include "ProbeStatusSerialize.h"


TEST(ProbeStatusTest, serializationOfJSonData)
{
    ProbeStatus status;
    status.health = GeneralHealth::GOOD;
    status.jsonData = "test data";

    QByteArray array;

    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << status;

    ProbeStatus status2;
    QDataStream stream2(&array, QIODevice::ReadOnly);
    stream2 >> status2;

    EXPECT_EQ(status.health, status2.health);
    EXPECT_EQ(status.jsonData, status2.jsonData);
}
