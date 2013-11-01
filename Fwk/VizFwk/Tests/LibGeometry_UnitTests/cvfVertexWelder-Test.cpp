//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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


#include "cvfBase.h"
#include "cvfVertexWelder.h"

#include <cmath>

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexWelderTest, Basic)
{
    VertexWelder w;
    w.initialize(0.01, 0.1, 100);

    {
        bool wasWelded = false;
        cvf::uint idx = w.weldVertex(Vec3f(0, 0, 0), &wasWelded);
        EXPECT_FALSE(wasWelded);
        EXPECT_EQ(0, idx);
    }

    {
        bool wasWelded = false;
        cvf::uint idx = w.weldVertex(Vec3f(0, 0, 0), &wasWelded);
        EXPECT_TRUE(wasWelded);
        EXPECT_EQ(0, idx);
    }


    {
        bool wasWelded = false;
        cvf::uint idx = w.weldVertex(Vec3f(1, 0, 0), &wasWelded);
        EXPECT_FALSE(wasWelded);
        EXPECT_EQ(1, idx);
    }

    {
        bool wasWelded = false;
        cvf::uint idx = w.weldVertex(Vec3f(0, 2, 0), &wasWelded);
        EXPECT_FALSE(wasWelded);
        EXPECT_EQ(2, idx);
    }

    {
        bool wasWelded = false;
        cvf::uint idx = w.weldVertex(Vec3f(0, 0, 3), &wasWelded);
        EXPECT_FALSE(wasWelded);
        EXPECT_EQ(3, idx);
    }


    {
        bool wasWelded = false;
        cvf::uint idx = w.weldVertex(Vec3f(0, 2.001f, 0), &wasWelded);
        EXPECT_TRUE(wasWelded);
        EXPECT_EQ(2, idx);
    }

    {
        bool wasWelded = false;
        cvf::uint idx = w.weldVertex(Vec3f(0, 2.02f, 0), &wasWelded);
        EXPECT_FALSE(wasWelded);
        EXPECT_EQ(4, idx);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexWelderTest, WeldBunchOfPointsAndCheckDistance)
{

    const int numSrcPoints = 100;
    Vec3fArray weldedVerts;
    weldedVerts.reserve(numSrcPoints);

    const double weldDist = 0.05;
    const double cellSize = 10*weldDist;
    VertexWelder w;
    w.initialize(weldDist, cellSize, 100);

    int i;
    for (i = 0; i < numSrcPoints; i++)
    {
        Vec3f v((float)i/numSrcPoints, (float)sin(i/10.0), (float)cos(i/20.0));

        bool wasWelded = false;
        w.weldVertex(v, &wasWelded);
        if (!wasWelded)
        {
            weldedVerts.add(v);
        }
    }


    int numPoints = static_cast<int>(weldedVerts.size());

    for (i = 0; i < numPoints; i++)
    {
        Vec3f v0 = weldedVerts[i];

        int j;
        for (j = 0; j < numPoints; j++)
        {
            if (j != i)
            {
                double dist = v0.pointDistance(weldedVerts[j]);
                EXPECT_LT(weldDist, dist);
            }
        }
    }
}


