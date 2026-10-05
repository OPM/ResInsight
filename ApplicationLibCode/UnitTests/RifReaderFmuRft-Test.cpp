#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifReaderFmuRft.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifReaderFmuRftTest, OldFormatLoadFile )
{
    QString folderName = QString( "%1/RifReaderFmuRft_old_format/" ).arg( TEST_DATA_DIR );

    auto folderNames = RifReaderFmuRft::findSubDirectoriesWithFmuRftData( folderName );
    EXPECT_EQ( 1, folderNames.size() );

    RifReaderFmuRft reader( folderName );
    reader.importData();

    auto wellNames = reader.wellNames();
    EXPECT_EQ( 2u, wellNames.size() );

    QString wellName  = "R_A6";
    auto    timeSteps = reader.availableTimeSteps( wellName );
    EXPECT_EQ( 1u, timeSteps.size() );
    EXPECT_STREQ( timeSteps.begin()->toString( "yyyy-MM-dd" ).toStdString().data(), "2018-11-07" );

    auto addresses = reader.eclipseRftAddresses();
    EXPECT_EQ( 1u, timeSteps.size() );

    for ( const auto& adr : addresses )
    {
        std::vector<double> values;
        reader.values( adr, &values );
        EXPECT_EQ( 2u, values.size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifReaderFmuRftTest, LoadFile )
{
    QString folderName = QString( "%1/RifReaderFmuRft/" ).arg( TEST_DATA_DIR );

    auto folderNames = RifReaderFmuRft::findSubDirectoriesWithFmuRftData( folderName );
    EXPECT_EQ( 1, folderNames.size() );

    RifReaderFmuRft reader( folderName );
    reader.importData();

    QString wellName  = "R_A6";
    auto    timeSteps = reader.availableTimeSteps( wellName );
    EXPECT_EQ( 1u, timeSteps.size() );
    EXPECT_STREQ( timeSteps.begin()->toString( "yyyy-MM-dd" ).toStdString().data(), "2018-11-07" );

    auto addresses = reader.eclipseRftAddresses();
    EXPECT_EQ( 1u, timeSteps.size() );

    for ( const auto& adr : addresses )
    {
        std::vector<double> values;
        reader.values( adr, &values );

        // Two measurements per date
        if ( adr.wellName() == "R_A2" ) EXPECT_EQ( 2u, values.size() );

        // One date with 6 measurements
        if ( adr.wellName() == "R_A6" ) EXPECT_EQ( 6u, values.size() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifReaderFmuRftTest, ConvertMdRangeToTvd )
{
    QString folderName = QString( "%1/RifReaderFmuRft/" ).arg( TEST_DATA_DIR );

    RifReaderFmuRft reader( folderName );
    reader.importData();

    QString wellName  = "R_A6";
    auto    timeSteps = reader.availableTimeSteps( wellName );
    ASSERT_EQ( 1u, timeSteps.size() );
    QDateTime timeStep = *timeSteps.begin();

    // The two observation points for R_A6 are MD 1759.9/TVD 1710.9 (Therys) and MD 1777.4/TVD
    // 1728.4 (Volon). Converting the full MD range should map back exactly onto the TVD range,
    // and the midpoint MD should map to the midpoint TVD (the two points are evenly spaced).
    auto tvdRange = reader.convertMdRangeToTvd( wellName, timeStep, 1759.9, 1777.4 );
    ASSERT_TRUE( tvdRange.has_value() );
    EXPECT_NEAR( 1710.9, tvdRange->first, 1.0e-6 );
    EXPECT_NEAR( 1728.4, tvdRange->second, 1.0e-6 );

    auto tvdMidRange = reader.convertMdRangeToTvd( wellName, timeStep, 1759.9, 1768.65 );
    ASSERT_TRUE( tvdMidRange.has_value() );
    EXPECT_NEAR( 1710.9, tvdMidRange->first, 1.0e-6 );
    EXPECT_NEAR( 1719.65, tvdMidRange->second, 1.0e-6 );

    // Requesting a range outside the observed MD interval should clamp to the nearest endpoint.
    auto tvdClampedRange = reader.convertMdRangeToTvd( wellName, timeStep, 1700.0, 1800.0 );
    ASSERT_TRUE( tvdClampedRange.has_value() );
    EXPECT_NEAR( 1710.9, tvdClampedRange->first, 1.0e-6 );
    EXPECT_NEAR( 1728.4, tvdClampedRange->second, 1.0e-6 );

    // No observations exist for an unknown well, so conversion should fail.
    auto noRange = reader.convertMdRangeToTvd( "UNKNOWN_WELL", timeStep, 0.0, 100.0 );
    EXPECT_FALSE( noRange.has_value() );
}
