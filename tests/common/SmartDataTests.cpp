#include <gmock/gmock.h>

#include "SmartData.h"


TEST(SmartDataTest, returnsCanonicalNameForKnownAttribute)
{
    EXPECT_EQ(SmartData::GetCanonicalName(0x01), "Raw_Read_Error_Rate");
    EXPECT_EQ(SmartData::GetCanonicalName(0x05), "Reallocated_Sector_Ct");
    EXPECT_EQ(SmartData::GetCanonicalName(0xC5), "Current_Pending_Sector");
    EXPECT_EQ(SmartData::GetCanonicalName(0xFE), "Free_Fall_Sensor");
}


TEST(SmartDataTest, returnsStableUnknownNameForUnknownAttribute)
{
    EXPECT_EQ(SmartData::GetCanonicalName(0x80), "Unknown_Attribute_128");
    EXPECT_EQ(SmartData::GetCanonicalName(0xFF), "Unknown_Attribute_255");
}
