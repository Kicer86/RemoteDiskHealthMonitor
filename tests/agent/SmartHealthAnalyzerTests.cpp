#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SmartHealthAnalyzer.h"
#include "ISmartReaderMock.h"
#include "DiskMock.h"
#include "common/SmartData.h"

using testing::Return;
using testing::_;

namespace
{
    SmartData::Attribute makeAttr(uint8_t id, int value, int worst, int threshold, int64_t rawVal)
    {
        return {id, SmartData::GetCanonicalName(id), value, worst, threshold, rawVal};
    }
}


class SmartHealthAnalyzerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        auto readerPtr = std::make_unique<ISmartReaderMock>();
        m_reader = readerPtr.get();
        m_analyzer = std::make_unique<SmartHealthAnalyzer>(std::move(readerPtr));
    }

    ISmartReaderMock* m_reader = nullptr;
    std::unique_ptr<SmartHealthAnalyzer> m_analyzer;
    DiskMock m_disk;
};


TEST_F(SmartHealthAnalyzerTest, AllHealthyAttributesReturnGood)
{
    SmartData data;
    data.attributes = {
        makeAttr(0x01, 100, 100, 50, 0),
        makeAttr(0x05, 100, 100, 36, 0),
        makeAttr(0x09, 90, 90, 0, 12345),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, ValueBelowThresholdReturnsBad)
{
    SmartData data;
    data.attributes = {
        makeAttr(0x01, 100, 100, 50, 0),
        makeAttr(0x05, 30, 30, 36, 5),   // value <= threshold
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::BAD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, ValueEqualToThresholdReturnsBad)
{
    SmartData data;
    data.attributes = {
        makeAttr(0x05, 36, 36, 36, 3),   // value == threshold
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::BAD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, CriticalAttributeWithNonZeroRawReturnsCheckStatus)
{
    SmartData data;
    data.attributes = {
        makeAttr(0x05, 100, 100, 36, 4),   // Reallocated_Sector_Ct, raw > 0
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::CHECK_STATUS, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, ProximityToThresholdReturnsCheckStatus)
{
    SmartData data;
    data.attributes = {
        // threshold=100, 15% margin=15, so value <= 115 triggers proximity
        makeAttr(0x01, 110, 110, 100, 0),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::CHECK_STATUS, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, ValueWellAboveThresholdIsGood)
{
    SmartData data;
    data.attributes = {
        makeAttr(0x01, 200, 200, 50, 0),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, SamsungProfileMasksRawReadErrorRate)
{
    // Samsung drives pack counters into upper bytes of raw value for ID 1.
    // A huge raw value that looks alarming is actually fine after masking.
    DiskMock samsungDisk("sda", "Samsung SSD 860");

    SmartData data;
    data.attributes = {
        // Raw value with upper bits set, but lower 32 bits are 0 → no real errors
        makeAttr(0x01, 100, 100, 51, 0x00000A0000000000LL),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({samsungDisk});

    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(samsungDisk));
}


TEST_F(SmartHealthAnalyzerTest, SamsungProfileDetectsRealErrors)
{
    DiskMock samsungDisk("sda", "Samsung SSD 860");

    SmartData data;
    data.attributes = {
        // Lower 32 bits are non-zero → real errors, but ID 1 is not critical → still GOOD
        makeAttr(0x01, 100, 100, 51, 0x00000A0000000005LL),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({samsungDisk});

    // ID 1 is not in the critical list, so non-zero raw alone doesn't trigger
    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(samsungDisk));
}


TEST_F(SmartHealthAnalyzerTest, SeagateProfileMasksSeekErrorRate)
{
    DiskMock seagateDisk("sda", "Seagate Barracuda");

    SmartData data;
    data.attributes = {
        // Seagate ID 7 (Seek_Error_Rate): huge packed value, lower 32 bits = 0
        makeAttr(0x07, 80, 80, 30, 0x0000FF0000000000LL),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, UnknownVendorUsesGenericProfile)
{
    DiskMock unknownDisk("sda", "SomeBrand XYZ");

    SmartData data;
    data.attributes = {
        // With generic profile, full raw value is used.
        // ID 5 is critical; raw > 0 → CHECK_STATUS
        makeAttr(0x05, 100, 100, 36, 2),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::CHECK_STATUS, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, GetRawDataReturnsSmartJsonFormat)
{
    SmartData data;
    data.attributes = {
        makeAttr(0x01, 100, 99, 50, 0),
        makeAttr(0x05, 98, 97, 36, 2),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    auto json = m_analyzer->GetRawData(m_disk);

    EXPECT_EQ("smart", json["type"]);
    ASSERT_TRUE(json["attributes"].is_array());
    ASSERT_EQ(2u, json["attributes"].size());

    EXPECT_EQ(0x01, json["attributes"][0]["id"]);
    EXPECT_EQ(100, json["attributes"][0]["value"]);
    EXPECT_EQ(99, json["attributes"][0]["worst"]);
    EXPECT_EQ(50, json["attributes"][0]["threshold"]);
    EXPECT_EQ(0, json["attributes"][0]["rawVal"]);

    EXPECT_EQ(0x05, json["attributes"][1]["id"]);
    EXPECT_EQ(2, json["attributes"][1]["rawVal"]);

    ASSERT_TRUE(json.contains("selfTestStatus"));
    EXPECT_FALSE(json["selfTestStatus"]["running"]);
}


TEST_F(SmartHealthAnalyzerTest, EmptySmartDataReturnsGood)
{
    SmartData data;  // no attributes

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, ZeroThresholdSkipsThresholdChecks)
{
    SmartData data;
    data.attributes = {
        // threshold=0 means "no threshold" — should not trigger BAD even if value is low
        makeAttr(0x09, 1, 1, 0, 50000),
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(m_disk));
}


// ─── NVMe-specific tests ───

TEST_F(SmartHealthAnalyzerTest, NvmeCriticalWarningNonZeroReturnsBad)
{
    SmartData data;
    data.attributes = {
        {1, "Critical_Warning", 0, 0, 0, 1},
        {2, "Temperature", 0, 0, 0, 45},
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::BAD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, NvmeMediaIntegrityErrorsReturnsBad)
{
    SmartData data;
    data.attributes = {
        {1, "Critical_Warning", 0, 0, 0, 0},
        {2, "Media_and_Data_Integrity_Errors", 0, 0, 0, 3},
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::BAD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, NvmePercentageUsedHighReturnsCheckStatus)
{
    SmartData data;
    data.attributes = {
        {1, "Critical_Warning", 0, 0, 0, 0},
        {2, "Percentage_Used", 0, 0, 0, 92},
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::CHECK_STATUS, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, NvmePercentageUsed100ReturnsBad)
{
    SmartData data;
    data.attributes = {
        {1, "Critical_Warning", 0, 0, 0, 0},
        {2, "Percentage_Used", 0, 0, 0, 100},
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::BAD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, NvmeAvailableSpareThresholdBreachReturnsBad)
{
    SmartData data;
    data.attributes = {
        // Available Spare at 5%, threshold 10% → value <= threshold → BAD
        {1, "Available_Spare", 5, 5, 10, 5},
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::BAD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, NvmeHealthyReturnGood)
{
    SmartData data;
    data.attributes = {
        {1, "Critical_Warning", 0, 0, 0, 0},
        {2, "Temperature", 0, 0, 0, 40},
        {3, "Available_Spare", 100, 100, 10, 100},
        {4, "Available_Spare_Threshold", 0, 0, 0, 10},
        {5, "Percentage_Used", 0, 0, 0, 5},
        {6, "Media_and_Data_Integrity_Errors", 0, 0, 0, 0},
    };

    EXPECT_CALL(*m_reader, ReadSMARTData(_)).WillOnce(Return(data));
    EXPECT_CALL(*m_reader, ReadTestStatus(_)).WillOnce(Return(SmartTestStatus{}));
    m_analyzer->Refresh({m_disk});

    EXPECT_EQ(GeneralHealth::GOOD, m_analyzer->GetStatus(m_disk));
}


TEST_F(SmartHealthAnalyzerTest, RefreshPolicyReturnsExpectedValues)
{
    auto policy = m_analyzer->GetRefreshPolicy();
    EXPECT_EQ(std::chrono::hours(4), policy.interval);
    EXPECT_FALSE(policy.proactiveCollection);
}
