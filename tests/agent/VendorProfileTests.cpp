#include <gmock/gmock.h>

#include "VendorProfile.h"


TEST(VendorProfileTest, genericProfileReturnsRawValue)
{
    GenericProfile profile;

    EXPECT_EQ(profile.interpretRawValue(0x01, 0x112233445566LL), 0x112233445566LL);
    EXPECT_EQ(profile.interpretRawValue(0xC3, -10), -10);
}


TEST(VendorProfileTest, samsungProfileUsesLower32BitsForPackedAttributes)
{
    SamsungProfile profile;

    EXPECT_EQ(profile.interpretRawValue(0x01, 0x112233445566LL), 0x33445566LL);
    EXPECT_EQ(profile.interpretRawValue(0x07, 0x112233445566LL), 0x33445566LL);
    EXPECT_EQ(profile.interpretRawValue(0x05, 0x112233445566LL), 0x112233445566LL);
}


TEST(VendorProfileTest, seagateProfileUsesLower32BitsForPackedAttributes)
{
    SeagateProfile profile;

    EXPECT_EQ(profile.interpretRawValue(0x01, 0x112233445566LL), 0x33445566LL);
    EXPECT_EQ(profile.interpretRawValue(0x07, 0x112233445566LL), 0x33445566LL);
    EXPECT_EQ(profile.interpretRawValue(0xC3, 0x112233445566LL), 0x33445566LL);
    EXPECT_EQ(profile.interpretRawValue(0x05, 0x112233445566LL), 0x112233445566LL);
}
