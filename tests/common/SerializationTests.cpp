
#include <gtest/gtest.h>

#include "DiskInfoSerialize.h"


TEST(SerializationTest, DiskInfo)
{
    const DiskInfo di("disk #1", GeneralHealth::CHECK_STATUS, {
        ProbeStatus{.health = GeneralHealth::GOOD, .jsonData = "Some data"},
        ProbeStatus{.health = GeneralHealth::BAD, .jsonData = "Some other data"}
    });

    QByteArray data;
    {
        QDataStream ds(&data, QDataStream::WriteOnly);
        ds << di;
    }

    DiskInfo di2;
    QDataStream ds(data);

    ds >> di2;

    EXPECT_EQ(di, di2);
}


TEST(SerializationTest, DiskInfoList)
{
    const std::vector<DiskInfo> di_vec = {
        DiskInfo("disk #1", GeneralHealth::CHECK_STATUS, {
            ProbeStatus{.health = GeneralHealth::GOOD, .jsonData = "Some data"},
            ProbeStatus{.health = GeneralHealth::BAD, .jsonData = "Some other data"}
        })
    };

    QByteArray data;
    {
        QDataStream ds(&data, QDataStream::WriteOnly);
        ds << di_vec;
    }

    std::vector<DiskInfo> di_vec2;
    QDataStream ds(data);

    ds >> di_vec2;

    EXPECT_EQ(di_vec, di_vec2);
}
