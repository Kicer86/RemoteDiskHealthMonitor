
#include <gmock/gmock.h>

#include "linux/DmesgParser.h"
#include "IPartitionsManagerMock.h"


using testing::NiceMock;
using testing::Return;


TEST(DmesgParserTest, NoErrors)
{
    IPartitionsManagerMock pm;

    const auto errors = DmesgParser::parse(
    R"(
        [ 1444.081302] usb 2-1: new high-speed USB device number 8 using xhci_hcd
        [ 1444.266866] usb 2-1: New USB device found, idVendor=2717, idProduct=ff40, bcdDevice= 3.18
        [ 1444.266867] usb 2-1: New USB device strings: Mfr=1, Product=2, SerialNumber=3
        [ 1444.266868] usb 2-1: Product: Android
        [ 1444.266869] usb 2-1: Manufacturer: Android
        [ 1444.266869] usb 2-1: SerialNumber: 42bfcc2e7d64
        [ 1446.026294] ata4: SATA link up 6.0 Gbps (SStatus 133 SControl 300)
        [ 1446.066268] ata2: SATA link up 6.0 Gbps (SStatus 133 SControl 300)
        [ 1446.158564] ata4.00: configured for UDMA/133
        [ 1446.161245] ata2.00: configured for UDMA/133
    )",
    pm
    );

    EXPECT_TRUE(errors.empty());
}


TEST(DmesgParserTest, BufferIOError)
{
    NiceMock<IPartitionsManagerMock> pm;
    ON_CALL(pm, isPartition(std::string("sdb1"))).WillByDefault(Return(true));
    ON_CALL(pm, diskForPartition(std::string("sdb1"))).WillByDefault(Return("sdb"));

    const auto errors = DmesgParser::parse(
    R"(
        [19737.431050] Buffer I/O error on device sdb1, logical block 6160400
        [19737.431060] Buffer I/O error on device sdb1, logical block 6160401
        [19737.431067] Buffer I/O error on device sdb1, logical block 6160402
        [19737.431075] Buffer I/O error on device sdb1, logical block 6160403
        [19737.431082] Buffer I/O error on device sdb1, logical block 6160404
        [19737.431088] Buffer I/O error on device sdb1, logical block 6160405
        [19737.431096] Buffer I/O error on device sdb1, logical block 6160406
        [19737.431102] Buffer I/O error on device sdb1, logical block 6160407
        [19737.431114] Buffer I/O error on device sdb1, logical block 6160408
        [19737.431121] Buffer I/O error on device sdb1, logical block 6160409
    )",
    pm
    );

    const Disk failedDisk("sdb");

    ASSERT_EQ(errors.size(), 1);
    ASSERT_NE(errors.find(failedDisk), errors.end());
}


TEST(DmesgParserTest, MapsCommonKernelIoErrorsToPhysicalDisk)
{
    NiceMock<IPartitionsManagerMock> pm;
    ON_CALL(pm, isPartition(std::string("nvme0n1p1"))).WillByDefault(Return(true));
    ON_CALL(pm, diskForPartition(std::string("nvme0n1p1"))).WillByDefault(Return("nvme0n1"));
    ON_CALL(pm, isPartition(std::string("sdb1"))).WillByDefault(Return(true));
    ON_CALL(pm, diskForPartition(std::string("sdb1"))).WillByDefault(Return("sdb"));

    const auto errors = DmesgParser::parse(
    R"(
        [  100.123456] blk_update_request: I/O error, dev nvme0n1p1, sector 123456 op 0x0:(READ)
        [  101.123456] blk_update_request: critical medium error, dev sda, sector 999 op 0x0:(READ)
        [  102.123456] EXT4-fs error (device sdb1): ext4_find_entry:1456: inode #2: comm systemd: reading directory lblock 0
    )",
    pm
    );

    ASSERT_EQ(errors.size(), 3);
    EXPECT_NE(errors.find(Disk("nvme0n1")), errors.end());
    EXPECT_NE(errors.find(Disk("sda")), errors.end());
    EXPECT_NE(errors.find(Disk("sdb")), errors.end());
}


TEST(DmesgParserTest, KeepsOriginalDeviceWhenPartitionMappingIsMissing)
{
    NiceMock<IPartitionsManagerMock> pm;
    ON_CALL(pm, isPartition(std::string("dm-0"))).WillByDefault(Return(true));
    ON_CALL(pm, diskForPartition(std::string("dm-0"))).WillByDefault(Return(""));

    const auto errors = DmesgParser::parse(
    R"(
        [  100.123456] Buffer I/O error on dev dm-0, logical block 12345
    )",
    pm
    );

    ASSERT_EQ(errors.size(), 1);
    EXPECT_NE(errors.find(Disk("dm-0")), errors.end());
}
