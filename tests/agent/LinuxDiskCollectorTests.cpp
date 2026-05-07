#include <gmock/gmock.h>

#include "linux/LinuxDiskCollector.h"


using testing::ElementsAre;


TEST(LinuxDiskCollectorTest, detectsPartitionsFromLsblkEntries)
{
    const std::vector<LsblkOutputParser::LsblkEntry> entries = {
        {"sda", "disk", 1000, {"sda1", "sda2"}, 8, 0},
        {"nvme0n1", "disk", 2000, {"nvme0n1p1"}, 259, 0},
    };

    LinuxDiskCollector collector(entries);

    EXPECT_TRUE(collector.isPartition("sda1"));
    EXPECT_TRUE(collector.isPartition("sda2"));
    EXPECT_TRUE(collector.isPartition("nvme0n1p1"));
    EXPECT_FALSE(collector.isPartition("sda"));
    EXPECT_FALSE(collector.isPartition("missing"));
}


TEST(LinuxDiskCollectorTest, mapsPartitionToParentDisk)
{
    const std::vector<LsblkOutputParser::LsblkEntry> entries = {
        {"sda", "disk", 1000, {"sda1", "sda2"}, 8, 0},
        {"sdb", "disk", 2000, {"sdb1"}, 8, 16},
    };

    LinuxDiskCollector collector(entries);

    EXPECT_EQ(collector.diskForPartition("sda1"), "sda");
    EXPECT_EQ(collector.diskForPartition("sda2"), "sda");
    EXPECT_EQ(collector.diskForPartition("sdb1"), "sdb");
    EXPECT_TRUE(collector.diskForPartition("sdc1").empty());
}


TEST(LinuxDiskCollectorTest, buildsDiskListFromEntries)
{
    const std::vector<LsblkOutputParser::LsblkEntry> entries = {
        {"unit-test-disk-a", "disk", 1000, {}, 8, 0},
        {"unit-test-disk-b", "disk", 2000, {}, 8, 16},
    };

    LinuxDiskCollector collector(entries);
    const auto disks = collector.GetDisksList();

    ASSERT_EQ(disks.size(), 2);
    EXPECT_EQ(disks[0].GetDeviceId(), "unit-test-disk-a");
    EXPECT_EQ(disks[0].GetCapacity(), 1000);
    EXPECT_EQ(disks[1].GetDeviceId(), "unit-test-disk-b");
    EXPECT_EQ(disks[1].GetCapacity(), 2000);
}
