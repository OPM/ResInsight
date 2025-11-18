#include "gtest/gtest.h"

#include "RiaLogging.h"
#include "RiaRftDefines.h"
#include "RiaTestDataDirectory.h"

#include "RifEclipseSummaryTools.h"
#include "RifOpmCommonSummary.h"
#include "RifReaderOpmRft.h"

#ifdef _MSC_VER
// Disable warning from external library to make sure treat warnings as error works
#pragma warning( disable : 4267 )
#endif
#include "opm/io/eclipse/EGrid.hpp"
#include "opm/io/eclipse/ERft.hpp"
#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

#include <QDebug>
#include <QFile>

static const QString H5_TEST_DATA_DIRECTORY = QString( "%1/h5-file/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( OpmSummaryTests, DISABLED_PerformanceReadOfRestartFilename )
{
    // 2024-11-14
    // This test is intended to measure the performance of reading restart filenames from a summary file
    // Both resdata and opm-common are used, and for some cases resdata is much slower than opm-common
    //
    // Performance comparison
    // BRENT-PRED_IAM_NSA_F - 100 iterations
    // opm-common:   10 ms
    // resdata:   10000 ms

    QString filePath = H5_TEST_DATA_DIRECTORY + "NORNE_ATW2013_RFTPLT_V2.SMSPEC";
    filePath         = "C:/gitroot/ResInsight-regression-test/ModelData/ensemble_reek_with_params/realization-7/iter-0/eclipse/model/"
                       "3_R001_REEK-7.SMSPEC";

    filePath = "c:/gitroot/ResInsight-regression-test/ModelData/TestCase_MultiCaseStatistics/SIMPLE_R1.SMSPEC";
    filePath = "f:/Models/equinor_azure/Sum_File/BRENT-PRED_IAM_NSA_F.SMSPEC";

    const int N = 100;

    auto startTime = RiaLogging::currentTime();
    for ( int i = 0; i < N; i++ )
    {
        std::vector<QString> warnings;
        auto                 restartFileInfos = RifEclipseSummaryTools::getRestartFileNames( filePath, warnings );
    }
    RiaLogging::logElapsedTime( "Completed opm-common", startTime );

    auto startTime2 = RiaLogging::currentTime();
    for ( int i = 0; i < N; i++ )
    {
        std::vector<QString> warnings;
        auto                 restartFileInfos = RifEclipseSummaryTools::getRestartFileNames( filePath, warnings );
    }

    RiaLogging::logElapsedTime( "Completed resdata", startTime2 );
}

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
TEST( OpmSummaryTests, DISABLED_OpmImportRftData )
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

            auto results = eRft.listOfRftArrays( i );
            for ( const auto& [name, arrayType, size] : results )
            {
                std::cout << name << ", " << size << "\n";
            }
        }
    }
}

