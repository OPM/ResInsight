//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include <iostream>
#include "gtest/gtest.h"

#include "../cafHexInterpolator.h"
#include "cvfMatrix4.h"

namespace caf {
class HexInterpolatorTester
{
    
};
}

void testHex(const std::array<cvf::Vec3d, 8>& hexCorners, double tolerance = 1e-4)
{
    double result = 0.0;
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[0]);
    EXPECT_NEAR(result, 0.0, tolerance);
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[1]);
    EXPECT_NEAR(result, 1.0, tolerance);
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[2]);
    EXPECT_NEAR(result, 2.0, tolerance);
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[3]);
    EXPECT_NEAR(result, 3.0, tolerance);
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[4]);
    EXPECT_NEAR(result, 4.0, tolerance);
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[5]);
    EXPECT_NEAR(result, 5.0, tolerance);
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[6]);
    EXPECT_NEAR(result, 6.0, tolerance);
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, hexCorners[7]);
    EXPECT_NEAR(result, 7.0, tolerance);

    cvf::Vec3d avg = cvf::Vec3d::ZERO;
    for ( const auto & v : hexCorners) avg += v;
    avg *= 0.125;
    result = caf::HexInterpolator::interpolateHex(hexCorners, {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0 }, avg);
    EXPECT_NEAR(result, 3.5, tolerance);

}

TEST(InterpolationTest, UnitElement)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
    cvf::Vec3d(-1.0,-1.0,-1.0),
    cvf::Vec3d( 1.0,-1.0,-1.0),
    cvf::Vec3d( 1.0, 1.0,-1.0),
    cvf::Vec3d(-1.0, 1.0,-1.0),
    cvf::Vec3d(-1.0,-1.0, 1.0),
    cvf::Vec3d( 1.0,-1.0, 1.0),
    cvf::Vec3d( 1.0, 1.0, 1.0),
    cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    testHex(hexCorners);
}

TEST(InterpolationTest, ScaledElement)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    for (  auto & v : hexCorners) v *= 2;

    testHex(hexCorners);
}

TEST(InterpolationTest, TranslatedElement)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    for (  auto & v : hexCorners) v += {2.2, 4.4, -7.6};

    testHex(hexCorners);
}

TEST(InterpolationTest, RotatedElement)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    cvf::Mat4d rot = cvf::Mat4d::fromRotation({1,1, 0}, 3.14159);

    for (  auto & v : hexCorners) v.transformPoint(rot);

    testHex(hexCorners);
}

TEST(InterpolationTest, CombinedUndeformed)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    for (  auto & v : hexCorners) v *= 2;
    
    for (  auto & v : hexCorners) v += {2.2, 4.4, -7.6};

    cvf::Mat4d rot = cvf::Mat4d::fromRotation({1,1, 0}, 3.14159);

    for (  auto & v : hexCorners) v.transformPoint(rot);

    testHex(hexCorners);
}

TEST(InterpolationTest, Compressed)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    for (  auto & v : hexCorners) v[0] *= 0.1;

    testHex(hexCorners);

    for (  auto & v : hexCorners) v[0] *= 10.0;
    for (  auto & v : hexCorners) v[1] *= 0.1;

    testHex(hexCorners);

    for (  auto & v : hexCorners) v[1] *= 10.0;
    for (  auto & v : hexCorners) v[2] *= 0.1;

    testHex(hexCorners);
}

TEST(InterpolationTest, Scewed)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    for ( int i = 4 ; i< 8 ; ++i) hexCorners[i] += {1.0, 0.0, 0.0};

    testHex(hexCorners);
}

TEST(InterpolationTest, Screwed)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    cvf::Mat4d rot = cvf::Mat4d::fromRotation({0, 0, 1}, 1.0*0.25*3.14159);
    
    for ( int i = 4 ; i< 8 ; ++i) hexCorners[i].transformPoint(rot);

    testHex(hexCorners);
}

TEST(InterpolationTest, Warp)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    {
        std::array<cvf::Vec3d, 8> cornerCopy = hexCorners;
        // Compress x
        for ( auto & v : cornerCopy ) v[0] *= 0.1;

        cornerCopy[0][0] += 0.25;
        cornerCopy[1][0] += 0.20;
        cornerCopy[7][0] += 0.25;
        cornerCopy[6][0] += 0.4;

        testHex(cornerCopy);
    }

    {
        std::array<cvf::Vec3d, 8> cornerCopy = hexCorners;
        // Compress z
        for ( auto & v : cornerCopy ) v[2] *= 0.1;

        cornerCopy[0][2] += 0.25;
        cornerCopy[2][2] += 0.25;
        cornerCopy[4][2] += 0.2;
        cornerCopy[6][2] += 0.4;

        testHex(cornerCopy);
    }
}

TEST(InterpolationTest, TotalCombined)
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { 
        cvf::Vec3d(-1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0,-1.0,-1.0),
        cvf::Vec3d( 1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0, 1.0,-1.0),
        cvf::Vec3d(-1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0,-1.0, 1.0),
        cvf::Vec3d( 1.0, 1.0, 1.0),
        cvf::Vec3d(-1.0, 1.0, 1.0)
    };

    cvf::Mat4d rot = cvf::Mat4d::fromRotation({0, 0, 1}, 1.0*0.25*3.14159);

    for ( int i = 4 ; i< 8 ; ++i) hexCorners[i].transformPoint(rot);

    for ( int i = 4 ; i< 8 ; ++i) hexCorners[i] += {0.2, 0.0, 0.0};


    {
        std::array<cvf::Vec3d, 8> cornerCopy = hexCorners;
        // Compress z
        for ( auto & v : hexCorners ) v[2] *= 0.3;

        hexCorners[0][2] += 0.25;
        hexCorners[2][2] += 0.25;
        hexCorners[4][2] += 0.2;
        hexCorners[6][2] += 0.4;
    }

    for (  auto & v : hexCorners) v *= 200;

    for (  auto & v : hexCorners) v += {2.3e5, 4.7e6, -2000};

    cvf::Mat4d rot2 = cvf::Mat4d::fromRotation({1,1, 0}, 3.14159);

    for (  auto & v : hexCorners) v.transformPoint(rot2);


    testHex(hexCorners);
}