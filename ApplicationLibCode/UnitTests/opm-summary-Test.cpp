#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifOpmCommonSummary.h"

#include "opm/io/eclipse/ERft.hpp"
#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

static const QString H5_TEST_DATA_DIRECTORY = QString( "%1/h5-file/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( OpmSummaryTests, ReadOpmSummaryDataListContent )
{
    std::vector<std::string> esmryKeywords;
    {
        QString filePath = H5_TEST_DATA_DIRECTORY + "NORNE_ATW2013_RFTPLT_V2.SMSPEC";

        Opm::EclIO::ESmry eSmry( filePath.toStdString() );

        esmryKeywords = eSmry.keywordList();
        eSmry.make_esmry_file();
    }

    std::vector<std::string> extEsmryKeywords;
    {
        QString filePath = H5_TEST_DATA_DIRECTORY + "NORNE_ATW2013_RFTPLT_V2.ESMRY";

        Opm::EclIO::ExtESmry extEsmry( filePath.toStdString() );

        extEsmryKeywords = extEsmry.keywordList();
    }

    EXPECT_EQ( esmryKeywords.size(), extEsmryKeywords.size() );
    for ( size_t i = 0; i < esmryKeywords.size(); i++ )
    {
        auto s1 = esmryKeywords[i];
        auto s2 = extEsmryKeywords[i];
        EXPECT_STREQ( s1.c_str(), s2.c_str() );

        RifEclipseSummaryAddress eclAdr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( s1 );
        EXPECT_TRUE( eclAdr.isValid() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( OpmSummaryTests, OpmImportRftData )
{
    std::vector<std::string> esmryKeywords;
    {
        // QString filePath = "e:/gitroot/opm-tests/model1/opm-simulation-reference/flow/MSW_MODEL_1.RFT";
        // QString filePath = "e:/gitroot/opm-tests/norne/ECL.2014.2/NORNE_ATW2013.RFT";
        // QString filePath = "d:/Models/Statoil/MSW-RFTfile/NORNE_ATW2013_RFTPLT_MSW.RFT";
        QString filePath = "e:/models/from_equinor_sftp/MSW-RFTfile/NORNE_ATW2013_RFTPLT_MSW.RFT";

        Opm::EclIO::ERft eRft( filePath.toStdString() );

        auto wells   = eRft.listOfWells();
        auto dates   = eRft.listOfdates();
        auto reports = eRft.listOfRftReports();

        std::cout << "\nWells:\n";
        for ( const auto& w : wells )
        {
            std::cout << w << "\n";
        }

        std::cout << "\nDates:\n";
        for ( const auto& date : dates )
        {
            auto [year, month, day] = date;

            std::cout << year << ", " << month << ", " << day << "\n";
        }

        std::cout << "\nReports:\n";
        for ( const auto& report : reports )
        {
            auto [text, date, floatValue] = report;

            std::cout << text << ", " << floatValue << "\n";
        }

        std::cout << "\nRFT Arrays:\n";
        for ( int i = 0; i < eRft.numberOfReports(); i++ )
        {
            std::cout << "\n";

            auto rftVectors = eRft.listOfRftArrays( i );

            for ( const auto& rftVec : rftVectors )
            {
                auto [name, arrType, itemCount] = rftVec;

                std::cout << name << ", " << itemCount << "\n";
            }
        }
    }
}
