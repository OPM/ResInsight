#include "gtest/gtest.h"

#include "RiaCameraTools.h"

namespace
{
void expectOrthonormal( const RiaCameraTools::Orientation& orientation )
{
    EXPECT_NEAR( 1.0, orientation.direction.length(), 1.0e-12 );
    EXPECT_NEAR( 1.0, orientation.up.length(), 1.0e-12 );
    EXPECT_NEAR( 0.0, orientation.direction * orientation.up, 1.0e-12 );
}

void expectVecNear( const cvf::Vec3d& expected, const cvf::Vec3d& actual )
{
    EXPECT_NEAR( expected.x(), actual.x(), 1.0e-12 );
    EXPECT_NEAR( expected.y(), actual.y(), 1.0e-12 );
    EXPECT_NEAR( expected.z(), actual.z(), 1.0e-12 );
}
} // namespace

TEST( RiaCameraToolsTest, AzimuthAndTiltAxisAlignedViews )
{
    // Looking straight down with north up on the screen
    auto top = RiaCameraTools::fromAzimuthAndTilt( 0.0, 0.0 );
    expectVecNear( cvf::Vec3d( 0, 0, -1 ), top.direction );
    expectVecNear( cvf::Vec3d( 0, 1, 0 ), top.up );

    // Horizontal views along the compass directions
    auto north = RiaCameraTools::fromAzimuthAndTilt( 0.0, 90.0 );
    expectVecNear( cvf::Vec3d( 0, 1, 0 ), north.direction );
    expectVecNear( cvf::Vec3d( 0, 0, 1 ), north.up );

    auto east = RiaCameraTools::fromAzimuthAndTilt( 90.0, 90.0 );
    expectVecNear( cvf::Vec3d( 1, 0, 0 ), east.direction );

    auto south = RiaCameraTools::fromAzimuthAndTilt( 180.0, 90.0 );
    expectVecNear( cvf::Vec3d( 0, -1, 0 ), south.direction );
}

TEST( RiaCameraToolsTest, AzimuthAndTiltIsOrthonormal )
{
    for ( double azimuth : { -45.0, 0.0, 37.0, 200.0, 359.0, 720.0 } )
    {
        for ( double tilt : { 0.0, 1.0, 60.0, 90.0, 135.0, 180.0 } )
        {
            expectOrthonormal( RiaCameraTools::fromAzimuthAndTilt( azimuth, tilt ) );
        }
    }
}

TEST( RiaCameraToolsTest, ObliqueViewLooksDownwardsAlongAzimuth )
{
    auto orientation = RiaCameraTools::fromAzimuthAndTilt( 200.0, 60.0 );

    EXPECT_LT( orientation.direction.z(), 0.0 );
    EXPECT_GT( orientation.up.z(), 0.0 );

    // The horizontal part of the viewing direction points towards south-southwest
    EXPECT_LT( orientation.direction.x(), 0.0 );
    EXPECT_LT( orientation.direction.y(), 0.0 );
}

TEST( RiaCameraToolsTest, ViewDirectionsMatchGuiCommands )
{
    auto fromNorth = RiaCameraTools::fromViewDirection( RiaDefines::ViewDirection::NORTH );
    expectVecNear( cvf::Vec3d( 0, -1, 0 ), fromNorth.direction );
    expectVecNear( cvf::Vec3d( 0, 0, 1 ), fromNorth.up );

    auto top = RiaCameraTools::fromViewDirection( RiaDefines::ViewDirection::TOP );
    expectVecNear( cvf::Vec3d( 0, 0, -1 ), top.direction );
    expectVecNear( cvf::Vec3d( 0, 1, 0 ), top.up );

    for ( auto direction : { RiaDefines::ViewDirection::NORTH,
                             RiaDefines::ViewDirection::SOUTH,
                             RiaDefines::ViewDirection::EAST,
                             RiaDefines::ViewDirection::WEST,
                             RiaDefines::ViewDirection::TOP,
                             RiaDefines::ViewDirection::BOTTOM } )
    {
        expectOrthonormal( RiaCameraTools::fromViewDirection( direction ) );
    }
}
