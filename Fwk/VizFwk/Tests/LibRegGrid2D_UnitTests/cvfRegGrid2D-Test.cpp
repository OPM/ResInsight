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
#include "cvfRegGrid2D.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, AllocateGrid)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(10, 20);
    EXPECT_EQ(10, regGrid2D->gridPointCountI());
    EXPECT_EQ(20, regGrid2D->gridPointCountJ());

    regGrid2D->allocateGrid(0, 0);
    EXPECT_EQ(0, regGrid2D->gridPointCountI());
    EXPECT_EQ(0, regGrid2D->gridPointCountJ());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef CVF_ENABLE_ASSERTS
TEST(RegGrid2DDeathTest, IllegalParametersAllocateGrid)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    EXPECT_DEATH(regGrid2D->allocateGrid(-1, 10), "Assertion");
    EXPECT_DEATH(regGrid2D->allocateGrid(10, -1), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, Spacing)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    EXPECT_EQ(0.0, regGrid2D->spacing().x());
    EXPECT_EQ(0.0, regGrid2D->spacing().y());

    regGrid2D->setSpacing(Vec2d(2.3, 5.4));
    EXPECT_EQ(2.3, regGrid2D->spacing().x());
    EXPECT_EQ(5.4, regGrid2D->spacing().y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef CVF_ENABLE_ASSERTS
TEST(RegGrid2DDeathTest, IllegalParametersSpacing)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    EXPECT_DEATH(regGrid2D->setSpacing(Vec2d(-1, 10)), "Assertion");
    EXPECT_DEATH(regGrid2D->setSpacing(Vec2d(10, -1)), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, Origo)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    EXPECT_EQ(0.0, regGrid2D->offset().x());
    EXPECT_EQ(0.0, regGrid2D->offset().y());

    regGrid2D->setOffset(Vec2d(2.3, 5.4));
    EXPECT_EQ(2.3, regGrid2D->offset().x());
    EXPECT_EQ(5.4, regGrid2D->offset().y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, ElevationValues)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());
    regGrid2D->allocateGrid(10, 20);

    DoubleArray values;
    values.resize(10 * 20);
    values.setAll(5);
    values.set(0, -10);
    values.set(1, 50);
    values.set(10 * 20 - 1, -99);
    regGrid2D->setElevations(values);

    double elevationValue = regGrid2D->elevation(0, 0);
    EXPECT_EQ(-10, elevationValue);

    elevationValue = regGrid2D->elevation(1, 0);
    EXPECT_EQ(50, elevationValue);

    elevationValue = regGrid2D->elevation(2, 0);
    EXPECT_EQ(5, elevationValue);

    elevationValue = regGrid2D->elevation(9, 19);
    EXPECT_EQ(-99, elevationValue);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef CVF_ENABLE_ASSERTS
