#include "RifSurfio.h"

#include "RiaTestDataDirectory.h"

#include <cmath>
#include <filesystem>
#include <gtest/gtest.h>

TEST( RifSurfioTest, ImportFromIrapFile )
{
    std::filesystem::path testDataDir  = std::filesystem::path( TEST_DATA_DIR ) / "RifSurfaceImporter";
    std::filesystem::path irapFilePath = testDataDir / "volantis.irap";

    auto surfaceData = RifSurfio::importSurfaceData( irapFilePath.string() );
    EXPECT_TRUE( surfaceData.has_value() );

    const auto& [regularSurface, values] = surfaceData.value();
    EXPECT_EQ( regularSurface.nx, 280 );
    EXPECT_EQ( regularSurface.ny, 440 );
    EXPECT_DOUBLE_EQ( regularSurface.incrementX, 25.0 );
    EXPECT_DOUBLE_EQ( regularSurface.incrementY, 25.0 );
    EXPECT_DOUBLE_EQ( regularSurface.rotation, 30.0 );
    EXPECT_EQ( regularSurface.ny * regularSurface.nx, values.size() );
}

TEST( RifSurfioTest, ImportFromGriFile )
{
    std::filesystem::path testDataDir  = std::filesystem::path( TEST_DATA_DIR ) / "RifSurfaceImporter";
    std::filesystem::path irapFilePath = testDataDir / "volantis.gri";

    auto surfaceData = RifSurfio::importSurfaceData( irapFilePath.string() );
    EXPECT_TRUE( surfaceData.has_value() );

    const auto& [regularSurface, values] = surfaceData.value();
    EXPECT_EQ( regularSurface.nx, 280 );
    EXPECT_EQ( regularSurface.ny, 440 );
    EXPECT_DOUBLE_EQ( regularSurface.incrementX, 25.0 );
    EXPECT_DOUBLE_EQ( regularSurface.incrementY, 25.0 );
    EXPECT_DOUBLE_EQ( regularSurface.rotation, 30.0 );
    EXPECT_EQ( regularSurface.ny * regularSurface.nx, values.size() );
}

//--------------------------------------------------------------------------------------------------
/// Export a small synthetic grid to a temporary GRI file, re-import it and verify the round-trip
/// preserves all header parameters and depth values.
//--------------------------------------------------------------------------------------------------
TEST( RifSurfioTest, ExportAndReimportGriRoundTrip )
{
    RigRegularSurfaceData params;
    params.nx         = 4;
    params.ny         = 3;
    params.originX    = 100.0;
    params.originY    = 200.0;
    params.incrementX = 25.0;
    params.incrementY = 25.0;
    params.rotation   = 0.0;

    // Fill with linearly increasing depth values
    std::vector<float> depthValues( params.nx * params.ny );
    for ( int j = 0; j < params.ny; ++j )
        for ( int i = 0; i < params.nx; ++i )
            depthValues[j * params.nx + i] = static_cast<float>( 1000.0 + j * 10.0 + i );

    std::filesystem::path tmpFile = std::filesystem::temp_directory_path() / "resinsight_test_export.gri";
    bool                  ok      = RifSurfio::exportToGri( tmpFile.string(), params, depthValues );
    ASSERT_TRUE( ok );

    auto imported = RifSurfio::importSurfaceData( tmpFile.string() );
    ASSERT_TRUE( imported.has_value() );

    const auto& [surf, vals] = imported.value();
    EXPECT_EQ( surf.nx, params.nx );
    EXPECT_EQ( surf.ny, params.ny );
    EXPECT_DOUBLE_EQ( surf.originX, params.originX );
    EXPECT_DOUBLE_EQ( surf.originY, params.originY );
    EXPECT_DOUBLE_EQ( surf.incrementX, params.incrementX );
    EXPECT_DOUBLE_EQ( surf.incrementY, params.incrementY );
    ASSERT_EQ( vals.size(), depthValues.size() );

    for ( size_t idx = 0; idx < depthValues.size(); ++idx )
    {
        EXPECT_FLOAT_EQ( vals[idx], depthValues[idx] ) << "Mismatch at index " << idx;
    }

    std::filesystem::remove( tmpFile );
}
