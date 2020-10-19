#include "gtest/gtest.h"

#include "RiaPolyArcLineSampler.h"
#include <iostream>

//--------------------------------------------------------------------------------------------------
TEST( RiaPolyArcLineSampler, Basic )
{
    std::vector<cvf::Vec3d> points{{0, 0, 0}, {0, 0, -10}, {0, 10, -20}, {0, 20, -20}, {0, 30, -30}};

    RiaPolyArcLineSampler sampler( {0, 0, -1}, points );

    auto [sampledPoints, mds] = sampler.sampledPointsAndMDs( 2, true );
#if 1
    for ( size_t pIdx = 0; pIdx < sampledPoints.size(); ++pIdx )
    {
        std::cout << sampledPoints[pIdx].x() << " " << sampledPoints[pIdx].y() << " " << sampledPoints[pIdx].z()
                  << " md: " << mds[pIdx] << std::endl;
    }
#endif
    EXPECT_EQ( 55, (int)sampledPoints.size() );
    EXPECT_NEAR( 51.4159, mds.back(), 1e-4 );

    EXPECT_NEAR( points[2].y(), sampledPoints[27].y(), 2 );
    EXPECT_NEAR( points[2].z(), sampledPoints[27].z(), 2 );

    EXPECT_NEAR( points[4].y(), sampledPoints.back().y(), 2 );
    EXPECT_NEAR( points[4].z(), sampledPoints.back().z(), 2 );
}

//--------------------------------------------------------------------------------------------------
TEST( RiaPolyArcLineSampler, TestInvalidInput )
{
    {
        // Two identical points after each other
        std::vector<cvf::Vec3d> points{{0, 0, -20}, {0, 0, -20}};

        RiaPolyArcLineSampler sampler( {0, 0, -1}, points );

        auto [sampledPoints, mds] = sampler.sampledPointsAndMDs( 2, true );

        EXPECT_EQ( 0, (int)sampledPoints.size() );
        EXPECT_EQ( 0, (int)mds.size() );
    }

    {
        std::vector<cvf::Vec3d> points;

        RiaPolyArcLineSampler sampler( {0, 0, -1}, points );

        auto [sampledPoints, mds] = sampler.sampledPointsAndMDs( 2, true );

        EXPECT_EQ( 0, (int)sampledPoints.size() );
        EXPECT_EQ( 0, (int)mds.size() );
    }

    {
        std::vector<cvf::Vec3d> points{{0, 0, 0}};

        RiaPolyArcLineSampler sampler( {0, 0, -1}, points );

        auto [sampledPoints, mds] = sampler.sampledPointsAndMDs( 2, true );

        EXPECT_EQ( 0, (int)sampledPoints.size() );
        EXPECT_EQ( 0, (int)mds.size() );
    }
}
