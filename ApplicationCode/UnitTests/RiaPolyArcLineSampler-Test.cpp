#include "gtest/gtest.h"

#include "RiaPolyArcLineSampler.h"
#include <iostream>

//--------------------------------------------------------------------------------------------------
TEST(RiaPolyArcLineSampler, Basic)
{
    std::vector<cvf::Vec3d> points {  {0,0,0}, {0,0,-10}, {0,10,-20},{0,20,-20}, {0,30, -30} }; 

    RiaPolyArcLineSampler sampler({0,0,-1}, points);

    std::vector<cvf::Vec3d> sampledPoints; 
    std::vector<double> mds;
    
    sampler.sampledPointsAndMDs(2, true, &sampledPoints, &mds);
    #if 0
    for (size_t pIdx = 0; pIdx < sampledPoints.size(); ++pIdx)
    {
        std::cout <<  sampledPoints[pIdx].x() << " " 
                  <<  sampledPoints[pIdx].y() << " " 
                  <<  sampledPoints[pIdx].z() << " md: " << mds[pIdx] << std::endl;
    } 
    #endif
    EXPECT_EQ(27, (int)sampledPoints.size());
    EXPECT_NEAR(51.4159,  mds.back(), 1e-4);

    EXPECT_NEAR(points[2].y(),  sampledPoints[12].y(), 2);
    EXPECT_NEAR(points[2].z(),  sampledPoints[12].z(), 2);

    EXPECT_NEAR(points[4].y(),  sampledPoints[25].y(), 2);
    EXPECT_NEAR(points[4].z(),  sampledPoints[25].z(), 2);
}