// std::vector<int> getValues(Opm::EclIO::ERft& fileReader, std::)

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( OpmSummaryTests, DISABLED_OpmComputeSegmentTopology )
{
    std::vector<std::string> esmryKeywords;
    {
        // QString filePath = "e:/gitroot/opm-tests/model1/opm-simulation-reference/flow/MSW_MODEL_1.RFT";
        // QString filePath = "e:/gitroot/opm-tests/norne/ECL.2014.2/NORNE_ATW2013.RFT";
        // QString filePath = "d:/Models/Statoil/MSW-RFTfile/NORNE_ATW2013_RFTPLT_MSW.RFT";
        //		QString filePath = "e:/models/from_equinor_sftp/MSW-RFTfile/NORNE_ATW2013_RFTPLT_MSW.RFT";
        QString filePath = "e:/models/from_equinor_sftp/MSWRFT_toCS/MSWLOOPED_UN_RFT.RFT";

        Opm::EclIO::ERft eRft( filePath.toStdString() );

        auto wells = eRft.listOfWells();
        auto dates = eRft.listOfdates();

        class RftSegmentData
        {
        public:
            RftSegmentData( int segnxt, int brno, int brnst, int brnen, int segNo )
                : m_segNext( segnxt )
                , m_segbrno( brno )
                , m_brnst( brnst )
                , m_brnen( brnen )
                , m_segmentNo( segNo )
            {
            }

            int segNext() const { return m_segNext; }
            int segBrno() const { return m_segbrno; }
            int segBrnst() const { return m_brnst; }
            int segBrnen() const { return m_brnen; }
            int segNo() const { return m_segmentNo; }

        private:
            int m_segNext;
            int m_segbrno;
            int m_brnst;
            int m_brnen;
            int m_segmentNo;
        };

        using RftSegmentKey = std::pair<std::string, Opm::EclIO::ERft::RftDate>;
        std::map<RftSegmentKey, std::vector<RftSegmentData>> rftWellDateSegments;

        for ( const auto& well : wells )
        {
            for ( const auto& date : dates )
            {
                std::vector<RftSegmentData> segmentsForWellDate;

                std::vector<int> segnxt;
                std::vector<int> segbrno;
                std::vector<int> brnstValues;
                std::vector<int> brnenValues;
                std::vector<int> segNo;

                {
                    std::string resultName = "SEGNXT";
                    if ( eRft.hasArray( resultName, well, date ) )
                    {
                        segnxt = eRft.getRft<int>( resultName, well, date );
                    }
                }
                {
                    std::string resultName = "SEGBRNO";
                    if ( eRft.hasArray( resultName, well, date ) )
                    {
                        segbrno = eRft.getRft<int>( resultName, well, date );
                    }
                }
                {
                    std::string resultName = "BRNST";
                    if ( eRft.hasArray( resultName, well, date ) )
                    {
                        brnstValues = eRft.getRft<int>( resultName, well, date );
                    }
                }
                {
                    std::string resultName = "BRNEN";
                    if ( eRft.hasArray( resultName, well, date ) )
                    {
                        brnenValues = eRft.getRft<int>( resultName, well, date );
                    }
                }

                if ( segnxt.empty() ) continue;
                if ( segnxt.size() != segbrno.size() ) continue;
                if ( brnenValues.empty() || brnstValues.empty() ) continue;

                for ( size_t i = 0; i < segnxt.size(); i++ )
                {
                    int branchIndex     = segbrno[i] - 1;
                    int nextBranchIndex = -1;
                    if ( i + 1 < segbrno.size() ) nextBranchIndex = segbrno[i + 1] - 1;

                    bool isLastSegmentOnBranch = branchIndex != nextBranchIndex;

                    int brnst = brnstValues[branchIndex];
                    int brnen = brnenValues[branchIndex];

                    int segmentId = -1;
                    if ( !isLastSegmentOnBranch )
                    {
                        if ( i + 1 < segnxt.size() ) segmentId = segnxt[i + 1];
                    }
                    else
                    {
                        segmentId = brnen;
                    }

                    segNo.push_back( segmentId );

                    segmentsForWellDate.emplace_back( RftSegmentData( segnxt[i], segbrno[i], brnst, brnen, segmentId ) );
                }

                auto wellDateKey = std::make_pair( well, date );

                rftWellDateSegments[wellDateKey] = segmentsForWellDate;
            }
        }

        for ( const auto& a : rftWellDateSegments )
        {
            auto [wellName, date] = a.first;
            auto segmentData      = a.second;

            std::cout << "\nWell: " << wellName << "Date : " << std::get<0>( date ) << " " << std::get<1>( date ) << " "
                      << std::get<2>( date ) << " \n";

            for ( const auto& r : segmentData )
            {
                std::cout << "SEGNXT  " << std::setw( 2 ) << r.segNext() << ", ";
                std::cout << "SEGBRNO " << std::setw( 2 ) << r.segBrno() << ", ";
                std::cout << "BNRST   " << std::setw( 2 ) << r.segBrnst() << ", ";
                std::cout << "BRNEN   " << std::setw( 2 ) << r.segBrnen() << ", ";
                std::cout << "SEGNO   " << std::setw( 2 ) << r.segNo() << "\n";
            }
        }
    }
}

TEST( OpmSummaryTests, OpmComputeSegmentTopology )
{
    std::vector<std::string> esmryKeywords;
    {
        // QString filePath = "e:/gitroot/opm-tests/model1/opm-simulation-reference/flow/MSW_MODEL_1.RFT";
        // QString filePath = "e:/gitroot/opm-tests/norne/ECL.2014.2/NORNE_ATW2013.RFT";
        // QString filePath = "d:/Models/Statoil/MSW-RFTfile/NORNE_ATW2013_RFTPLT_MSW.RFT";
        //		QString filePath = "e:/models/from_equinor_sftp/MSW-RFTfile/NORNE_ATW2013_RFTPLT_MSW.RFT";
        QString filePath = "e:/models/from_equinor_sftp/MSWRFT_toCS/MSWLOOPED_UN_RFT.RFT";

        RifReaderOpmRft reader( filePath );

        auto addresses = reader.eclipseRftAddresses();

        std::vector<float> segStartDepth;
        std::vector<float> segEndDepth;
        std::vector<float> segNumber;

        std::set<RifEclipseRftAddress> segmentAdresses;
        for ( const auto& adr : addresses )
        {
            if ( adr.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES )
            {
                segmentAdresses.insert( adr );
            }
        }

        for ( const auto& adr : segmentAdresses )
        {
            qDebug() << adr.timeStep().toString( "YYYY MM dd" ) << " " << adr.segmentResultName();

            if ( adr.segmentResultName() == RiaDefines::segmentNumberResultName() )
            {
                std::vector<double> values;
                reader.values( adr, &values );
            }
        }
    }
}

