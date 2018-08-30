#include "gtest/gtest.h"

#include "RiuSummaryVectorDescriptionMap.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiuSummaryVectorDescriptionMap, TestInit)
{
    {
        std::string s("SRSFC");
        auto test = RiuSummaryVectorDescriptionMap::instance()->vectorInfo(s);

        EXPECT_TRUE(test.category == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT);
        EXPECT_TRUE(test.longName == "Reach brine concentration");
    }

    {
        std::string s("SRSFC");
        auto test = RiuSummaryVectorDescriptionMap::instance()->vectorLongName(s);

        EXPECT_TRUE(test == "Reach brine concentration");
    }

    {
        std::string s("does not exist");
        auto test = RiuSummaryVectorDescriptionMap::instance()->vectorInfo(s);

        EXPECT_TRUE(test.category == RifEclipseSummaryAddress::SUMMARY_INVALID);
        EXPECT_TRUE(test.longName == "");
    }

    {
        std::string s("does not exist");
        auto test = RiuSummaryVectorDescriptionMap::instance()->vectorLongName(s);

        EXPECT_TRUE(test == "");
    }

    {
        std::string s("does not exist");
        auto test = RiuSummaryVectorDescriptionMap::instance()->vectorLongName(s, true);

        EXPECT_TRUE(test == s);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiuSummaryVectorDescriptionMap, TestCustomNaming)
{
    {
        std::string s("SRSFCABC");
        auto test = RiuSummaryVectorDescriptionMap::instance()->vectorInfo(s);

        EXPECT_TRUE(test.category == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT);
        EXPECT_TRUE(test.longName == "Reach brine concentration");
    }

    {
        std::string s("BHD__ABC");
        auto test = RiuSummaryVectorDescriptionMap::instance()->vectorLongName(s);

        EXPECT_TRUE(test == "Hydraulic head");
    }
}
