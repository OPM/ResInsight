#include "gtest/gtest.h"

#include "RiuSummaryVectorDescriptionMap.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiuSummaryVectorDescriptionMap, TestInit)
{
    {
        std::string s("SRSFC");
        auto test = RiuSummaryVectorDescriptionMap::instance()->fieldInfo(s);

        EXPECT_TRUE(test == "Reach brine concentration");
    }

    {
        std::string s("does not exist");
        auto test = RiuSummaryVectorDescriptionMap::instance()->fieldInfo(s);

        EXPECT_TRUE(test == s);
    }
}
