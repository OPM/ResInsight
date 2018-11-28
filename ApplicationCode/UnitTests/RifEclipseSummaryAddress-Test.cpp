#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"

#include <string>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Field)
{
    std::string addrString = "FOPT";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_FIELD, addr.category());
    EXPECT_EQ("FOPT", addr.quantityName());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Aquifer)
{
    std::string addrString = "AAQR:456";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_AQUIFER, addr.category());
    EXPECT_EQ("AAQR", addr.quantityName());
    EXPECT_EQ(456, addr.aquiferNumber());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Network)
{
    std::string addrString = "NETW";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_NETWORK, addr.category());
    EXPECT_EQ("NETW", addr.quantityName());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, DISABLED_TestEclipseAddressParsing_Misc)
{
    std::string addrString = "CPU";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_MISC, addr.category());
    EXPECT_EQ("CPU", addr.quantityName());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Region)
{
    std::string addrString = "RPR:7081";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_REGION, addr.category());
    EXPECT_EQ("RPR", addr.quantityName());
    EXPECT_EQ(7081, addr.regionNumber());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_RegionToRegion)
{
    std::string addrString = "ROFR:7081-8001";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION, addr.category());
    EXPECT_EQ("ROFR", addr.quantityName());
    EXPECT_EQ(7081, addr.regionNumber());
    EXPECT_EQ(8001, addr.regionNumber2());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellGroup)
{
    std::string addrString = "GOPR:WELLS1";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL_GROUP, addr.category());
    EXPECT_EQ("GOPR", addr.quantityName());
    EXPECT_EQ("WELLS1", addr.wellGroupName());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Well)
{
    std::string addrString = "WOPR:B-2H";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL, addr.category());
    EXPECT_EQ("WOPR", addr.quantityName());
    EXPECT_EQ("B-2H", addr.wellName());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellCompletion)
{
    std::string addrString = "COFRL:B-1H:15,13,14";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION, addr.category());
    EXPECT_EQ("COFRL", addr.quantityName());
    EXPECT_EQ("B-1H", addr.wellName());
    EXPECT_EQ(15, addr.cellI());
    EXPECT_EQ(13, addr.cellJ());
    EXPECT_EQ(14, addr.cellK());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellLgr)
{
    std::string addrString = "LWABC:LGRNA:B-10H";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL_LGR, addr.category());
    EXPECT_EQ("LWABC", addr.quantityName());
    EXPECT_EQ("LGRNA", addr.lgrName());
    EXPECT_EQ("B-10H", addr.wellName());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellCompletionLgr)
{
    std::string addrString = "LCGAS:LGR1:B-1H:11,12,13";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR, addr.category());
    EXPECT_EQ("LCGAS", addr.quantityName());
    EXPECT_EQ("LGR1", addr.lgrName());
    EXPECT_EQ("B-1H", addr.wellName());
    EXPECT_EQ(11, addr.cellI());
    EXPECT_EQ(12, addr.cellJ());
    EXPECT_EQ(13, addr.cellK());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_WellSegment)
{
    std::string addrString = "SOFR:B-5H:32";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT, addr.category());
    EXPECT_EQ("SOFR", addr.quantityName());
    EXPECT_EQ("B-5H", addr.wellName());
    EXPECT_EQ(32, addr.wellSegmentNumber());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Block)
{
    std::string addrString = "BPR:123,122,121";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_BLOCK, addr.category());
    EXPECT_EQ("BPR", addr.quantityName());
    EXPECT_EQ(123, addr.cellI());
    EXPECT_EQ(122, addr.cellJ());
    EXPECT_EQ(121, addr.cellK());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_BlockLgr)
{
    std::string addrString = "LBABC:LGRN:45,47,49";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR, addr.category());
    EXPECT_EQ("LBABC", addr.quantityName());
    EXPECT_EQ("LGRN", addr.lgrName());
    EXPECT_EQ(45, addr.cellI());
    EXPECT_EQ(47, addr.cellJ());
    EXPECT_EQ(49, addr.cellK());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_Imported)
{
    std::string addrString = "FAULT (Imp)";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_IMPORTED, addr.category());
    EXPECT_EQ("FAULT (Imp)", addr.quantityName());
    EXPECT_FALSE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_ErrorResult1)
{
    std::string addrString = "ER:AAQR:456";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_AQUIFER, addr.category());
    EXPECT_EQ("AAQR", addr.quantityName());
    EXPECT_EQ(456, addr.aquiferNumber());
    EXPECT_TRUE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_ErrorResult2)
{
    std::string addrString = "ERR:LCGAS:LGR1:B-1H:11,12,13";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR, addr.category());
    EXPECT_EQ("LCGAS", addr.quantityName());
    EXPECT_EQ("LGR1", addr.lgrName());
    EXPECT_EQ("B-1H", addr.wellName());
    EXPECT_EQ(11, addr.cellI());
    EXPECT_EQ(12, addr.cellJ());
    EXPECT_EQ(13, addr.cellK());
    EXPECT_TRUE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressParsing_ErrorResult3)
{
    std::string addrString = "ERROR:FAULT (Imp)";

    RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddress(addrString);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_IMPORTED, addr.category());
    EXPECT_EQ("FAULT (Imp)", addr.quantityName());
    EXPECT_TRUE(addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressIjkParsing)
{
    RifEclipseSummaryAddress::SummaryVarCategory cat = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
    std::map<RifEclipseSummaryAddress::SummaryIdentifierType, std::string> identifiers(
        {
            { RifEclipseSummaryAddress::INPUT_WELL_NAME, "1-BH" },
            { RifEclipseSummaryAddress::INPUT_CELL_IJK, "6, 7, 8" },
            { RifEclipseSummaryAddress::INPUT_VECTOR_NAME, "WOPR" },
        }
    );

    RifEclipseSummaryAddress addr(cat, identifiers);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION, addr.category());
    EXPECT_EQ("WOPR", addr.quantityName());
    EXPECT_EQ("1-BH", addr.wellName());
    EXPECT_EQ(6, addr.cellI());
    EXPECT_EQ(7, addr.cellJ());
    EXPECT_EQ(8, addr.cellK());
    EXPECT_TRUE(!addr.isErrorResult());
}

TEST(RifEclipseSummaryAddressTest, TestEclipseAddressRegToRegParsing)
{
    RifEclipseSummaryAddress::SummaryVarCategory cat = RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION;
    std::map<RifEclipseSummaryAddress::SummaryIdentifierType, std::string> identifiers(
        {
            { RifEclipseSummaryAddress::INPUT_REGION_2_REGION, "123 - 456" },
            { RifEclipseSummaryAddress::INPUT_VECTOR_NAME, "ROFR" },
        }
    );

    RifEclipseSummaryAddress addr(cat, identifiers);

    EXPECT_TRUE(addr.isValid());
    EXPECT_EQ(RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION, addr.category());
    EXPECT_EQ("ROFR", addr.quantityName());
    EXPECT_EQ(123, addr.regionNumber());
    EXPECT_EQ(456, addr.regionNumber2());
    EXPECT_TRUE(!addr.isErrorResult());
}
