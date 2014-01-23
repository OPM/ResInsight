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

#include "cvfMatrixState.h"
#include "cvfCamera.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MatrixStateTest, Constructor)
{
    MatrixState ms(Vec2i::ZERO, Vec2ui::ZERO, Mat4d::IDENTITY, Mat4d::IDENTITY);

    EXPECT_EQ(1, ms.versionTick());

    // Might get into trouble here due to internal double to float conversion
    Mat4f pm = ms.projectionMatrix();
    EXPECT_TRUE(pm.isIdentity());

    Mat4f vm = ms.viewMatrix();
    EXPECT_TRUE(vm.isIdentity());

    Mat4f mm = ms.modelMatrix();
    EXPECT_TRUE(mm.isIdentity());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MatrixStateTest, ModelViewMatrix)
{
    const Vec3d eye(0, -2, 0);
    const Vec3d vrp(0, 0, 0);
    const Vec3d up(0, 0, 1);

    Camera cam;
    cam.setFromLookAt(eye, vrp, up);
    MatrixState ms(Vec2i::ZERO, Vec2ui::ZERO, Mat4d::IDENTITY, cam.viewMatrix());

    {
        Mat4f mvm = ms.modelViewMatrix();
        Mat4f mvmi = ms.modelViewMatrixInverse();

        const Vec3f wc0(0, 0, 0);
        Vec3f ec = wc0.getTransformedPoint(mvm);
        EXPECT_FLOAT_EQ(0,  ec.x());
        EXPECT_FLOAT_EQ(0,  ec.y());
        EXPECT_FLOAT_EQ(-2, ec.z());

        Vec3f wc = ec.getTransformedPoint(mvmi);
        EXPECT_FLOAT_EQ(0, wc.x());
        EXPECT_FLOAT_EQ(0, wc.y());
        EXPECT_FLOAT_EQ(0, wc.z());
    }

    Mat4d transform = Mat4d::fromTranslation(Vec3d(2, 2, 0));
    ms.setModelMatrix(transform);
    {
        Mat4f mvm = ms.modelViewMatrix();
        Mat4f mvmi = ms.modelViewMatrixInverse();

        const Vec3f wc0(0, 0, 0);
        Vec3f ec = wc0.getTransformedPoint(mvm);
        EXPECT_FLOAT_EQ(2,  ec.x());
        EXPECT_FLOAT_EQ(0,  ec.y());
        EXPECT_FLOAT_EQ(-4, ec.z());

        Vec3f wc = ec.getTransformedPoint(mvmi);
        EXPECT_FLOAT_EQ(0, wc.x());
        EXPECT_FLOAT_EQ(0, wc.y());
        EXPECT_FLOAT_EQ(0, wc.z());
    }
}