TEST( OpmSummaryTests, OpenEmptySummaryFile )
{
    QString SUMMARY_TEST_DATA_DIRECTORY = QString( "%1/SummaryData/empty-file/" ).arg( TEST_DATA_DIR );
    QString rootPath                    = SUMMARY_TEST_DATA_DIRECTORY + "BLASTO_PRED-19";
    QString smspecFilePath              = rootPath + ".SMSPEC";

    Opm::EclIO::ESmry eSmry( smspecFilePath.toStdString() );

    // Test to verify that is is possible to read an empty summary file
    // eSmry.make_esmry_file() will fail if the summary file is empty

    EXPECT_TRUE( eSmry.numberOfTimeSteps() == 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( OpmSummaryTests, DISABLED_ReadOpmRadialGrid )
{
    {
        QString filePath = "e:/models/from_equinor_sftp/2021.10.1-Radial-testcase/global-radial/GLOBAL-RADIAL.EGRID";

        Opm::EclIO::EGrid eGrid( filePath.toStdString() );

        auto dims = eGrid.dimension();

        size_t cellCount = dims[0] * dims[1] * dims[2];

        std::array<double, 8> X{};
        std::array<double, 8> Y{};
        std::array<double, 8> Z{};

        for ( size_t cidx = 0; cidx < cellCount; cidx++ )
        {
            eGrid.getCellCorners( cidx, X, Y, Z );

            for ( size_t i = 0; i < 8; i++ )
            {
                std::cout << X[i] << " " << Y[i] << " " << Z[i] << "\n";
            }
        }
    }
}

TEST( OpmSummaryHeaderSearch, NoEsmryRestart )
{
    QString SUMMARY_TEST_DATA_DIRECTORY = QString( "%1/summary-header-search/no-esmry-restart/" ).arg( TEST_DATA_DIR );
    QString filename                    = SUMMARY_TEST_DATA_DIRECTORY + "/realization-18/pred_ref/eclipse/model/DROGON-18.ESMRY";

    const bool           useOpmReader = true;
    std::vector<QString> warnings;
    auto                 restartFilenames = RifEclipseSummaryTools::getRestartFileNames( filename, useOpmReader, warnings );

    EXPECT_TRUE( restartFilenames.size() == 1 );
    auto restartFileName = restartFilenames[0];

    // search for text iter-3
    EXPECT_TRUE( restartFileName.contains( "iter-3" ) );

    // check that the file does not exist
    QFile file( restartFileName );
    EXPECT_FALSE( file.exists() );
}

TEST( OpmSummaryHeaderSearch, OnlyEsmryFiles )
{
    QString SUMMARY_TEST_DATA_DIRECTORY = QString( "%1/summary-header-search/only-esmry/" ).arg( TEST_DATA_DIR );
    QString filename                    = SUMMARY_TEST_DATA_DIRECTORY + "/realization-18/pred_ref/eclipse/model/DROGON-18.ESMRY";

    const bool           useOpmReader = true;
    std::vector<QString> warnings;
    auto                 restartFilenames = RifEclipseSummaryTools::getRestartFileNames( filename, useOpmReader, warnings );

    EXPECT_TRUE( restartFilenames.size() == 1 );
    auto restartFileName = restartFilenames[0];

    // search for text iter-3
    EXPECT_TRUE( restartFileName.contains( "iter-3" ) );

    // check that the file exists
    QFile file( restartFileName );
    EXPECT_TRUE( file.exists() );
}

TEST( OpmSummaryHeaderSearch, OnlySmspecFiles )
{
    QString SUMMARY_TEST_DATA_DIRECTORY = QString( "%1/summary-header-search/only-smspec/" ).arg( TEST_DATA_DIR );
    QString filename                    = SUMMARY_TEST_DATA_DIRECTORY + "/realization-18/pred_ref/eclipse/model/DROGON-18.SMSPEC";

    const bool           useOpmReader = true;
    std::vector<QString> warnings;
    auto                 restartFilenames = RifEclipseSummaryTools::getRestartFileNames( filename, useOpmReader, warnings );

    EXPECT_TRUE( restartFilenames.size() == 1 );
    auto restartFileName = restartFilenames[0];

    // search for text iter-3
    EXPECT_TRUE( restartFileName.contains( "iter-3" ) );

    // check that the file exists
    QFile file( restartFileName );
    EXPECT_TRUE( file.exists() );
}
