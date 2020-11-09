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

#include "gtest/gtest.h"
#include <iostream>

#include "../cafHexGridIntersectionTools.h"

TEST( cafHexIntersectionTools, basic )
{
    std::vector<cvf::Vec3d> triangleVxes = { { 0.0, 0.5, 0.0 },
                                             { 1.0, 0.5, 0.0 },
                                             { 0.0, 1.0, 0.0 },
                                             { 1.0, 0.5, 0.0 },
                                             { 1.0, 0.8, 0.0 },
                                             { 0.0, 1.0, 0.0 } };
    std::vector<cvf::Vec3d> polygon2     = { { 0.5, 0.0, 2002.0 }, { 1.0, 0.0, 2000.0 }, { 0.5, 1.0, 2001.0 } };

    const std::vector<int>  cellFaceForEachTriangleEdge = { 1, 6, 4, 2, 3, 6 };
    const cvf::Vec3d        tp1                         = { 0.5, 0.01, 0.0 };
    const cvf::Vec3d        tp2                         = { 1.0, 0.0, 0.0 };
    const cvf::Vec3d        tp3                         = { 0.5, 1.0, 0.0 };
    std::vector<cvf::Vec3d> clippedTriangleVxes;
    std::vector<int>        cellFaceForEachClippedTriangleEdge;

    caf::HexGridIntersectionTools::clipPlanarTrianglesWithInPlaneTriangle( triangleVxes,
                                                                           cellFaceForEachTriangleEdge,
                                                                           tp1,
                                                                           tp2,
                                                                           tp3,
                                                                           &clippedTriangleVxes,
                                                                           &cellFaceForEachClippedTriangleEdge );

    for ( auto& point : clippedTriangleVxes )
    {
        // std::cout << "   ( " << point[0] << ", " << point[1] << ", " << point[2] << " )" << std::endl;
    }

    for ( auto& face : cellFaceForEachClippedTriangleEdge )
    {
        // std::cout << "   [ " << face  << " ]" << std::endl;
    }
    EXPECT_NEAR( 0.5, clippedTriangleVxes[7][0], 1e-7 );
    EXPECT_NEAR( 0.5, clippedTriangleVxes[7][1], 1e-7 );
    EXPECT_NEAR( 0.75, clippedTriangleVxes[2][0], 1e-7 );
    EXPECT_NEAR( 0.5, clippedTriangleVxes[2][1], 1e-7 );

    EXPECT_NEAR( 0.555555, clippedTriangleVxes[13][0], 1e-5 );
    EXPECT_NEAR( 0.888888, clippedTriangleVxes[13][1], 1e-5 );
    EXPECT_NEAR( 0.5, clippedTriangleVxes[9][0], 1e-7 );
    EXPECT_NEAR( 0.9, clippedTriangleVxes[9][1], 1e-7 );

    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[0] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[1] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[2] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[3] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[4] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[5] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[6] );

    EXPECT_EQ( 1, cellFaceForEachClippedTriangleEdge[7] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[8] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[9] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[10] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[11] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[12] );
    EXPECT_EQ( 3, cellFaceForEachClippedTriangleEdge[13] );
    EXPECT_EQ( 6, cellFaceForEachClippedTriangleEdge[14] );
}