TEST(RegGrid2DDeathTest, InvalidParametersElevation)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(10, 20);
    EXPECT_DEATH(regGrid2D->setElevation(-1, 0, 0), "Assertion");
    EXPECT_DEATH(regGrid2D->setElevation(10, 0, 0), "Assertion");
    EXPECT_DEATH(regGrid2D->setElevation(0, -1, 0), "Assertion");
    EXPECT_DEATH(regGrid2D->setElevation(0, 20, 0), "Assertion");

    DoubleArray values;
    values.resize(20 * 20);
    EXPECT_DEATH(regGrid2D->setElevations(values), "Assertion");
    EXPECT_DEATH(regGrid2D->setElevations(values.ptr(), 20*20), "Assertion");
}
#endif



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, PointEvaluation)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(2, 2);
    regGrid2D->setSpacing(Vec2d(1, 1));

    //  y
    //  /\                     (line cannot end with \)
    //  |
    // 4(0,1)   -2(1,1)
    //  |--------|
    //  |        |
    //  |        |
    //  |--------|---> x
    // 2(0,0)   5(1,0)
    regGrid2D->setElevation(0, 0,  2);
    regGrid2D->setElevation(1, 0,  5);
    regGrid2D->setElevation(0, 1,  4);
    regGrid2D->setElevation(1, 1, -2);


    // Reg grid corners
    {
        Vec2d coord(0, 0);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(2, elevationValue);
    }

    {
        Vec2d coord(1, 0);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(5, elevationValue);
    }

    {
        Vec2d coord(0, 1);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(4, elevationValue);
    }

    {
        Vec2d coord(1, 1);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(-2, elevationValue);
    }

    // Undefined on edge of grid, not possible to use bilinear interpolation
    {
        Vec2d coord(0.5, 1);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(UNDEFINED_DOUBLE, elevationValue);
    }

    double coordOffset = 0.00001;
    double deltaValue = 0.001;

    // Interpolation along sides
    {
        Vec2d coord(0.5, 0 + coordOffset);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_NEAR(3.5, elevationValue, 0.01);
    }
    {
        Vec2d coord(0.5, 1 - coordOffset);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_NEAR(1.0, elevationValue, deltaValue);
    }
    {
        Vec2d coord(0 + coordOffset, 0.5);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_NEAR(3.0, elevationValue, deltaValue);
    }

    {
        Vec2d coord(1 - coordOffset, 0.5);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_NEAR(1.5, elevationValue, deltaValue);
    }

    // Interpolation in center
    {
        Vec2d coord(0.5, 0.5);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(2.25, elevationValue);
    }


    // Interpolation of coordinates outside grid
    {
        Vec2d coord(-0.5, 0.5);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(elevationValue, UNDEFINED_DOUBLE);
    }

    {
        Vec2d coord(-0.5, -0.5);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(elevationValue, UNDEFINED_DOUBLE);
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, PointEvaluationOffset)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(2, 2);
    regGrid2D->setSpacing(Vec2d(1, 1));
    regGrid2D->setOffset(Vec2d(1, 1));

    //  y
    //  /\                     (line cannot end with \)
    //  |
    // 4(0,1)   -2(1,1)
    //  |--------|
    //  |        |
    //  |        |
    //  |--------|---> x
    // 2(0,0)   5(1,0)
    regGrid2D->setElevation(0, 0,  2);
    regGrid2D->setElevation(1, 0,  5);
    regGrid2D->setElevation(0, 1,  4);
    regGrid2D->setElevation(1, 1, -2);


    // Outside reg grid
    {
        Vec2d coord(0, 0);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(UNDEFINED_DOUBLE, elevationValue);
    }

    // Lower left corner
    {
        Vec2d coord(1, 1);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(2, elevationValue);
    }

    // Center right corner
    {
        Vec2d coord(1.5, 1.5);
        double elevationValue = regGrid2D->pointElevation(coord);
        EXPECT_EQ(2.25, elevationValue);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, MinMaxElevation)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(2, 2);

    DoubleArray values;
    values.resize(2 * 2);
    values.set(0, 2);
    values.set(1, 5);
    values.set(2, 4);
    values.set(3, -2);
    regGrid2D->setElevations(values);

    double min, max;
    regGrid2D->minMaxElevation(&min, &max);
    EXPECT_EQ(-2, min);
    EXPECT_EQ(5, max);

    regGrid2D->minMaxElevationInRegion(0, 0, 0, 0, &min, &max);
    EXPECT_EQ(2, min);
    EXPECT_EQ(2, max);

    regGrid2D->minMaxElevationInRegion(0, 0, 1, 0, &min, &max);
    EXPECT_EQ(2, min);
    EXPECT_EQ(5, max);

    regGrid2D->minMaxElevationInRegion(1, 1, 1, 1, &min, &max);
    EXPECT_EQ(-2, min);
    EXPECT_EQ(-2, max);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef CVF_ENABLE_ASSERTS
