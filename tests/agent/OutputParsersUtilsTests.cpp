#include <gmock/gmock.h>

#include "OutputParsersUtils.h"


using testing::ElementsAre;


TEST(OutputParsersUtilsTest, trimsLinesAndDropsOuterEmptyLines)
{
    const auto result = ParsersUtils::clean("\n  header  \n\tvalue\t\n\n  tail  \n\n");

    EXPECT_THAT(result, ElementsAre("header", "value", "", "tail"));
}


TEST(OutputParsersUtilsTest, preservesInnerEmptyLines)
{
    const auto result = ParsersUtils::clean("first\n\nsecond");

    EXPECT_THAT(result, ElementsAre("first", "", "second"));
}


TEST(OutputParsersUtilsTest, returnsEmptyListForWhitespaceOnlyInput)
{
    EXPECT_TRUE(ParsersUtils::clean(" \n\t\n\r\n").empty());
    EXPECT_TRUE(ParsersUtils::clean("").empty());
}
