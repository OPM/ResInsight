#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifPerforationIntervalReader.h"

#include <cmath> // Needed for HUGE_VAL on Linux
#include <numeric>

#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

static const QString PERFORATION_TEST_DATA_DIRECTORY = QString( "%1/RifPerforationIntervalReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifPerforationIntervalReaderTest, SpacesInWellNameHandledSuccessfully )
{
    std::map<QString, std::vector<RifPerforationInterval>> perforationIntervals = RifPerforationIntervalReader::readPerforationIntervals(
        PERFORATION_TEST_DATA_DIRECTORY + "perforations_with_space_after_well_name.ev" );

    EXPECT_EQ( size_t( 10 ), perforationIntervals["A1_RI_HZX"].size() );
}

//--------------------------------------------------------------------------------------------------
/// Records with too few fields used to read outside the list of parts and crash.
//--------------------------------------------------------------------------------------------------
TEST( RifPerforationIntervalReaderTest, ShortRecordsAreSkipped )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString fileName = tempDir.path() + "/short.ev";
    {
        QFile file( fileName );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        QTextStream out( &file );
        out << "WELLNAME A1\n";
        out << "10 JAN 2025 perforation 1500 2000\n";
        out << "10 JAN 2025 perforation 1500 2000 0.2 0.0\n";
        out << "START perforation 1500 2000\n";
        out << "START perforation 1500 2000 0.2 0.0\n";
    }

    auto intervals = RifPerforationIntervalReader::readPerforationIntervals( fileName );

    ASSERT_EQ( size_t( 2 ), intervals["A1"].size() );
    EXPECT_FALSE( intervals["A1"][0].startOfHistory );
    EXPECT_TRUE( intervals["A1"][1].startOfHistory );
}
