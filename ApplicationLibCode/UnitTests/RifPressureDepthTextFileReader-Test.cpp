#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifPressureDepthTextFileReader.h"
#include "RigPressureDepthData.h"

#include <QFile>
#include <QTextStream>

static const QString CASE_REAL_TEST_DATA_DIRECTORY_04 = QString( "%1/RifPressureDepthTextFileReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifPressureDepthTextFileReaderTest, LoadFile )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_04 + "example_file.txt";

    auto [items, errorMessage] = RifPressureDepthTextFileReader::readFile( fileName );

    EXPECT_TRUE( errorMessage.isEmpty() );
    ASSERT_EQ( 3u, items.size() );

    EXPECT_EQ( "G-14", items[0].wellName().toStdString() );
    EXPECT_EQ( 28, items[0].timeStep().date().day() );
    EXPECT_EQ( 12, items[0].timeStep().date().month() );
    EXPECT_EQ( 1995, items[0].timeStep().date().year() );
    std::vector<std::pair<double, double>> values0 = items[0].getPressureDepthValues();
    EXPECT_EQ( 4u, values0.size() );
    double delta = 0.001;
    EXPECT_NEAR( 418.88, values0[0].first, delta );
    EXPECT_NEAR( 2726.91, values0[0].second, delta );

    EXPECT_EQ( "G-14", items[1].wellName().toStdString() );
    EXPECT_EQ( 28, items[1].timeStep().date().day() );
    EXPECT_EQ( 12, items[1].timeStep().date().month() );
    EXPECT_EQ( 1996, items[1].timeStep().date().year() );
    std::vector<std::pair<double, double>> values1 = items[1].getPressureDepthValues();
    EXPECT_NEAR( 418.88, values1[0].first, delta );
    EXPECT_NEAR( 2726.91, values1[0].second, delta );

    EXPECT_EQ( "F-56", items[2].wellName().toStdString() );
    EXPECT_EQ( 15, items[2].timeStep().date().day() );
    EXPECT_EQ( 1, items[2].timeStep().date().month() );
    EXPECT_EQ( 2012, items[2].timeStep().date().year() );
    std::vector<std::pair<double, double>> values2 = items[2].getPressureDepthValues();
    EXPECT_EQ( 7u, values2.size() );
    EXPECT_NEAR( 413.32, values2[6].first, delta );
    EXPECT_NEAR( 2896.555, values2[6].second, delta );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifPressureDepthTextFileReaderTest, LoadFileNonExistingFiles )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_04 + "this_file_does_not_exist.csv";

    auto [items, errorMessage] = RifPressureDepthTextFileReader::readFile( fileName );

    EXPECT_FALSE( errorMessage.isEmpty() );
    EXPECT_EQ( 0u, items.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifPressureDepthTextFileReaderTest, FieldUnitData )
{
    auto content = R"(
--TVDMSL
RFT  
--
WELLNAME 'A1'
DATE 18-NOV-2018
PRESSURE DEPTH
PSIA FEET
12008.00  22640.66
12020.40  22674.44
\n)";

    auto [items, errorMessage] = RifPressureDepthTextFileReader::parse( content );

    EXPECT_TRUE( errorMessage.isEmpty() );
    ASSERT_EQ( 1u, items.size() );

    EXPECT_EQ( "A1", items[0].wellName().toStdString() );
    std::vector<std::pair<double, double>> values0 = items[0].getPressureDepthValues();
    EXPECT_EQ( 2u, values0.size() );
    double delta = 0.001;
    EXPECT_NEAR( 12008.0, values0[0].first, delta );
    EXPECT_NEAR( 22640.66, values0[0].second, delta );
}
