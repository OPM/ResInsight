#include "gtest/gtest.h"

#include "RifRoffFileTools.h"

#include "RiaTestDataDirectory.h"

#include "Reader.hpp"

#include <QDir>
#include <QFile>

#include <fstream>

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsBasic )
{
    // 3 zones spanning 6 K-layers total. ROFF k-order is top→bottom: zone 1 at top,
    // zone 3 at bottom. After the K-flip, reservoir K=0 is the top of the file.
    const std::vector<int> nLayers = { 2, 1, 3 };
    const size_t           nx      = 2;
    const size_t           ny      = 3;
    const size_t           nz      = 6;

    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( nLayers, nx, ny, nz );
    ASSERT_EQ( nx * ny * nz, values.size() );

    // Expected zone per reservoir K (after flipping the ROFF top-down ordering):
    //   ROFF k=0,1 → zone 1 → reservoir K=5,4
    //   ROFF k=2   → zone 2 → reservoir K=3
    //   ROFF k=3,4,5 → zone 3 → reservoir K=2,1,0
    const std::vector<int> expectedZoneByReservoirK = { 3, 3, 3, 2, 1, 1 };

    for ( size_t k = 0; k < nz; ++k )
    {
        for ( size_t j = 0; j < ny; ++j )
        {
            for ( size_t i = 0; i < nx; ++i )
            {
                const size_t idx = i + nx * j + nx * ny * k;
                EXPECT_EQ( static_cast<double>( expectedZoneByReservoirK[k] ), values[idx] );
            }
        }
    }
}

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsRejectsSumMismatch )
{
    // Sum of nLayers (2+1+2 = 5) does not match nz (6) -> empty result.
    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( { 2, 1, 2 }, 1, 1, 6 );
    EXPECT_TRUE( values.empty() );
}

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsRejectsNonPositiveLayer )
{
    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( { 3, 0, 3 }, 1, 1, 6 );
    EXPECT_TRUE( values.empty() );
}

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsRejectsEmptyInput )
{
    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( {}, 1, 1, 0 );
    EXPECT_TRUE( values.empty() );
}

TEST( RifRoffFileTools, ReadSubgridsFromRoffFile )
{
    QDir    baseFolder( TEST_DATA_DIR );
    QString filePath = baseFolder.absoluteFilePath( "RifRoffReader/with_subgrids.roff" );
    ASSERT_TRUE( QFile::exists( filePath ) );

    std::ifstream stream( filePath.toStdString(), std::ios::binary );
    ASSERT_TRUE( stream.good() );

    roff::Reader reader( stream );
    reader.parse();

    ASSERT_EQ( 3u, reader.getArrayLength( "subgrids.nLayers" ) );

    const std::vector<int> nLayers = reader.getIntArray( "subgrids.nLayers" );
    ASSERT_EQ( 3u, nLayers.size() );
    EXPECT_EQ( 2, nLayers[0] );
    EXPECT_EQ( 1, nLayers[1] );
    EXPECT_EQ( 3, nLayers[2] );

    ASSERT_EQ( 3u, reader.getArrayLength( "subgrids.names" ) );
    const std::vector<std::string> names = reader.getStringArray( "subgrids.names" );
    ASSERT_EQ( 3u, names.size() );
    EXPECT_EQ( "Top Zone", names[0] );
    EXPECT_EQ( "Middle Zone", names[1] );
    EXPECT_EQ( "Bottom Zone", names[2] );
}
