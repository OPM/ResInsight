#include "gtest/gtest.h"

#include "RifEclipseUserDataKeywordTools.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseUserDataKeywordToolsTest, TestIdentifierItemsPerLine)
{
    {
        std::string s = "AA";
        EXPECT_EQ(size_t(0), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s).size());
    }
    {
        std::string s = "BB";
        EXPECT_EQ(size_t(3), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
    }
    {
        std::string s = "CC";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
        EXPECT_EQ(size_t(3), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[1]);
    }
    {
        std::string s = "FF";
        EXPECT_EQ(size_t(0), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s).size());
    }
    {
        std::string s = "GG";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
    }
    {
        std::string s = "NN";
        EXPECT_EQ(size_t(0), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s).size());
    }
    {
        std::string s = "RR";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
    }
    {
        std::string s = "SS";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[1]);
    }
    {
        std::string s = "WW";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
    }
    {
        std::string s = "LB";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
        EXPECT_EQ(size_t(3), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[1]);
    }
    {
        std::string s = "LC";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[1]);
        EXPECT_EQ(size_t(3), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[2]);
    }
    {
        std::string s = "LW";
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[0]);
        EXPECT_EQ(size_t(1), RifEclipseUserDataKeywordTools::requiredItemsPerLineForKeyword(s)[1]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseUserDataKeywordToolsTest, BuildTableHeaderText)
{
    std::vector<std::string> keywordNames   = { "TIME", "YEARX", "WGT1",    "WR42" };
    std::vector<std::string> firstheader    = {                  "OP-1",    "OP-1" };
    std::vector<std::vector<std::string>> headerLines = { firstheader };

    auto tableHeaderData = RifEclipseUserDataKeywordTools::buildColumnHeaderText(keywordNames, headerLines);
    EXPECT_EQ(size_t(4), tableHeaderData.size());
    EXPECT_EQ(size_t(1), tableHeaderData[0].size());
    EXPECT_EQ(size_t(1), tableHeaderData[1].size());
    EXPECT_EQ(size_t(2), tableHeaderData[2].size());
    EXPECT_EQ(size_t(2), tableHeaderData[3].size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseUserDataKeywordToolsTest, BuildTableHeaderTextComplex)
{
    std::vector<std::string> keywordNames   = { "TIME", "WGT1",    "FVIR",      "RPR",  "GOPR",     "CWIR",         "FTPTS36", "CWIR"           };
    std::vector<std::string> firstheader    = {         "OP-1",                 "8",    "MANI-D2",  "F-2H",                    "2H"             };
    std::vector<std::string> secondHeader   = {                                                     "18", "83","3",            "9", "8","7"     };
    std::vector<std::vector<std::string>> headerLines = { firstheader, secondHeader };

    auto tableHeaderData = RifEclipseUserDataKeywordTools::buildColumnHeaderText(keywordNames, headerLines);
    EXPECT_EQ(size_t(8), tableHeaderData.size());
    
    EXPECT_EQ(size_t(1), tableHeaderData[0].size());
    EXPECT_EQ(size_t(2), tableHeaderData[1].size());
    EXPECT_EQ(size_t(1), tableHeaderData[2].size());
    EXPECT_EQ(size_t(2), tableHeaderData[3].size());
    EXPECT_EQ(size_t(2), tableHeaderData[4].size());
    EXPECT_EQ(size_t(5), tableHeaderData[5].size());
    EXPECT_EQ(size_t(1), tableHeaderData[6].size());
    EXPECT_EQ(size_t(5), tableHeaderData[7].size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseUserDataKeywordToolsTest, MissingHeaderData)
{
    {
        std::vector<std::string> keywordNames   = { "TIME", "WGT1"      };
        std::vector<std::string> firstheader    = {         }; // Missing well name
        std::vector<std::vector<std::string>> headerLines = { firstheader };

        auto tableHeaderData = RifEclipseUserDataKeywordTools::buildColumnHeaderText(keywordNames, headerLines);
        EXPECT_EQ(size_t(0), tableHeaderData.size());
    }


    {
        std::vector<std::string> keywordNames   = { "TIME", "WGT1",    "FVIR",      "RPR",  "GOPR",     "CWIR",         "FTPTS36", "CWIR"           };
        std::vector<std::string> firstheader    = {         "OP-1",                 "8",    "MANI-D2",  "F-2H",                    "2H"             };
        std::vector<std::string> secondHeader   = {                                                     "18", "83","3",            "9", "8"         }; // Missing value from last triplet
        std::vector<std::vector<std::string>> headerLines = { firstheader, secondHeader };

        auto tableHeaderData = RifEclipseUserDataKeywordTools::buildColumnHeaderText(keywordNames, headerLines);
        EXPECT_EQ(size_t(0), tableHeaderData.size());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseUserDataKeywordToolsTest, CreationOfSummaryAddresses)
{
    // Region
    {
        std::string quantity = "RGT1";
        std::vector<std::string> columnData = {"1"};

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_REGION);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_EQ(1, address.regionNumber());
    }

    // Well group
    {
        std::string quantity = "GT1";
        std::vector<std::string> columnData = { "OP-1" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_WELL_GROUP);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_STREQ(columnData[0].data(), address.wellGroupName().data());
    }


    // Well
    {
        std::string quantity = "WGT1";
        std::vector<std::string> columnData = { "OP-1" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_WELL);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_STREQ(columnData[0].data(), address.wellName().data());
    }
     
    // Well completion
    {
        std::string quantity = "CWIT";
        std::vector<std::string> columnData = { "F-3H", "1", "2", "3" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_STREQ(columnData[0].data(), address.wellName().data());
        EXPECT_EQ(1, address.cellI());
        EXPECT_EQ(2, address.cellJ());
        EXPECT_EQ(3, address.cellK());
    }

    // Well LGR
    {
        std::string quantity = "LWGT1";
        std::vector<std::string> columnData = { "OP-1", "LGR-NAME" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_WELL_LGR);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_STREQ(columnData[0].data(), address.wellName().data());
        EXPECT_STREQ(columnData[1].data(), address.lgrName().data());
    }

    // Well completion LGR
    {
        std::string quantity = "LC";
        std::vector<std::string> columnData = { "F-3H", "LGR-NAME", "1", "2", "3" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_STREQ(columnData[0].data(), address.wellName().data());
        EXPECT_STREQ(columnData[1].data(), address.lgrName().data());
        EXPECT_EQ(1, address.cellI());
        EXPECT_EQ(2, address.cellJ());
        EXPECT_EQ(3, address.cellK());
    }

    // Well segment
    {
        std::string quantity = "SCWIT";
        std::vector<std::string> columnData = { "F-3H", "1" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_STREQ(columnData[0].data(), address.wellName().data());
        EXPECT_EQ(1, address.wellSegmentNumber());
    }

    // Block
    {
        std::string quantity = "BWIT";
        std::vector<std::string> columnData = { "1", "2", "3" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_BLOCK);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_EQ(1, address.cellI());
        EXPECT_EQ(2, address.cellJ());
        EXPECT_EQ(3, address.cellK());
    }

    // Block LGR
    {
        std::string quantity = "LBWIT";
        std::vector<std::string> columnData = { "LGR-name", "1", "2", "3" };

        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
        EXPECT_STREQ(columnData[0].data(), address.lgrName().data());
        EXPECT_EQ(1, address.cellI());
        EXPECT_EQ(2, address.cellJ());
        EXPECT_EQ(3, address.cellK());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseUserDataKeywordToolsTest, CreationOfMisc)
{
    // Misc
    {
        std::string quantity = "JI-NOT-REQOGNIZED";
        std::vector<std::string> columnData = {  };
        auto address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantity, columnData);

        EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_MISC);
        EXPECT_STREQ(quantity.data(), address.quantityName().data());
    }
}
