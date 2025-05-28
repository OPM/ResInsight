#include "RifSurfio.h"

#include "RiaTestDataDirectory.h"

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
