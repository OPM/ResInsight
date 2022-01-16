#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifOpmCommonSummary.h"

#include "RifReaderOpmRft.h"
#include "opm/io/eclipse/ERft.hpp"
#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

#include "RiaRftDefines.h"
#include <QDebug>

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

            auto rftVectors = eRft.listOfRftArrays( i );

            for ( const auto& rftVec : rftVectors )
            {
                auto [name, arrType, itemCount] = rftVec;

                std::cout << name << ", " << itemCount << "\n";
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

        auto adresses = reader.eclipseRftAddresses();

        std::vector<float> segStartDepth;
        std::vector<float> segEndDepth;
        std::vector<float> segNumber;

        std::set<RifEclipseRftAddress> segmentAdresses;
        for ( const auto& adr : adresses )
        {
            if ( adr.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES )
            {
                segmentAdresses.insert( adr );
            }
        }

        for ( const auto& adr : segmentAdresses )
        {
            qDebug() << adr.timeStep().toString( "YYYY MM dd" ) << " " << adr.resultName();

            if ( adr.resultName() == RiaDefines::segmentNumberResultName() )
            {
                std::vector<double> values;
                reader.values( adr, &values );
            }
        }
    }
}
