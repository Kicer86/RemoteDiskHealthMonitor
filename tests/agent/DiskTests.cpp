#include <gmock/gmock.h>

#include "Disk.h"


TEST(DiskTest, exposesConstructorValues)
{
    Disk disk("sda", "Samsung SSD 870", 1024, "SSD");

    EXPECT_EQ(disk.GetDeviceId(), "sda");
    EXPECT_EQ(disk.GetModel(), "Samsung SSD 870");
    EXPECT_EQ(disk.GetVendor(), "Samsung");
    EXPECT_EQ(disk.GetCapacity(), 1024);
    EXPECT_EQ(disk.GetDriveType(), "SSD");
}


TEST(DiskTest, detectsKnownVendorsFromModel)
{
    const std::vector<std::pair<std::string, std::string>> cases = {
        {"Samsung SSD 870", "Samsung"},
        {"WDC WD10EFRX", "WDC"},
        {"Western Digital Blue", "WDC"},
        {"ST4000DM004", "Seagate"},
        {"Seagate IronWolf", "Seagate"},
        {"TOSHIBA HDWD110", "Toshiba"},
        {"HGST HUS724040", "HGST"},
        {"Hitachi HDS721010", "HGST"},
        {"Intel SSDSC2", "Intel"},
        {"Crucial MX500", "Micron"},
        {"Micron 1100", "Micron"},
        {"Kingston SA400", "Kingston"},
        {"SanDisk SDSSDA", "SanDisk"},
    };

    for (const auto& [model, expectedVendor] : cases)
    {
        const Disk disk("disk", model);
        EXPECT_EQ(disk.GetVendor(), expectedVendor) << model;
    }
}


TEST(DiskTest, returnsEmptyVendorForUnknownOrShortModel)
{
    EXPECT_TRUE(Disk("disk", "Generic Model").GetVendor().empty());
    EXPECT_TRUE(Disk("disk", "S").GetVendor().empty());
    EXPECT_TRUE(Disk("disk", "").GetVendor().empty());
}
