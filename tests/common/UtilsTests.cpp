#include <gmock/gmock.h>

#include "Utils.h"


TEST(UtilsTest, formatsBytesUsingBinaryUnits)
{
    EXPECT_EQ(formatBytes(0), "0 B");
    EXPECT_EQ(formatBytes(1023), "1023 B");
    EXPECT_EQ(formatBytes(1024), "1.00 KiB");
    EXPECT_EQ(formatBytes(1536), "1.50 KiB");
    EXPECT_EQ(formatBytes(1024ULL * 1024ULL * 1024ULL), "1.00 GiB");
}


TEST(UtilsTest, returnsEmptyTableForNoRows)
{
    std::vector<std::vector<std::string>> rows;

    EXPECT_TRUE(formatTable(rows).empty());
}


TEST(UtilsTest, formatsRowsIntoPaddedColumns)
{
    std::vector<std::vector<std::string>> rows = {
        {"Name", "Health"},
        {"sda", "GOOD"},
        {"nvme0n1", "BAD"},
    };

    EXPECT_EQ(formatTable(rows),
              "Name    Health \n"
              "sda     GOOD   \n"
              "nvme0n1 BAD    \n");
}
