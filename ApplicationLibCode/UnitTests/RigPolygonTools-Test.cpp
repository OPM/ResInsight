/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RigPolygonTools.h"
#include "gtest/gtest.h"

// Test for erode function
TEST( RigPolygonToolsTest, ErodeTest )
{
    // Arrange
    RigPolygonTools::IntegerImage image      = { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };
    int                           kernelSize = 1;
    RigPolygonTools::IntegerImage expected   = { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };

    // Act
    RigPolygonTools::IntegerImage result = RigPolygonTools::erode( image, kernelSize );

    // Assert
    EXPECT_EQ( result, expected );
}

// Test for erode function with invalid input
TEST( RigPolygonToolsTest, ErodeInvalidInputTest )
{
    // Arrange
    RigPolygonTools::IntegerImage emptyImage         = {};
    int                           negativeKernelSize = -1;

    // Act & Assert
    EXPECT_EQ( emptyImage, RigPolygonTools::erode( emptyImage, 1 ) );
    EXPECT_EQ( emptyImage, RigPolygonTools::erode( { { 1, 1 }, { 1 } }, 1 ) ); // Inconsistent row sizes
    EXPECT_EQ( emptyImage, RigPolygonTools::erode( { { 1, 1 }, { 1, 1 } }, negativeKernelSize ) );
}

// Test for dilate function
TEST( RigPolygonToolsTest, DilateTest )
{
    // Arrange
    RigPolygonTools::IntegerImage image = {
        { 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0 },
        { 0, 0, 1, 0, 0 },
        { 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0 },
    };
    int kernelSize = 2;

    RigPolygonTools::IntegerImage expected = {
        { 0, 0, 0, 0, 0 },
        { 0, 1, 1, 1, 0 },
        { 0, 1, 1, 1, 0 },
        { 0, 1, 1, 1, 0 },
        { 0, 0, 0, 0, 0 },
    };

    // Act
    RigPolygonTools::IntegerImage result = RigPolygonTools::dilate( image, kernelSize );

    // Assert
    EXPECT_EQ( result, expected );
}

// Test for dilate function with invalid input
TEST( RigPolygonToolsTest, DilateInvalidInputTest )
{
    // Arrange
    RigPolygonTools::IntegerImage emptyImage         = {};
    int                           negativeKernelSize = -1;

    // Act & Assert
    EXPECT_EQ( emptyImage, RigPolygonTools::dilate( emptyImage, 1 ) );
    EXPECT_EQ( emptyImage, RigPolygonTools::dilate( { { 1, 1 }, { 1 } }, 1 ) ); // Inconsistent row sizes
    EXPECT_EQ( emptyImage, RigPolygonTools::dilate( { { 1, 1 }, { 1, 1 } }, negativeKernelSize ) );
}

// Test for fillInterior function
TEST( RigPolygonToolsTest, FillInteriorTest )
{
    // Arrange
    RigPolygonTools::IntegerImage image    = { { 1, 1, 1 }, { 1, 0, 1 }, { 1, 1, 1 } };
    RigPolygonTools::IntegerImage expected = { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };

    // Act
    RigPolygonTools::IntegerImage result = RigPolygonTools::fillInterior( image );

    // Assert
    EXPECT_EQ( result, expected );
}

// Test for fillInterior function with invalid input
TEST( RigPolygonToolsTest, FillInteriorInvalidInputTest )
{
    // Arrange
    RigPolygonTools::IntegerImage emptyImage = {};

    // Act & Assert
    EXPECT_EQ( emptyImage, RigPolygonTools::fillInterior( emptyImage ) );
    EXPECT_EQ( emptyImage, RigPolygonTools::fillInterior( { { 1, 1 }, { 1 } } ) ); // Inconsistent row sizes
}

// Test for boundary function
TEST( RigPolygonToolsTest, BoundaryTest )
{
    // Arrange
    RigPolygonTools::IntegerImage    image    = { { 1, 1, 1 }, { 1, 0, 1 }, { 1, 1, 1 } };
    std::vector<std::pair<int, int>> expected = { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 }, { 2, 1 }, { 2, 0 }, { 1, 0 } };

    // Act
    std::vector<std::pair<int, int>> result = RigPolygonTools::boundary( image );

    // Assert
    EXPECT_EQ( result, expected );
}

// Test for boundary function with invalid input
TEST( RigPolygonToolsTest, BoundaryInvalidInputTest )
{
    // Arrange
    RigPolygonTools::IntegerImage emptyImage = {};

    // Act & Assert
    EXPECT_TRUE( RigPolygonTools::boundary( emptyImage ).empty() );
    EXPECT_TRUE( RigPolygonTools::boundary( { { 1, 1 }, { 1 } } ).empty() ); // Inconsistent row sizes
}

// Test for simplifyPolygon function
TEST( RigPolygonToolsTest, SimplifyPolygonTest )
{
    // Arrange
    std::vector<cvf::Vec3d> vertices =
        { { 0.0, 0.0, 0.0 }, { 1.0, 0.1, 0.0 }, { 1.5, -0.1, 0.0 }, { 3.0, 5.0, 0.0 }, { 5.0, 6.0, 0.0 }, { 7.0, 7.0, 0.0 }, { 8.0, 8.0, 0.0 } };
    double                  epsilon  = 1;
    std::vector<cvf::Vec3d> expected = { { 0.0, 0.0, 0.0 }, { 1.5, -0.1, 0.0 }, { 3.0, 5.0, 0.0 }, { 8.0, 8.0, 0.0 } };

    // Act
    RigPolygonTools::simplifyPolygon( vertices, epsilon );

    // Assert
    EXPECT_EQ( vertices.size(), expected.size() );
    for ( size_t i = 0; i < vertices.size(); ++i )
    {
        EXPECT_DOUBLE_EQ( vertices[i].x(), expected[i].x() );
        EXPECT_DOUBLE_EQ( vertices[i].y(), expected[i].y() );
        EXPECT_DOUBLE_EQ( vertices[i].z(), expected[i].z() );
    }
}

// Test for simplifyPolygon function with invalid input
TEST( RigPolygonToolsTest, SimplifyPolygonInvalidInputTest )
{
    // Arrange
    std::vector<cvf::Vec3d> emptyVertices;
    double                  epsilon = 1.0;

    // Act & Assert
    EXPECT_NO_THROW( RigPolygonTools::simplifyPolygon( emptyVertices, epsilon ) );
    EXPECT_EQ( emptyVertices.size(), 0 );
}