TEST(RegGrid2DDeathTest, InvalidParametersMinMaxElevation)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(10, 20);

    double min, max;
    EXPECT_DEATH(regGrid2D->minMaxElevationInRegion(-1, 0, 0, 0, &min, &max), "Assertion");
    EXPECT_DEATH(regGrid2D->minMaxElevationInRegion(0, -1, 0, 0, &min, &max), "Assertion");
    EXPECT_DEATH(regGrid2D->minMaxElevationInRegion(0, 0, -1, 0, &min, &max), "Assertion");
    EXPECT_DEATH(regGrid2D->minMaxElevationInRegion(0, 0, 0, -1, &min, &max), "Assertion");

    EXPECT_DEATH(regGrid2D->minMaxElevationInRegion(1, 0, 0, 0, &min, &max), "Assertion");
    EXPECT_DEATH(regGrid2D->minMaxElevationInRegion(0, 1, 0, 0, &min, &max), "Assertion");
}
#endif



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, MapLineSegmentOnGrid)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(4, 4);
    regGrid2D->setSpacing(Vec2d(1, 1));

    double delta = 0.001;

    // Test increasing ij
    {
        Vec3dArray intersections;

        Vec2d start(0.5, 0.5);
        Vec2d end(1.5, 2.5);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(5, intersections.size());

        EXPECT_NEAR(0.5, intersections[0].x(), delta);
        EXPECT_NEAR(0.5, intersections[0].y(), delta);

        EXPECT_NEAR(0.75, intersections[1].x(), delta);
        EXPECT_NEAR(1.0 , intersections[1].y(), delta);

        EXPECT_NEAR(1.0, intersections[2].x(), delta);
        EXPECT_NEAR(1.5, intersections[2].y(), delta);

        EXPECT_NEAR(1.25, intersections[3].x(), delta);
        EXPECT_NEAR(2.0 , intersections[3].y(), delta);

        EXPECT_NEAR(1.5, intersections[4].x(), delta);
        EXPECT_NEAR(2.5, intersections[4].y(), delta);
    }

    // Test increasing i, decreasing j
    {
        Vec3dArray intersections;

        Vec2d start(0.5, 2.5);
        Vec2d end(1.5, 0.5);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(5, intersections.size());
    }

    // Test decreasing i, increasing j
    {
        Vec3dArray intersections;

        Vec2d start(1.5, 0.5);
        Vec2d end(0.5, 2.5);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(5, intersections.size());
    }

    // Test decreasing ij
    {
        Vec3dArray intersections;

        Vec2d start(1.5, 2.5);
        Vec2d end(0.5, 0.5);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(5, intersections.size());
    }


    // Intersection at gridpoints
    {
        Vec3dArray intersections;

        Vec2d start(2.5, 2.5);
        Vec2d end(0.5, 0.5);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(4, intersections.size());
    }

    // Intersection at gridpoints, start and end at gridpoints
    {
        Vec3dArray intersections;

        Vec2d start(0, 0);
        Vec2d end(2, 2);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(3, intersections.size());
    }

    // Intersection at gridpoints, start and end at gridpoints
    {
        Vec3dArray intersections;

        Vec2d start(2, 2);
        Vec2d end(0, 0);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(3, intersections.size());
    }

    // Vertical line ascending
    {
        Vec3dArray intersections;

        Vec2d start(1.1, 0.1);
        Vec2d end(1.1, 2.1);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(4, intersections.size());
    }
    
    // Vertical line descending
    {
        Vec3dArray intersections;

        Vec2d start(1.1, 2.1);
        Vec2d end(1.1, 0.1);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(4, intersections.size());
    }

    // Horizontal line ascending
    {
        Vec3dArray intersections;

        Vec2d start(0.1, 1.1);
        Vec2d end(2.1, 1.1);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(4, intersections.size());
    }

    // Horizontal line descending
    {
        Vec3dArray intersections;

        Vec2d start(2.1, 1.1);
        Vec2d end(0.1, 1.1);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(4, intersections.size());
    }

    // Line starting outside grid
    {
        Vec3dArray intersections;

        Vec2d start(-1, 1);
        Vec2d end(10, 1);
        regGrid2D->mapLineSegmentOnGrid(start, end, &intersections);
        EXPECT_EQ(4, intersections.size());

        EXPECT_NEAR(0, intersections[0].x(), delta);
        EXPECT_NEAR(1, intersections[0].y(), delta);

        EXPECT_NEAR(1, intersections[1].x(), delta);
        EXPECT_NEAR(1, intersections[1].y(), delta);

        EXPECT_NEAR(2, intersections[2].x(), delta);
        EXPECT_NEAR(1, intersections[2].y(), delta);

        EXPECT_NEAR(3, intersections[3].x(), delta);
        EXPECT_NEAR(1, intersections[3].y(), delta);
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2D, GridCoordinate)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(4, 4);

    regGrid2D->setSpacing(Vec2d(1, 1));
    {
        Rectd boundingRect = regGrid2D->boundingRectangle();
        EXPECT_EQ(0.0, boundingRect.min().x());
        EXPECT_EQ(0.0, boundingRect.min().y());
        EXPECT_EQ(3.0, boundingRect.max().x());
        EXPECT_EQ(3.0, boundingRect.max().y());
    }

    regGrid2D->setSpacing(Vec2d(2, 2));
    {
        Rectd boundingRect = regGrid2D->boundingRectangle();
        EXPECT_EQ(0.0, boundingRect.min().x());
        EXPECT_EQ(0.0, boundingRect.min().y());
        EXPECT_EQ(6.0, boundingRect.max().x());
        EXPECT_EQ(6.0, boundingRect.max().y());
    }

    regGrid2D->setOffset(Vec2d(2, 2));
    regGrid2D->setSpacing(Vec2d(3, 3));
    {
        Rectd boundingRect = regGrid2D->boundingRectangle();
        EXPECT_EQ(2.0, boundingRect.min().x());
        EXPECT_EQ(2.0, boundingRect.min().y());
        EXPECT_EQ(11.0, boundingRect.max().x());
        EXPECT_EQ(11.0, boundingRect.max().y());
    }
}

