#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifOpmCommonSummary.h"

#include "opm/io/eclipse/EGrid.hpp"
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
TEST( OpmSummaryTests, DISABLED_ReadOpmRadialGrid )
{
    {
        QString filePath = "e:/models/from_equinor_sftp/2021.10.1-Radial-testcase/global-radial/GLOBAL-RADIAL.EGRID";

        Opm::EclIO::EGrid eGrid( filePath.toStdString() );

        auto dims     = eGrid.dimension();
        bool isRadial = eGrid.is_radial();

        size_t cellCount = dims[0] * dims[1] * dims[2];

        std::array<double, 8> X;
        std::array<double, 8> Y;
        std::array<double, 8> Z;

        for ( size_t cidx = 0; cidx < cellCount; cidx++ )
        {
            eGrid.getCellCorners( cidx, X, Y, Z, false );

            for ( size_t i = 0; i < 8; i++ )
            {
                std::cout << X[i] << " " << Y[i] << " " << Z[i] << "\n";
            }
        }
    }
}
