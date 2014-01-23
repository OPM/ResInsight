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
#include "cvfRay.h"
#include "cvfPlane.h"

#include "gtest/gtest.h"
#include "cvfBoundingBox.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, GettersAndSetters)
{
    Ray r;
    
    r.setOrigin(Vec3d(1,2,3));
    r.setDirection(Vec3d(0,1,0));

    EXPECT_TRUE(r.origin() == Vec3d(1,2,3));
    EXPECT_TRUE(r.direction() == Vec3d(0,1,0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, LimitedRangeSettersAndGetters)
{
    Ray r;

    EXPECT_TRUE(r.minimumDistance() > cvf::UNDEFINED_DOUBLE_THRESHOLD);
    EXPECT_TRUE(r.maximumDistance() > cvf::UNDEFINED_DOUBLE_THRESHOLD);

    r.setMinimumDistance(10.0);
    r.setMaximumDistance(20.0);
    EXPECT_DOUBLE_EQ(10.0, r.minimumDistance());
    EXPECT_DOUBLE_EQ(20.0, r.maximumDistance());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, LimitedRangeTriangleIntersect)
{
    Ray r;

    r.setOrigin(Vec3d::ZERO);
    r.setDirection(Vec3d::Z_AXIS);

    Vec3d v1 = Vec3d(-1, -1, 15);
    Vec3d v2 = Vec3d( 2, -1, 15);
    Vec3d v3 = Vec3d(-1,  2, 15);

    EXPECT_TRUE(r.triangleIntersect(v1, v2, v3));

    r.setMinimumDistance(10.0);
    r.setMaximumDistance(20.0);
    EXPECT_TRUE(r.triangleIntersect(v1, v2, v3));

    r.setMinimumDistance(10.0);
    r.setMaximumDistance(cvf::UNDEFINED_DOUBLE);
    EXPECT_TRUE(r.triangleIntersect(v1, v2, v3));

    r.setMinimumDistance(20.0);
    r.setMaximumDistance(cvf::UNDEFINED_DOUBLE);
    EXPECT_FALSE(r.triangleIntersect(v1, v2, v3));

    r.setMinimumDistance(cvf::UNDEFINED_DOUBLE);
    r.setMaximumDistance(20.0);
    EXPECT_TRUE(r.triangleIntersect(v1, v2, v3));

    r.setMinimumDistance(cvf::UNDEFINED_DOUBLE);
    r.setMaximumDistance(10.0);
    EXPECT_FALSE(r.triangleIntersect(v1, v2, v3));
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, LimitedRangeQuadIntersect)
{
    Ray r;

    r.setOrigin(Vec3d::ZERO);
    r.setDirection(Vec3d::Z_AXIS);

    Vec3d v1 = Vec3d(-1, -1, 15);
    Vec3d v2 = Vec3d( 2, -1, 15);
    Vec3d v3 = Vec3d( 2,  2, 15);
    Vec3d v4 = Vec3d(-1,  2, 15);

    EXPECT_TRUE(r.quadIntersect(v1, v2, v3, v4));

    r.setMinimumDistance(10.0);
    r.setMaximumDistance(20.0);
    EXPECT_TRUE(r.quadIntersect(v1, v2, v3, v4));

    r.setMinimumDistance(10.0);
    r.setMaximumDistance(cvf::UNDEFINED_DOUBLE);
    EXPECT_TRUE(r.quadIntersect(v1, v2, v3, v4));

    r.setMinimumDistance(20.0);
    r.setMaximumDistance(cvf::UNDEFINED_DOUBLE);
    EXPECT_FALSE(r.quadIntersect(v1, v2, v3, v4));

    r.setMinimumDistance(cvf::UNDEFINED_DOUBLE);
    r.setMaximumDistance(20.0);
    EXPECT_TRUE(r.quadIntersect(v1, v2, v3, v4));

    r.setMinimumDistance(cvf::UNDEFINED_DOUBLE);
    r.setMaximumDistance(10.0);
    EXPECT_FALSE(r.quadIntersect(v1, v2, v3, v4));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, LimitedRangeBoxIntersect)
{
    Ray r;

    r.setOrigin(Vec3d::ZERO);
    r.setDirection(Vec3d::Z_AXIS);

    BoundingBox box(Vec3d(-1, -1, 14), Vec3d(1,1,16));
    EXPECT_TRUE(r.boxIntersect(box));

    r.setMinimumDistance(10.0);
    r.setMaximumDistance(20.0);
    EXPECT_TRUE(r.boxIntersect(box));

    r.setMinimumDistance(20.0);
    r.setMaximumDistance(50.0);
    EXPECT_FALSE(r.boxIntersect(box));

    r.setMinimumDistance(0.0);
    r.setMaximumDistance(10.0);
    EXPECT_FALSE(r.boxIntersect(box));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, LimitedRangePlaneIntersect)
{
    Ray r;

    r.setOrigin(Vec3d::ZERO);
    r.setDirection(Vec3d::Z_AXIS);

    Plane plane(0,0,1,-15);
    EXPECT_TRUE(r.planeIntersect(plane));

    r.setMinimumDistance(10.0);
    r.setMaximumDistance(20.0);
    EXPECT_TRUE(r.planeIntersect(plane));

    r.setMinimumDistance(20.0);
    r.setMaximumDistance(50.0);
    EXPECT_FALSE(r.planeIntersect(plane));

    r.setMinimumDistance(0.0);
    r.setMaximumDistance(10.0);
    EXPECT_FALSE(r.planeIntersect(plane));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, Transform)
{
    Ray r;

    r.setOrigin(Vec3d(1,2,3));
    r.setDirection(Vec3d(0,1,0));

    Mat4d m;
    Vec3d t(1,2,3);
    m.setTranslation(t);

    r.transform(m);

    EXPECT_EQ(2, r.origin().x());
    EXPECT_EQ(4, r.origin().y());
    EXPECT_EQ(6, r.origin().z());
    EXPECT_EQ(0, r.direction().x());
    EXPECT_EQ(1, r.direction().y());
    EXPECT_EQ(0, r.direction().z());

    Ray r2 = r.getTransformed(m);

    EXPECT_EQ(2, r.origin().x());
    EXPECT_EQ(4, r.origin().y());
    EXPECT_EQ(6, r.origin().z());
    EXPECT_EQ(0, r.direction().x());
    EXPECT_EQ(1, r.direction().y());
    EXPECT_EQ(0, r.direction().z());

    EXPECT_EQ(3, r2.origin().x());
    EXPECT_EQ(6, r2.origin().y());
    EXPECT_EQ(9, r2.origin().z());
    EXPECT_EQ(0, r2.direction().x());
    EXPECT_EQ(1, r2.direction().y());
    EXPECT_EQ(0, r2.direction().z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RayTest, PlaneInersect)
{
    {
        Ray r;
        r.setOrigin(Vec3d(1, 2, 5));
        r.setDirection(Vec3d(0, 0, -5));

        // Plane normal going towards ray
        Plane p1;
        p1.setFromPointAndNormal(Vec3d(0, 0, 4), Vec3d(0, 0, 7));

        Vec3d isect(0, 0, 0);
        EXPECT_TRUE(r.planeIntersect(p1, &isect));
        EXPECT_DOUBLE_EQ(1.0, isect.x());
        EXPECT_DOUBLE_EQ(2.0, isect.y());
        EXPECT_DOUBLE_EQ(4.0, isect.z());

        // Plane normal facing 'away' from plane
        Plane p2;
        p2.setFromPointAndNormal(Vec3d(0, 0, 4), Vec3d(0, 0, -7));

        isect.setZero();
        EXPECT_TRUE(r.planeIntersect(p2, &isect));
        EXPECT_DOUBLE_EQ(1.0, isect.x());
        EXPECT_DOUBLE_EQ(2.0, isect.y());
        EXPECT_DOUBLE_EQ(4.0, isect.z());
    }

    {
        // Ray origin behind the plane
        Ray r;
        r.setOrigin(Vec3d(1, 2, 3));
        r.setDirection(Vec3d(0, 0, -5));

        Plane p;
        p.setFromPointAndNormal(Vec3d(0, 0, 4), Vec3d(0, 0, 7));

        Vec3d isect(0, 0, 0);
        EXPECT_FALSE(r.planeIntersect(p, &isect));
        EXPECT_DOUBLE_EQ(0.0, isect.x());
        EXPECT_DOUBLE_EQ(0.0, isect.y());
        EXPECT_DOUBLE_EQ(0.0, isect.z());
    }

    {
        // Ray lying in plane
        Ray r;
        r.setOrigin(Vec3d(1, 2, 3));
        r.setDirection(Vec3d(0, 2, 3));

        Plane p;
        p.setFromPointAndNormal(Vec3d(4, 5, 6), Vec3d(1, 0, 0));

        EXPECT_FALSE(r.planeIntersect(p, NULL));
    }

    {
        // Ray lying in plane
        Ray r;
        r.setOrigin(Vec3d(-6, -7, 3));
        r.setDirection(Vec3d(10, 0, -14));

        Plane p;
        p.setFromPointAndNormal(Vec3d(4, 5, -6), Vec3d(0, -6, 0));

        EXPECT_FALSE(r.planeIntersect(p, NULL));
    }
}

