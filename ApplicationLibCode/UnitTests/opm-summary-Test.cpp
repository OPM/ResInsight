#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifOpmCommonSummary.h"

#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

static const QString H5_TEST_DATA_DIRECTORY = QString( "%1/h5-file/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_OpmSummaryTests, ReadOpmSummaryDataListContent )
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

        RifEclipseSummaryAddress eclAdr = RifEclipseSummaryAddress::fromEclipseTextAddress( s1 );
        EXPECT_TRUE( eclAdr.isValid() );
    }

    /*
        auto keywords = eSmry.keywordList();
        for ( auto k : keywords )
        {
            std::cout << k << "\n";
        }
    */
}
