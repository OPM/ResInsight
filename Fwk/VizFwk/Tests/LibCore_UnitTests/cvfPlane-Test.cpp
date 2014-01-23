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
#include "cvfPlane.h"
#include "cvfMath.h"

#include <cmath>

#include "gtest/gtest.h"


using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, BasicConstruction)
{
    Plane plane1;
    EXPECT_FALSE(plane1.isValid());

    Plane plane2(1.0, 2.0, 3.0, 4.0);
    EXPECT_TRUE(plane2.isValid());

    Plane plane3(plane2);
    EXPECT_TRUE(plane3.isValid());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, Operators)
{
    Plane plane1;
    Plane plane2(1.0, 2.0, 3.0, 4.0);
    Plane plane3(plane2);
    Plane plane4 = plane3;  // Assignment operator


    // == operator
    // --------------------------------------

    // Test plane1
    EXPECT_FALSE(plane1==plane2);
    EXPECT_FALSE(plane1==plane3);
    EXPECT_FALSE(plane1==plane4);

    // Test plane2
    EXPECT_FALSE(plane2==plane1);
    EXPECT_TRUE( plane2==plane3);
    EXPECT_TRUE( plane2==plane4);

    // Test plane3
    EXPECT_FALSE(plane3==plane1);
    EXPECT_TRUE( plane3==plane2);
    EXPECT_TRUE( plane3==plane4);

    // Test plane4
    EXPECT_FALSE(plane4==plane1);
    EXPECT_TRUE( plane4==plane2);
    EXPECT_TRUE( plane4==plane3);


    // != operator
    // --------------------------------------

    // Test plane1
    EXPECT_TRUE( plane1!=plane2);
    EXPECT_TRUE( plane1!=plane3);
    EXPECT_TRUE( plane1!=plane4);

    // Test plane2
    EXPECT_TRUE( plane2!=plane1);
    EXPECT_FALSE(plane2!=plane3);
    EXPECT_FALSE(plane2!=plane4);

    // Test plane3
    EXPECT_TRUE( plane3!=plane1);
    EXPECT_FALSE(plane3!=plane2);
    EXPECT_FALSE(plane3!=plane4);

    // Test plane4
    EXPECT_TRUE( plane4!=plane1);
    EXPECT_FALSE(plane4!=plane2);
    EXPECT_FALSE(plane4!=plane3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, IsValid)
{
    Plane plane1;
    EXPECT_FALSE(plane1.isValid());

    Plane plane2(0.0, 0.0, 0.0, 0.0);
    EXPECT_FALSE(plane2.isValid());

    Plane plane3(1.0, 2.0, 3.0, 4.0);
    EXPECT_TRUE(plane3.isValid());

    plane2 = plane3;
    EXPECT_TRUE(plane2.isValid());
    
    plane2 = plane1;
    EXPECT_FALSE(plane2.isValid());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, Set)
{
    Plane plane;
    plane.set(7.0, -5.0, -3.0, -4.0);
    
    // Points which should be located in the plane
    Vec3d p1(1.0, 0.0, 1.0);
    Vec3d p2(2.0, 2.0, 0.0);
    Vec3d p3(3.0, 1.0, 4.0);

    EXPECT_DOUBLE_EQ(0.0, plane.distance(p1));
    EXPECT_DOUBLE_EQ(0.0, plane.distance(p2));
    EXPECT_DOUBLE_EQ(0.0, plane.distance(p3));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(PlaneDeathTest, Set)
{
    Plane plane;
    EXPECT_DEATH(plane.set(0.0, 0.0, 0.0, 0.0), "Assertion");
}
#endif



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, SetFromPointAndNormal)
{
    Vec3d point(1.0, 2.0, 3.0);
    Vec3d normal(1.0, 8.0, 6.0);

    // Define a plane by point and normal
    Plane plane;
    EXPECT_TRUE(plane.setFromPointAndNormal(point, normal));

    // Project a point we know is on the plane down on the plane itself. 
    // This projected point should be the same as the unprojected point
    Vec3d projectedPoint = plane.projectPoint(point);
    EXPECT_DOUBLE_EQ(point.x(), projectedPoint.x());
    EXPECT_DOUBLE_EQ(point.y(), projectedPoint.y());
    EXPECT_DOUBLE_EQ(point.z(), projectedPoint.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, SetFromPoints)
{
    Vec3d p1(1.0, 2.0, 3.0);
    Vec3d p2(4.0, 5.0, 6.0);
    Vec3d p3(7.0, 8.0, 9.0);
    Vec3d p4(1.0, 1.1, 2.42);
    Vec3d p5(3.0, 5.1, 1.22);
    Vec3d normal(1.0, 2.0, 3.0);
    Plane plane;

    // Define a plane from three points on a line, which should fail
    EXPECT_FALSE(plane.setFromPoints(p1, p2, p3));

    // Define a plane by 3 points not aligned on a line
    EXPECT_TRUE(plane.setFromPoints(p1, p4, p5));

    // Project a point we know is on the plane down on the plane itself. 
    // This projected point should be the same as the unprojected point
    Vec3d pointInPlane = (p1+p4+p5)/3.0;
    Vec3d projectedPoint = plane.projectPoint(pointInPlane);
    EXPECT_DOUBLE_EQ(pointInPlane.x(), projectedPoint.x());
    EXPECT_DOUBLE_EQ(pointInPlane.y(), projectedPoint.y());
    EXPECT_DOUBLE_EQ(pointInPlane.z(), projectedPoint.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, Normal)
{
    Vec3d point(1.0, 2.0, 3.0);
    Vec3d normal(1.0, 8.0, 6.0);
    Plane plane;
    
    EXPECT_TRUE(plane.setFromPointAndNormal(point, normal));

    Vec3d planeNormal = plane.normal();
        
    EXPECT_DOUBLE_EQ(normal.x(), planeNormal.x());
    EXPECT_DOUBLE_EQ(normal.y(), planeNormal.y());
    EXPECT_DOUBLE_EQ(normal.z(), planeNormal.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, PointInPlane)
{
    Vec3d p1(1.0, 0.0, 1.0);
    Vec3d p2(2.0, 2.0, 0.0);
    Vec3d p3(3.0, 1.0, 4.0);
    Plane plane;
    plane.setFromPoints(p1, p2, p3);

    // Get a point somewhere on the plane
    Vec3d pip = plane.pointInPlane();

    // Project the point we already know is on the plane, down onto the plane
    // These two points should be the same
    Vec3d projectedPoint = plane.projectPoint(pip);
    EXPECT_DOUBLE_EQ(pip.x(), projectedPoint.x());
    EXPECT_DOUBLE_EQ(pip.y(), projectedPoint.y());
    EXPECT_DOUBLE_EQ(pip.z(), projectedPoint.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, Flip)
{
    Vec3d point(1.0, 2.0, 3.0);
    Vec3d normal(1.0, 0.0, 0.0);
    Plane plane1;
    plane1.setFromPointAndNormal(point, normal);
    Plane plane2 = plane1;

    plane2.flip();
    EXPECT_FALSE(plane1.normal() == plane2.normal());
    EXPECT_TRUE(plane1.normal() == -plane2.normal());

    plane2.flip();
    EXPECT_TRUE(plane1.normal() == plane2.normal());
    EXPECT_FALSE(plane1.normal() == -plane2.normal());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, Transform)
{
    // Todo: Add test
    //       void transform(const Mat4d& mat);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, Distance)
{
    Vec3d p1(1.0, 0.0, 1.0);
    Vec3d p2(2.0, 2.0, 0.0);
    Vec3d p3(3.0, 1.0, 4.0);
    Plane plane;
    plane.setFromPoints(p1, p2, p3);
    Vec3d pip = plane.pointInPlane();


    // Test distance(..) by pulling a point on the plane in 
    // the direction of the normal
    // ------------------------------------------------------
    EXPECT_DOUBLE_EQ(5.0, plane.distance(pip + plane.normal().getNormalized()*5.0));
    EXPECT_DOUBLE_EQ(-6.0, plane.distance(pip + plane.normal().getNormalized()*(-6.0)));


    // Test distanceSquared by checking the sign
    // -> which side the point is on
    // ------------------------------------------------------
    double distanceSquaredPos = plane.distanceSquared(pip + plane.normal()*5.0);
    double distanceSquaredNeg = plane.distanceSquared(pip + plane.normal()*(-5.0));
    EXPECT_TRUE(distanceSquaredPos > 0.0);
    EXPECT_TRUE(distanceSquaredNeg < 0.0);


    // Test distanceSquared() against distance()
    // ------------------------------------------------------
    double factor = sqrt(plane.A()*plane.A() + plane.B()*plane.B() + plane.C()*plane.C());

    EXPECT_DOUBLE_EQ(distanceSquaredPos/factor, plane.distance(pip + plane.normal()*5.0));
    EXPECT_DOUBLE_EQ(distanceSquaredNeg/factor, plane.distance(pip + plane.normal()*(-5.0)));


    // Test distanceToOrigin();
    // ------------------------------------
    Vec3d p4(1.234, 0.0, 0.0);
    plane.setFromPointAndNormal(p4, p4.getNormalized());
    EXPECT_DOUBLE_EQ(1.234, plane.distanceToOrigin());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, ProjectPointAndVector)
{
    Vec3d point(1.0, 2.0, 3.0);
    Vec3d normal(1.0, 8.0, 6.0);
    Plane plane;
    plane.setFromPointAndNormal(point, normal);

    // Get a point somewhere on the plane
    Vec3d pip = plane.pointInPlane();

    // Project the point we already know is on the plane, down onto the plane
    // These two points should be the same
    Vec3d projectedPoint = plane.projectPoint(pip);
    EXPECT_DOUBLE_EQ(pip.x(), projectedPoint.x());
    EXPECT_DOUBLE_EQ(pip.y(), projectedPoint.y());
    EXPECT_DOUBLE_EQ(pip.z(), projectedPoint.z());

    // projectVector(..) is used directly by projectPoint(..) and is as such automatically tested above
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, PlaneAndPlaneIntersect)
{
    Plane plane1;
    plane1.setFromPointAndNormal(Vec3d::X_AXIS, Vec3d::X_AXIS);
    Plane plane2;
    plane2.setFromPointAndNormal(Vec3d::Y_AXIS, Vec3d::Y_AXIS);

    Vec3d point, normal;
    EXPECT_TRUE(plane1.intersect(plane2, &point, &normal));
    EXPECT_TRUE(normal == Vec3d::Z_AXIS);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, PlaneAndLineIntersect)
{
    // a above, b below
    {
        Plane plane;
        plane.setFromPointAndNormal(Vec3d::ZERO, Vec3d::Z_AXIS);
    
        cvf::Vec3d a(10.0, 10.0, 10.0);
        cvf::Vec3d b(10.0, 10.0, -10.0);
    
        cvf::Vec3d intersection;

        EXPECT_TRUE(plane.intersect(a, b, &intersection));
        EXPECT_TRUE(intersection == Vec3d(10.0, 10.0, 0.0));
    }

    // a below, b above
    {
        Plane plane;
        plane.setFromPointAndNormal(Vec3d::ZERO, Vec3d::Z_AXIS);

        cvf::Vec3d a(10.0, 10.0, -10.0);
        cvf::Vec3d b(10.0, 10.0, 10.0);

        cvf::Vec3d intersection;

        EXPECT_TRUE(plane.intersect(a, b, &intersection));
        EXPECT_TRUE(intersection == Vec3d(10.0, 10.0, 0.0));
    }

    // b on plane
    {
        Plane plane;
        plane.setFromPointAndNormal(Vec3d::ZERO, Vec3d::Z_AXIS);

        cvf::Vec3d a(10.0, 10.0, 10.0);
        cvf::Vec3d b(10.0, 10.0, 0.0);

        cvf::Vec3d intersection;

        EXPECT_TRUE(plane.intersect(a, b, &intersection));
        EXPECT_TRUE(intersection == Vec3d(10.0, 10.0, 0.0));
    }

    // a and b above plane
    {
        Plane plane;
        plane.setFromPointAndNormal(Vec3d::ZERO, Vec3d::Z_AXIS);

        cvf::Vec3d a(10.0, 10.0, 10.0);
        cvf::Vec3d b(10.0, 10.0, 1.0);

        cvf::Vec3d intersection;

        EXPECT_FALSE(plane.intersect(a, b, &intersection));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, SideForPoint)
{
    Plane plane;
    plane.setFromPointAndNormal(Vec3d::ZERO, Vec3d::Z_AXIS);

    // Front
    {
        cvf::Vec3d a = Vec3d::Z_AXIS;
        EXPECT_TRUE(Plane::FRONT == plane.side(a));
    }

    // On
    {
        cvf::Vec3d a = Vec3d::ZERO;
        EXPECT_TRUE(Plane::ON == plane.side(a));
    }

    // Back
    {
        cvf::Vec3d a = -Vec3d::Z_AXIS;
        EXPECT_TRUE(Plane::BACK == plane.side(a));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PlaneTest, SideForPoints)
{
    Plane plane;
    plane.setFromPointAndNormal(Vec3d::ZERO, Vec3d::Z_AXIS);

    cvf::Vec3d pointFront(Vec3d::Z_AXIS);
    cvf::Vec3d pointOn(Vec3d::ZERO);
    cvf::Vec3d pointBack(-Vec3d::Z_AXIS);

    // All front
    {
        cvf::Vec3dArray a;
        a.reserve(10);

        a.add(pointFront);
        EXPECT_TRUE(Plane::FRONT == plane.side(a));

        a.add(pointOn);
        EXPECT_TRUE(Plane::FRONT == plane.side(a));
    }

    // All on
    {
        cvf::Vec3dArray a;
        a.reserve(10);

        a.add(pointOn);
        a.add(pointOn);
        EXPECT_TRUE(Plane::ON == plane.side(a));
    }

    // All back
    {
        cvf::Vec3dArray a;
        a.reserve(10);

        a.add(pointBack);
        EXPECT_TRUE(Plane::BACK == plane.side(a));

        a.add(pointOn);
        EXPECT_TRUE(Plane::BACK == plane.side(a));
    }

    // Both sides
    {
        cvf::Vec3dArray a;
        a.reserve(10);

        a.add(pointFront);
        a.add(pointBack);
        EXPECT_TRUE(Plane::BOTH == plane.side(a));

        a.add(pointOn);
        EXPECT_TRUE(Plane::BOTH == plane.side(a));
    }
}
