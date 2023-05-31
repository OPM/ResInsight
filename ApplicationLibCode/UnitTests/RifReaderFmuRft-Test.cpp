#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifReaderFmuRft.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void verifyData( RifReaderFmuRft& reader )
{
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
TEST( RifReaderFmuRftTest, OldFormatLoadFile )
{
    QString folderName = QString( "%1/RifReaderFmuRft_old_format/" ).arg( TEST_DATA_DIR );

    auto folderNames = RifReaderFmuRft::findSubDirectoriesWithFmuRftData( folderName );
    EXPECT_EQ( 1, folderNames.size() );

    RifReaderFmuRft reader( folderName );
    reader.load();

    verifyData( reader );
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
    reader.load();

    verifyData( reader );
}
