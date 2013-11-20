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
#include "cvfCamera.h"
#include "cvfViewport.h"
#include "cvfRay.h"
#include "cvfBoundingBox.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, DefaultValues)
{
    Camera c;

    EXPECT_EQ(40.0, c.fieldOfViewYDeg());
    EXPECT_EQ(0.05, c.nearPlane());
    EXPECT_EQ(10000.0, c.farPlane());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, ViewMatrix)
{
    ref<Camera> myCamera = new Camera;
    ASSERT_EQ(1, myCamera->refCount());

    // Default identity view matrix
    EXPECT_TRUE(myCamera->viewMatrix().isIdentity());
    
    myCamera->setViewMatrix(Mat4d::ZERO);
    EXPECT_TRUE(myCamera->viewMatrix() == Mat4d::ZERO);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, LookAt)
{
    Camera c;

    Vec3d eye, vrp, vup, viewDir;
    c.toLookAt(&eye, &vrp, &vup);
    viewDir = (vrp - eye).getNormalized();

    // Check default (like OpenGL)
    EXPECT_TRUE(eye == Vec3d::ZERO);
    EXPECT_TRUE(vup == Vec3d::Y_AXIS);
    EXPECT_TRUE(viewDir == -Vec3d::Z_AXIS);

    c.setFromLookAt(Vec3d(1,2,3), Vec3d(1,5,3), Vec3d(5,0,0));
    c.toLookAt(&eye, &vrp, &vup);
    viewDir = (vrp - eye).getNormalized();

    EXPECT_DOUBLE_EQ(1.0, eye.x());
    EXPECT_DOUBLE_EQ(2.0, eye.y());
    EXPECT_DOUBLE_EQ(3.0, eye.z());

    EXPECT_DOUBLE_EQ(0.0, viewDir.x());
    EXPECT_DOUBLE_EQ(1.0, viewDir.y());
    EXPECT_DOUBLE_EQ(0.0, viewDir.z());

    EXPECT_DOUBLE_EQ(1.0, vup.x());
    EXPECT_DOUBLE_EQ(0.0, vup.y());
    EXPECT_DOUBLE_EQ(0.0, vup.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, RetrieveComponents)
{
    Camera c;

    {
        const Vec3d eye(3.0, 0, 0);
        const Vec3d vrp(0, 0, 0);
        const Vec3d up(0, 0, 2.0);
        c.setFromLookAt(eye, vrp, up);
    }

    Vec3d pos = c.position();
    EXPECT_DOUBLE_EQ(3.0, pos.x());
    EXPECT_DOUBLE_EQ(0.0, pos.y());
    EXPECT_DOUBLE_EQ(0.0, pos.z());

    Vec3d dir = c.direction();
    EXPECT_DOUBLE_EQ(-1.0, dir.x());
    EXPECT_DOUBLE_EQ( 0.0, dir.y());
    EXPECT_DOUBLE_EQ( 0.0, dir.z());

    Vec3d up = c.up();
    EXPECT_DOUBLE_EQ( 0.0, up.x());
    EXPECT_DOUBLE_EQ( 0.0, up.y());
    EXPECT_DOUBLE_EQ( 1.0, up.z());

    Vec3d right = c.right();
    EXPECT_DOUBLE_EQ( 0.0, right.x());
    EXPECT_DOUBLE_EQ( 1.0, right.y());
    EXPECT_DOUBLE_EQ( 0.0, right.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, FitView)
{
    Camera c;

    BoundingBox bb;
    bb.add(Vec3d(0, 0, 0));
    bb.add(Vec3d(1, 1, 1));

    Vec3d inViewDir(0, 1, 0);
    Vec3d inUp(0, 0, 1);
    double coverageFactor = 0.5;

    c.fitView(bb, inViewDir, inUp, coverageFactor);

    Vec3d eye, vrp, vup, viewDir;
    c.toLookAt(&eye, &vrp, &vup);
    viewDir = (vrp - eye).getNormalized();
    
    EXPECT_DOUBLE_EQ(0.5, eye.x());
    EXPECT_DOUBLE_EQ(0.5, eye.z());

    // Hard to estimate Y here, but with rel factor 2, it should be around -3.3 or something
    EXPECT_LT(eye.y(), -3.0);
    EXPECT_GT(eye.y(), -3.5);

    EXPECT_DOUBLE_EQ(0.0, viewDir.x());
    EXPECT_DOUBLE_EQ(1.0, viewDir.y());
    EXPECT_DOUBLE_EQ(0.0, viewDir.z());

    EXPECT_DOUBLE_EQ(0.0, vup.x());
    EXPECT_DOUBLE_EQ(0.0, vup.y());
    EXPECT_DOUBLE_EQ(1.0, vup.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, setClipPlanesFromBoundingBox)
{
    Camera c;

    BoundingBox bb;
    bb.add(Vec3d(0, 0, 0));
    bb.add(Vec3d(1, 1, 1));

    c.setFromLookAt(Vec3d(0.5,3,0.5), Vec3d(0.5,0.5,0.5), Vec3d(0,0,1));

    double minNear = 0.005;
    c.setClipPlanesFromBoundingBox(bb, minNear);

    EXPECT_LT(c.nearPlane(), 2.0);
    EXPECT_GE(c.nearPlane(), 1.0);
    EXPECT_GE(c.nearPlane(), minNear);

    EXPECT_GE(c.farPlane(), 3.0);
    EXPECT_LE(c.farPlane(), 4.0);

    c.setFromLookAt(Vec3d(0.5,0.5,0.5), Vec3d(0.5,0,0.5), Vec3d(0,0,1));

    c.setClipPlanesFromBoundingBox(bb, minNear);
    EXPECT_DOUBLE_EQ(minNear, c.nearPlane());
    EXPECT_GE(c.farPlane(), 0.5);
    EXPECT_LE(c.farPlane(), 1.5);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, setProjectionAsPerspective)
{
    Camera c;
    c.viewport()->set(0,0,300, 200);
    c.setProjectionAsPerspective(50, 0.01, 20.0);

    EXPECT_EQ(Camera::PERSPECTIVE, c.projection());
    EXPECT_EQ(50.0, c.fieldOfViewYDeg());
    EXPECT_EQ(0.01, c.nearPlane());
    EXPECT_EQ(20.0, c.farPlane());

    EXPECT_NEAR(9.326153163e-3, c.frontPlaneFrustumHeight(), 1e-10);
    EXPECT_NEAR(9.326153163e-3/200, c.frontPlanePixelHeight(), 1e-10);

    Mat4d pm = c.projectionMatrix();

    // Check agains result of:
    //     glMatrixMode(GL_PROJECTION);
    //     glLoadIdentity();
    //     gluPerspective(50, 1.5, 0.01, 20.0);
    // 
    //     GLdouble adProjectionMatrix[16];
    //     glGetDoublev(GL_PROJECTION_MATRIX, adProjectionMatrix);
    //
    const double absErr = 1e-7;
    EXPECT_NEAR(1.4296712875366211, pm.rowCol(0,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(1,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(2,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(3,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(0,1), absErr);
    EXPECT_NEAR(2.1445069313049316, pm.rowCol(1,1), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(2,1), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(3,1), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(0,2), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(1,2), absErr);
    EXPECT_NEAR(-1.0010005235671997, pm.rowCol(2,2), absErr);
    EXPECT_NEAR(-1.0, pm.rowCol(3,2), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(0,3), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(1,3), absErr);
    EXPECT_NEAR(-0.020010005682706833, pm.rowCol(2,3), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(3,3), absErr);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, setProjectionAsOrtho)
{
    Camera c;
    c.viewport()->set(0,0,300, 200);
    c.setProjectionAsOrtho(4.0, 3.0, 8);

    EXPECT_EQ(Camera::ORTHO, c.projection());
    EXPECT_EQ(UNDEFINED_DOUBLE, c.fieldOfViewYDeg());
    EXPECT_EQ(3.0, c.nearPlane());
    EXPECT_EQ(8.0, c.farPlane());

    EXPECT_EQ(4.0, c.frontPlaneFrustumHeight());
    EXPECT_EQ(4.0/200, c.frontPlanePixelHeight());

    Mat4d pm = c.projectionMatrix();

    // Check agains result of:
    //     glMatrixMode(GL_PROJECTION);
    //     glLoadIdentity();
    //     glOrtho(-3, 3, -2, 2, 3, 8);
    // 
    //     GLdouble adProjectionMatrix[16];
    //     glGetDoublev(GL_PROJECTION_MATRIX, adProjectionMatrix);
    //
    const double absErr = 1e-7;
    EXPECT_NEAR(0.33333334326744080, pm.rowCol(0,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(1,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(2,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(3,0), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(0,1), absErr);
    EXPECT_NEAR(0.5, pm.rowCol(1,1), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(2,1), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(3,1), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(0,2), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(1,2), absErr);
    EXPECT_NEAR(-0.40000000596046448, pm.rowCol(2,2), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(3,2), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(0,3), absErr);
    EXPECT_NEAR(0.0, pm.rowCol(1,3), absErr);
    EXPECT_NEAR(-2.20000004768371583, pm.rowCol(2,3), absErr);
    EXPECT_NEAR(1.0, pm.rowCol(3,3), absErr);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, SetViewport)
{
    Camera c;

    c.setViewport(0, 1, 100, 200);

    EXPECT_EQ(0, c.viewport()->x());
    EXPECT_EQ(1, c.viewport()->y());
    EXPECT_EQ(100, c.viewport()->width());
    EXPECT_EQ(200, c.viewport()->height());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, RayFromWinCoord)
{
    Camera c;
    c.setFromLookAt(Vec3d(0,0,10), Vec3d(0,0,0), Vec3d(0,1,0));
    c.viewport()->set(0, 0, 300, 200);
    c.setProjectionAsPerspective(50, 0.01, 20.0);

    ref<Ray> ray = c.rayFromWindowCoordinates(150,100);
    Vec3d o = ray->origin();
    Vec3d d = ray->direction();

    ASSERT_TRUE(ray.notNull());
    ASSERT_DOUBLE_EQ(1.0, d.length());
    ASSERT_DOUBLE_EQ(0.0, o.x());
    ASSERT_DOUBLE_EQ(0.0, o.y());
    ASSERT_DOUBLE_EQ(9.99, o.z());
    ASSERT_DOUBLE_EQ(0.0, d.x());
    ASSERT_DOUBLE_EQ(0.0, d.y());
    ASSERT_DOUBLE_EQ(-1.0, d.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, PlaneFromLineWinCoord)
{
    Camera c;
    c.setFromLookAt(Vec3d(0,0,10), Vec3d(0,0,0), Vec3d(0,1,0));
    c.viewport()->set(0, 0, 300, 200);
    c.setProjectionAsPerspective(50, 0.01, 20.0);

    ref<Plane> plane = c.planeFromLineWindowCoordinates(Vec2i(100,100), Vec2i(200,100));
    ASSERT_TRUE(plane.notNull());

    // Should end up with normal pointing up
    Vec3d n = plane->normal().getNormalized();
    EXPECT_TRUE(n*Vec3d::Y_AXIS > 0.99);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, unproject)
{
    Camera c;

    c.setFromLookAt(Vec3d(0,0,10), Vec3d(0,0,0), Vec3d(0,1,0));
    c.viewport()->set(0, 0, 300, 200);
    c.setProjectionAsPerspective(50, 0.01, 20.0);

    Vec3d out;
    bool res = c.unproject(Vec3d(50,75,0), &out);

    ASSERT_TRUE(res);

    // Check agains result of:
    //     Camera* camera = m_snippet->camera();
    // 
    //     camera->setFromLookAt(Vec3d(0,0,10), Vec3d(0,0,0), Vec3d(0,1,0));
    //     camera->viewport()->set(0,0,300, 200);
    //     camera->setProjectionAsPerspective(50, 0.01, 20.0);
    // 
    //     Mat4d viewMat = camera->viewMatrix();
    //     Mat4d projMat = camera->projectionMatrix();
    // 
    // 
    //     GLdouble dWinCoord[3];
    //     GLdouble dCoord3D[3];
    //     GLint pVP[4];
    // 
    //     pVP[0] = (GLint) camera->viewport()->x();
    //     pVP[1] = (GLint) camera->viewport()->y();
    //     pVP[2] = (GLint) camera->viewport()->width();
    //     pVP[3] = (GLint) camera->viewport()->height();
    // 
    //     // Try and hit middle of pixel
    //     dWinCoord[0] = 50;
    //     dWinCoord[1] = 75;
    //     dWinCoord[2] = 0;
    // 
    //     // Get the point
    //     gluUnProject(dWinCoord[0], dWinCoord[1], dWinCoord[2], (GLdouble*)viewMat.ptr(), (GLdouble*)projMat.ptr(), pVP, &dCoord3D[0], &dCoord3D[1], &dCoord3D[2]);

    const double absErr = 1e-7;
    EXPECT_NEAR(-0.0046630765815497853, out.x(), absErr);
    EXPECT_NEAR(-0.0011657691453874461, out.y(), absErr);
    EXPECT_NEAR(9.9900000000000002, out.z(), absErr);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CameraTest, project)
{
    Camera c;

    c.setFromLookAt(Vec3d(0,0,10), Vec3d(0,0,0), Vec3d(0,1,0));
    c.viewport()->set(0, 0, 300, 200);
    c.setProjectionAsPerspective(50, 0.01, 20.0);

    // See unproject test above
    Vec3d point(-0.0046630765815497853,
                -0.0011657691453874461,
                9.9900000000000002);


    Vec3d out;
    bool res = c.project(point, &out);
    ASSERT_TRUE(res);

    const double absErr = 1e-7;
    EXPECT_NEAR(50, out.x(), absErr);
    EXPECT_NEAR(75, out.y(), absErr);
    EXPECT_NEAR(0, out.z(), absErr);
}
