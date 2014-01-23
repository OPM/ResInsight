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
#include "cvfFrustum.h"
#include "cvfVector3.h"
#include "cvfBoundingBox.h"

#include "gtest/gtest.h"


using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FrustumTest, BasicConstruction)
{
    Frustum f1;
    Frustum f2(f1);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FrustumTest, Operators)
{
    Frustum f1;
    Frustum f2;

    Vec3d center(0.0, 0.0, 0.0);
    double radius = 1.0;

    Plane planeBottom; planeBottom.setFromPointAndNormal(center - (Vec3d::Y_AXIS*radius),  Vec3d::Y_AXIS);
    Plane planeTop;    planeTop.setFromPointAndNormal(   center + (Vec3d::Y_AXIS*radius), -Vec3d::Y_AXIS);
    Plane planeLeft;   planeLeft.setFromPointAndNormal(  center - (Vec3d::X_AXIS*radius),  Vec3d::X_AXIS);
    Plane planeRight;  planeRight.setFromPointAndNormal( center + (Vec3d::X_AXIS*radius), -Vec3d::X_AXIS);
    Plane planeFront;  planeFront.setFromPointAndNormal( center - (Vec3d::Z_AXIS*radius),  Vec3d::Z_AXIS);
    Plane planeBack;   planeBack.setFromPointAndNormal(  center + (Vec3d::Z_AXIS*radius), -Vec3d::Z_AXIS);

    f1.setPlane(Frustum::BOTTOM, planeBottom);
    f1.setPlane(Frustum::TOP,    planeTop);
    f1.setPlane(Frustum::LEFT,   planeLeft);
    f1.setPlane(Frustum::RIGHT,  planeRight);
    f1.setPlane(Frustum::FRONT,  planeFront);
    f1.setPlane(Frustum::BACK,   planeBack);

    EXPECT_FALSE(f1 == f2);
    EXPECT_TRUE(f1 != f2);

    f1 = f2;
    EXPECT_TRUE(f1 == f2);
    EXPECT_FALSE(f1 != f2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FrustumTest, SetGetPlane)
{
    Frustum f;

    Vec3d center(0.0, 0.0, 0.0);
    double radius = 1.0;

    Plane planeBottom; planeBottom.setFromPointAndNormal(center - (Vec3d::Y_AXIS*radius),  Vec3d::Y_AXIS);
    Plane planeTop;    planeTop.setFromPointAndNormal(   center + (Vec3d::Y_AXIS*radius), -Vec3d::Y_AXIS);
    Plane planeLeft;   planeLeft.setFromPointAndNormal(  center - (Vec3d::X_AXIS*radius),  Vec3d::X_AXIS);
    Plane planeRight;  planeRight.setFromPointAndNormal( center + (Vec3d::X_AXIS*radius), -Vec3d::X_AXIS);
    Plane planeFront;  planeFront.setFromPointAndNormal( center - (Vec3d::Z_AXIS*radius),  Vec3d::Z_AXIS);
    Plane planeBack;   planeBack.setFromPointAndNormal(  center + (Vec3d::Z_AXIS*radius), -Vec3d::Z_AXIS);

    f.setPlane(Frustum::BOTTOM, planeBottom);
    f.setPlane(Frustum::TOP,    planeTop);
    f.setPlane(Frustum::LEFT,   planeLeft);

    Plane fPlaneBottom = f.plane(Frustum::BOTTOM);
    Plane fPlaneTop    = f.plane(Frustum::TOP);
    Plane fPlaneLeft   = f.plane(Frustum::LEFT);
    Plane fPlaneRight  = f.plane(Frustum::RIGHT);
    Plane fPlaneFront  = f.plane(Frustum::FRONT);
    Plane fPlaneBack   = f.plane(Frustum::BACK);

    // Check existence
    EXPECT_TRUE( fPlaneBottom.isValid());
    EXPECT_TRUE( fPlaneTop.isValid());
    EXPECT_TRUE( fPlaneLeft.isValid());
    EXPECT_FALSE(fPlaneRight.isValid());
    EXPECT_FALSE(fPlaneFront.isValid());
    EXPECT_FALSE(fPlaneBack.isValid());

    // Check if the planes are correct
    EXPECT_TRUE(fPlaneBottom == planeBottom);
    EXPECT_TRUE(fPlaneTop == planeTop);
    EXPECT_TRUE(fPlaneLeft == planeLeft);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FrustumTest, IsOutsidePoint)
{
    Frustum f;

    Vec3d center(0.0, 0.0, 0.0);
    double radius = 1.0;

    Plane planeBottom; planeBottom.setFromPointAndNormal(center - (Vec3d::Y_AXIS*radius),  Vec3d::Y_AXIS);
    Plane planeTop;    planeTop.setFromPointAndNormal(   center + (Vec3d::Y_AXIS*radius), -Vec3d::Y_AXIS);
    Plane planeLeft;   planeLeft.setFromPointAndNormal(  center - (Vec3d::X_AXIS*radius),  Vec3d::X_AXIS);
    Plane planeRight;  planeRight.setFromPointAndNormal( center + (Vec3d::X_AXIS*radius), -Vec3d::X_AXIS);
    Plane planeFront;  planeFront.setFromPointAndNormal( center - (Vec3d::Z_AXIS*radius),  Vec3d::Z_AXIS);
    Plane planeBack;   planeBack.setFromPointAndNormal(  center + (Vec3d::Z_AXIS*radius), -Vec3d::Z_AXIS);

    f.setPlane(Frustum::BOTTOM, planeBottom);
    f.setPlane(Frustum::TOP,    planeTop);
    f.setPlane(Frustum::LEFT,   planeLeft);
    f.setPlane(Frustum::RIGHT,  planeRight);
    f.setPlane(Frustum::FRONT,  planeFront);
    f.setPlane(Frustum::BACK,   planeBack);

    EXPECT_FALSE(f.isOutside(center + Vec3d::X_AXIS*radius*0.9));  // Inside
    EXPECT_FALSE(f.isOutside(center + Vec3d::X_AXIS*radius));      // On boundary
    EXPECT_TRUE( f.isOutside(center + Vec3d::X_AXIS*radius*1.1));  // Outside

    EXPECT_FALSE(f.isOutside(center + Vec3d::Y_AXIS*radius*0.9));  // Inside
    EXPECT_FALSE(f.isOutside(center + Vec3d::Y_AXIS*radius));      // On boundary
    EXPECT_TRUE( f.isOutside(center + Vec3d::Y_AXIS*radius*1.1));  // Outside

    EXPECT_FALSE(f.isOutside(center + Vec3d::Z_AXIS*radius*0.9));  // Inside
    EXPECT_FALSE(f.isOutside(center + Vec3d::Z_AXIS*radius));      // On boundary
    EXPECT_TRUE( f.isOutside(center + Vec3d::Z_AXIS*radius*1.1));  // Outside
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FrustumTest, IsOutsideBoundingBox)
{
    Frustum f;

    Vec3d center(0.0, 0.0, 0.0);
    double radius = 1.0;

    Plane planeBottom; planeBottom.setFromPointAndNormal(center - (Vec3d::Y_AXIS*radius),  Vec3d::Y_AXIS);
    Plane planeTop;    planeTop.setFromPointAndNormal(   center + (Vec3d::Y_AXIS*radius), -Vec3d::Y_AXIS);
    Plane planeLeft;   planeLeft.setFromPointAndNormal(  center - (Vec3d::X_AXIS*radius),  Vec3d::X_AXIS);
    Plane planeRight;  planeRight.setFromPointAndNormal( center + (Vec3d::X_AXIS*radius), -Vec3d::X_AXIS);
    Plane planeFront;  planeFront.setFromPointAndNormal( center - (Vec3d::Z_AXIS*radius),  Vec3d::Z_AXIS);
    Plane planeBack;   planeBack.setFromPointAndNormal(  center + (Vec3d::Z_AXIS*radius), -Vec3d::Z_AXIS);

    f.setPlane(Frustum::BOTTOM, planeBottom);
    f.setPlane(Frustum::TOP,    planeTop);
    f.setPlane(Frustum::LEFT,   planeLeft);
    f.setPlane(Frustum::RIGHT,  planeRight);
    f.setPlane(Frustum::FRONT,  planeFront);
    f.setPlane(Frustum::BACK,   planeBack);

    Vec3d min = center - (Vec3d::X_AXIS*radius) - (Vec3d::Y_AXIS*radius) - (Vec3d::Z_AXIS*radius);
    Vec3d max = center + (Vec3d::X_AXIS*radius) + (Vec3d::Y_AXIS*radius) + (Vec3d::Z_AXIS*radius);
    Vec3d diagonal = max - min;


    // Inside tests
    // -------------------------------------------------------------------------

    // Test when frustum = boundingbox
    EXPECT_FALSE(f.isOutside(BoundingBox(min, max)));

    // Test when boundingbox is inside frustum
    EXPECT_FALSE(f.isOutside(BoundingBox(min + diagonal*0.1, max - diagonal*0.1)));

    // Test when boundingbox envelops the frustum frustum
    EXPECT_FALSE(f.isOutside(BoundingBox(min - diagonal*0.1, max + diagonal*0.1)));

    // Test when min is inside and max outside frustum (Partly inside and outside)
    EXPECT_FALSE(f.isOutside(BoundingBox(min + diagonal*0.1, max + diagonal*0.1)));

    // Test when min is outside and max inside frustum (Partly inside and outside)
    EXPECT_FALSE(f.isOutside(BoundingBox(min - diagonal*0.1, max - diagonal*0.1)));


    // Outside test
    // -------------------------------------------------------------------------

    // Test both min an max is below and to the left of the frustum
    EXPECT_TRUE(f.isOutside(BoundingBox(min - diagonal*0.5, max - diagonal*1.4)));

    // Test both min an max is above and to the right of the frustum
    EXPECT_TRUE(f.isOutside(BoundingBox(min + diagonal*1.4, max + diagonal*0.5)));
}


