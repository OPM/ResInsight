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
#include "cvfQuat.h"
#include "cvfMatrix4.h"
#include "cvfMath.h"

#include "gtest/gtest.h"
#include <cmath>

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(QuatTest, ConstructionAndAssigmentOperator)
{
    {
        Quat<double> q1;
        ASSERT_DOUBLE_EQ(0, q1.x());
        ASSERT_DOUBLE_EQ(0, q1.y());
        ASSERT_DOUBLE_EQ(0, q1.z());
        ASSERT_DOUBLE_EQ(0, q1.w());

        Quat<double> q2(1, 2, 3, 4);
        ASSERT_DOUBLE_EQ(1, q2.x());
        ASSERT_DOUBLE_EQ(2, q2.y());
        ASSERT_DOUBLE_EQ(3, q2.z());
        ASSERT_DOUBLE_EQ(4, q2.w());

        Quat<double> q3(q2);
        ASSERT_DOUBLE_EQ(1, q3.x());
        ASSERT_DOUBLE_EQ(2, q3.y());
        ASSERT_DOUBLE_EQ(3, q3.z());
        ASSERT_DOUBLE_EQ(4, q3.w());

        Quat<double> q4(-1, -2, -3, -4);
        ASSERT_DOUBLE_EQ(-1, q4.x());
        ASSERT_DOUBLE_EQ(-2, q4.y());
        ASSERT_DOUBLE_EQ(-3, q4.z());
        ASSERT_DOUBLE_EQ(-4, q4.w());

        q4 = q3;
        ASSERT_DOUBLE_EQ(1, q4.x());
        ASSERT_DOUBLE_EQ(2, q4.y());
        ASSERT_DOUBLE_EQ(3, q4.z());
        ASSERT_DOUBLE_EQ(4, q4.w());
    }

    {
        Quat<float> q1;
        ASSERT_FLOAT_EQ(0, q1.x());
        ASSERT_FLOAT_EQ(0, q1.y());
        ASSERT_FLOAT_EQ(0, q1.z());
        ASSERT_FLOAT_EQ(0, q1.w());

        Quat<float> q2(1, 2, 3, 4);
        ASSERT_FLOAT_EQ(1, q2.x());
        ASSERT_FLOAT_EQ(2, q2.y());
        ASSERT_FLOAT_EQ(3, q2.z());
        ASSERT_FLOAT_EQ(4, q2.w());

        Quat<float> q3(q2);
        ASSERT_FLOAT_EQ(1, q3.x());
        ASSERT_FLOAT_EQ(2, q3.y());
        ASSERT_FLOAT_EQ(3, q3.z());
        ASSERT_FLOAT_EQ(4, q3.w());

        Quat<float> q4(-1, -2, -3, -4);
        ASSERT_FLOAT_EQ(-1, q4.x());
        ASSERT_FLOAT_EQ(-2, q4.y());
        ASSERT_FLOAT_EQ(-3, q4.z());
        ASSERT_FLOAT_EQ(-4, q4.w());

        q4 = q3;
        ASSERT_FLOAT_EQ(1, q4.x());
        ASSERT_FLOAT_EQ(2, q4.y());
        ASSERT_FLOAT_EQ(3, q4.z());
        ASSERT_FLOAT_EQ(4, q4.w());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(QuatTest, Comparison)
{
    {
        Quat<double> q0;
        Quat<double> q1(1, 2, 3, 4);

        ASSERT_TRUE(q0 == q0);
        ASSERT_TRUE(q1 == q1);
        ASSERT_TRUE(q1 != q0);

        Quat<double> q2(q1);
        ASSERT_TRUE(q1 == q2);

        q2.x() = 99;
        ASSERT_FALSE(q1 == q2);
        ASSERT_TRUE(q1 != q2);

        Quat<double> qA(0, 2, 3, 4);
        Quat<double> qB(1, 0, 3, 4);
        Quat<double> qC(1, 2, 0, 4);
        Quat<double> qD(1, 2, 3, 0);
        ASSERT_FALSE(q1 == qA);
        ASSERT_FALSE(q1 == qB);
        ASSERT_FALSE(q1 == qC);
        ASSERT_FALSE(q1 == qD);
        ASSERT_TRUE(q1 != qA);
        ASSERT_TRUE(q1 != qB);
        ASSERT_TRUE(q1 != qC);
        ASSERT_TRUE(q1 != qD);
    }

    {
        Quat<float> q0;
        Quat<float> q1(1, 2, 3, 4);

        ASSERT_TRUE(q0 == q0);
        ASSERT_TRUE(q1 == q1);
        ASSERT_TRUE(q1 != q0);

        Quat<float> q2(q1);
        ASSERT_TRUE(q1 == q2);

        q2.x() = 99;
        ASSERT_FALSE(q1 == q2);
        ASSERT_TRUE(q1 != q2);

        Quat<float> qA(0, 2, 3, 4);
        Quat<float> qB(1, 0, 3, 4);
        Quat<float> qC(1, 2, 0, 4);
        Quat<float> qD(1, 2, 3, 0);
        ASSERT_FALSE(q1 == qA);
        ASSERT_FALSE(q1 == qB);
        ASSERT_FALSE(q1 == qC);
        ASSERT_FALSE(q1 == qD);
        ASSERT_TRUE(q1 != qA);
        ASSERT_TRUE(q1 != qB);
        ASSERT_TRUE(q1 != qC);
        ASSERT_TRUE(q1 != qD);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(QuatTest, GettersAndSetters)
{
    {
        Quat<double> q(1, 2, 3, 4);
        ASSERT_DOUBLE_EQ(1, q.x());
        ASSERT_DOUBLE_EQ(2, q.y());
        ASSERT_DOUBLE_EQ(3, q.z());
        ASSERT_DOUBLE_EQ(4, q.w());

        q.x() = 100;
        q.y() = 200;
        q.z() = 300;
        q.w() = 400;
        ASSERT_DOUBLE_EQ(100, q.x());
        ASSERT_DOUBLE_EQ(200, q.y());
        ASSERT_DOUBLE_EQ(300, q.z());
        ASSERT_DOUBLE_EQ(400, q.w());

        q.set(10, 20, 30, 40);
        ASSERT_DOUBLE_EQ(10, q.x());
        ASSERT_DOUBLE_EQ(20, q.y());
        ASSERT_DOUBLE_EQ(30, q.z());
        ASSERT_DOUBLE_EQ(40, q.w());
    }

    {
        Quat<float> q(1, 2, 3, 4);
        ASSERT_DOUBLE_EQ(1, q.x());
        ASSERT_DOUBLE_EQ(2, q.y());
        ASSERT_DOUBLE_EQ(3, q.z());
        ASSERT_DOUBLE_EQ(4, q.w());

        q.x() = 100;
        q.y() = 200;
        q.z() = 300;
        q.w() = 400;
        ASSERT_DOUBLE_EQ(100, q.x());
        ASSERT_DOUBLE_EQ(200, q.y());
        ASSERT_DOUBLE_EQ(300, q.z());
        ASSERT_DOUBLE_EQ(400, q.w());

        q.set(10, 20, 30, 40);
        ASSERT_DOUBLE_EQ(10, q.x());
        ASSERT_DOUBLE_EQ(20, q.y());
        ASSERT_DOUBLE_EQ(30, q.z());
        ASSERT_DOUBLE_EQ(40, q.w());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(QuatTest, Normalize)
{
    {
        const double absErr = 0.0001;

        Quat<double> q;
        Quat<double> e;

        q.set(1, 0, 0, 0);
        e.set(1, 0, 0, 0);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(0, 1, 0, 0);
        e.set(0, 1, 0, 0);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(0, 0, 1, 0);
        e.set(0, 0, 1, 0);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(0, 0, 0, 0);
        e.set(0, 0, 0, 0);
        ASSERT_FALSE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(1, 1, 1, 1);
        e.set(0.5, 0.5, 0.5, 0.5);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(1, 2, 3, 4);
        ASSERT_TRUE(q.normalize());
        ASSERT_NEAR(0.1825, q.x(), absErr);
        ASSERT_NEAR(0.3651, q.y(), absErr);
        ASSERT_NEAR(0.5477, q.z(), absErr);
        ASSERT_NEAR(0.7302, q.w(), absErr);
    }

    {
        const double absErr = 0.0001;

        Quat<float> q;
        Quat<float> e;

        q.set(1, 0, 0, 0);
        e.set(1, 0, 0, 0);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(0, 1, 0, 0);
        e.set(0, 1, 0, 0);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(0, 0, 1, 0);
        e.set(0, 0, 1, 0);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(0, 0, 0, 0);
        e.set(0, 0, 0, 0);
        ASSERT_FALSE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(1, 1, 1, 1);
        e.set(0.5, 0.5, 0.5, 0.5);
        ASSERT_TRUE(q.normalize());
        ASSERT_TRUE(e == q);

        q.set(1, 2, 3, 4);
        ASSERT_TRUE(q.normalize());
        ASSERT_NEAR(0.1825, q.x(), absErr);
        ASSERT_NEAR(0.3651, q.y(), absErr);
        ASSERT_NEAR(0.5477, q.z(), absErr);
        ASSERT_NEAR(0.7302, q.w(), absErr);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(QuatTest, AxisAngleToQuatSpecialCases)
{
    {
        Quat<double> q;
        Quat<double> e;

        q.setFromAxisAngle(Vec3d(1, 0, 0), 0);
        e.set(0, 0, 0, 1);
        ASSERT_TRUE(e == q);

        q.setFromAxisAngle(Vec3d(1, 1, 1), 0);
        e.set(0, 0, 0, 1);
        ASSERT_TRUE(e == q);
    }

    {
        Quat<float> q;
        Quat<float> e;

        q.setFromAxisAngle(Vec3f(1, 0, 0), 0);
        e.set(0, 0, 0, 1);
        ASSERT_TRUE(e == q);

        q.setFromAxisAngle(Vec3f(1, 1, 1), 0);
        e.set(0, 0, 0, 1);
        ASSERT_TRUE(e == q);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(QuatTest, FromAndtoMatrix4)
{
    const double absErr = 0.00001;
    const double absErrDeg = 0.001;

    // Input matrices and axis angles borrowed from Matrix4Test::FromRotation


    // --------------------
    {
        const double expAngleDeg = 45;
        const Vector3<double> expAxisNorm = Vector3<double>(1, 1, 1).getNormalized();
        const Matrix4<double> mi( 0.80474, -0.31062,  0.50588, 0,
                                  0.50588,  0.80474, -0.31062, 0,
                                 -0.31062,  0.50588,  0.80474, 0,
                                  0,        0,        0,       1);

        const Quat<double> qi = Quat<double>::fromAxisAngle(expAxisNorm, Math::toRadians(expAngleDeg));


        {
            const Quat<double> q = Quat<double>::fromRotationMatrix(mi);
            double angle= 0;
            Vector3<double> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);
            
            const Matrix4<double> m = qi.toMatrix4();
            ASSERT_NEAR(mi.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mi.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mi.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mi.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mi.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mi.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mi.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mi.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mi.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }

        {
            const Matrix4<float> mif(mi);
            const Quat<float> q = Quat<float>::fromRotationMatrix(mif);
            float angle= 0;
            Vector3<float> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Quat<float> qif(qi);
            const Matrix4<float> m = qif.toMatrix4();
            ASSERT_NEAR(mif.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mif.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mif.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mif.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mif.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mif.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mif.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mif.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mif.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }
    }


    // --------------------
    {
        const double expAngleDeg = 200;
        const Vector3<double> expAxisNorm = Vector3<double>(1, 1, 1).getNormalized();
        const Matrix4<double> mi(-0.29313,  0.84403,  0.44910, 0,
                                  0.44910, -0.29313,  0.84403, 0,
                                  0.84403,  0.44910, -0.29313, 0,
                                  0,        0,        0,       1);

        const Quat<double> qi = Quat<double>::fromAxisAngle(expAxisNorm, Math::toRadians(expAngleDeg));


        {
            const Quat<double> q = Quat<double>::fromRotationMatrix(mi);
            double angle= 0;
            Vector3<double> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Matrix4<double> m = qi.toMatrix4();
            ASSERT_NEAR(mi.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mi.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mi.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mi.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mi.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mi.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mi.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mi.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mi.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }

        {
            const Matrix4<float> mif(mi);
            const Quat<float> q = Quat<float>::fromRotationMatrix(mif);
            float angle= 0;
            Vector3<float> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Quat<float> qif(qi);
            const Matrix4<float> m = qif.toMatrix4();
            ASSERT_NEAR(mif.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mif.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mif.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mif.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mif.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mif.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mif.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mif.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mif.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }
    }


    // --------------------
    {
        const double expAngleDeg = 150;
        const Vector3<double> expAxisNorm = Vector3<double>(1, 2, 3).getNormalized();
        const Matrix4<double> mi(-0.73274, -0.13432,  0.66712, 0,
                                  0.66747, -0.33288,  0.66609, 0,
                                  0.13260,  0.93336,  0.33356, 0,
                                  0,        0,        0,       1);

        const Quat<double> qi = Quat<double>::fromAxisAngle(expAxisNorm, Math::toRadians(expAngleDeg));


        {
            const Quat<double> q = Quat<double>::fromRotationMatrix(mi);
            double angle= 0;
            Vector3<double> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Matrix4<double> m = qi.toMatrix4();
            ASSERT_NEAR(mi.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mi.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mi.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mi.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mi.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mi.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mi.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mi.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mi.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }

        {
            const Matrix4<float> mif(mi);
            const Quat<float> q = Quat<float>::fromRotationMatrix(mif);
            float angle= 0;
            Vector3<float> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Quat<float> qif(qi);
            const Matrix4<float> m = qif.toMatrix4();
            ASSERT_NEAR(mif.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mif.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mif.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mif.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mif.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mif.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mif.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mif.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mif.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }
    }


    // --------------------
    {
        const double expAngleDeg = 160;
        const Vector3<double> expAxisNorm = Vector3<double>(1, 0.1, 0.2).getNormalized();
        const Matrix4<double> mi( 0.90763,  0.11798,  0.40284, 0,
                                  0.25149, -0.92122, -0.29683, 0,
                                  0.33609,  0.37072, -0.86580, 0,
                                  0,        0,        0,       1);

        const Quat<double> qi = Quat<double>::fromAxisAngle(expAxisNorm, Math::toRadians(expAngleDeg));


        {
            const Quat<double> q = Quat<double>::fromRotationMatrix(mi);
            double angle= 0;
            Vector3<double> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Matrix4<double> m = qi.toMatrix4();
            ASSERT_NEAR(mi.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mi.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mi.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mi.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mi.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mi.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mi.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mi.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mi.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }

        {
            const Matrix4<float> mif(mi);
            const Quat<float> q = Quat<float>::fromRotationMatrix(mif);
            float angle= 0;
            Vector3<float> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Quat<float> qif(qi);
            const Matrix4<float> m = qif.toMatrix4();
            ASSERT_NEAR(mif.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mif.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mif.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mif.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mif.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mif.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mif.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mif.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mif.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }
    }


    // --------------------
    {
        const double expAngleDeg = 170;
        const Vector3<double> expAxisNorm = Vector3<double>(0.2, 1, 0.1).getNormalized();
        const Matrix4<double> mi(-0.90920,  0.36111,  0.20727, 0,
                                  0.39500,  0.90549,  0.15514, 0,
                                 -0.13166,  0.22292, -0.96590, 0,
                                  0,        0,        0,       1);

        const Quat<double> qi = Quat<double>::fromAxisAngle(expAxisNorm, Math::toRadians(expAngleDeg));


        {
            const Quat<double> q = Quat<double>::fromRotationMatrix(mi);
            double angle= 0;
            Vector3<double> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Matrix4<double> m = qi.toMatrix4();
            ASSERT_NEAR(mi.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mi.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mi.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mi.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mi.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mi.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mi.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mi.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mi.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }

        {
            const Matrix4<float> mif(mi);
            const Quat<float> q = Quat<float>::fromRotationMatrix(mif);
            float angle= 0;
            Vector3<float> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Quat<float> qif(qi);
            const Matrix4<float> m = qif.toMatrix4();
            ASSERT_NEAR(mif.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mif.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mif.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mif.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mif.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mif.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mif.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mif.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mif.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }
    }


    // --------------------
    {
        const double expAngleDeg = 150;
        const Vector3<double> expAxisNorm = Vector3<double>(0.1, 0.2, 1).getNormalized();
        const Matrix4<double> mi(-0.848250, -0.45241,  0.27531, 0,
                                  0.523490, -0.79494,  0.30664, 0,
                                  0.080127,  0.40423,  0.91114, 0,
                                  0,         0,        0,       1);

        const Quat<double> qi = Quat<double>::fromAxisAngle(expAxisNorm, Math::toRadians(expAngleDeg));


        {
            const Quat<double> q = Quat<double>::fromRotationMatrix(mi);
            double angle= 0;
            Vector3<double> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Matrix4<double> m = qi.toMatrix4();
            ASSERT_NEAR(mi.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mi.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mi.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mi.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mi.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mi.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mi.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mi.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mi.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }

        {
            const Matrix4<float> mif(mi);
            const Quat<float> q = Quat<float>::fromRotationMatrix(mif);
            float angle= 0;
            Vector3<float> axis(0, 0, 0);
            q.toAxisAngle(&axis, &angle);
            ASSERT_NEAR(expAxisNorm.x(), axis.x(), absErr);
            ASSERT_NEAR(expAxisNorm.y(), axis.y(), absErr);
            ASSERT_NEAR(expAxisNorm.z(), axis.z(), absErr);
            ASSERT_NEAR(expAngleDeg, Math::toDegrees(angle), absErrDeg);

            const Quat<float> qif(qi);
            const Matrix4<float> m = qif.toMatrix4();
            ASSERT_NEAR(mif.rowCol(0, 0), m.rowCol(0, 0), absErr);
            ASSERT_NEAR(mif.rowCol(0, 1), m.rowCol(0, 1), absErr);
            ASSERT_NEAR(mif.rowCol(0, 2), m.rowCol(0, 2), absErr);
            ASSERT_NEAR(mif.rowCol(1, 0), m.rowCol(1, 0), absErr);
            ASSERT_NEAR(mif.rowCol(1, 1), m.rowCol(1, 1), absErr);
            ASSERT_NEAR(mif.rowCol(1, 2), m.rowCol(1, 2), absErr);
            ASSERT_NEAR(mif.rowCol(2, 0), m.rowCol(2, 0), absErr);
            ASSERT_NEAR(mif.rowCol(2, 1), m.rowCol(2, 1), absErr);
            ASSERT_NEAR(mif.rowCol(2, 2), m.rowCol(2, 2), absErr);
        }
    }
}




//==================================================================================================
//
// Parametrized test for AxisAngle to/from Quat
//
//==================================================================================================

struct AxisAngleQuat
{
    Vec3d axis;
    double angleDeg;
    Quat<double> quat;
};

class QuatTestAxisAngle : public ::testing::TestWithParam<AxisAngleQuat>
{
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST_P(QuatTestAxisAngle, ParameterizedSetFromAxisAngle)
{
    const double absErr = 0.0001;

    const AxisAngleQuat& aaq = GetParam();
    const Quat<double>& expected = aaq.quat;

    {
        const Quat<double> q = Quat<double>::fromAxisAngle(aaq.axis, Math::toRadians(aaq.angleDeg));
        ASSERT_NEAR(expected.x(), q.x(), absErr);
        ASSERT_NEAR(expected.y(), q.y(), absErr);
        ASSERT_NEAR(expected.z(), q.z(), absErr);
        ASSERT_NEAR(expected.w(), q.w(), absErr);

        Quat<double> q2;
        q2.setFromAxisAngle(aaq.axis, Math::toRadians(aaq.angleDeg));   
        ASSERT_TRUE(q == q2);
    }

    {
        const Quat<float> q = Quat<float>::fromAxisAngle(Vec3f(aaq.axis), (float)Math::toRadians(aaq.angleDeg));
        ASSERT_NEAR(expected.x(), q.x(), absErr);
        ASSERT_NEAR(expected.y(), q.y(), absErr);
        ASSERT_NEAR(expected.z(), q.z(), absErr);
        ASSERT_NEAR(expected.w(), q.w(), absErr);

        Quat<float> q2;
        q2.setFromAxisAngle(Vec3f(aaq.axis), (float)Math::toRadians(aaq.angleDeg)); 
        ASSERT_TRUE(q == q2);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST_P(QuatTestAxisAngle, ParameterizedtoAxisAngle)
{
    const double absErr = 0.0001;
    const double absErrDeg = 0.01;

    const AxisAngleQuat& aaq = GetParam();

    double expectedAngleDeg = aaq.angleDeg;
    Vec3d expectedAxis = aaq.axis;

    // We'll never get negative values back so just flip sign of angle
    // and direction of axis vector
    if (expectedAngleDeg < 0)
    {
        expectedAngleDeg *= -1;
        expectedAxis *= -1;
    }

    expectedAxis.normalize();


    {
        Vec3d axis(0, 0, 0);
        double angle = 0;
        aaq.quat.toAxisAngle(&axis, &angle);

        ASSERT_NEAR(expectedAxis.x(),   axis.x(),   absErr);
        ASSERT_NEAR(expectedAxis.y(),   axis.y(),   absErr);
        ASSERT_NEAR(expectedAxis.z(),   axis.z(),   absErr);
        ASSERT_NEAR(expectedAngleDeg,   Math::toDegrees(angle), absErrDeg);
    }

    {
        Vec3f axis(0, 0, 0);
        float angle = 0;

        const Quat<float> quatf(aaq.quat);
        quatf.toAxisAngle(&axis, &angle);

        ASSERT_NEAR(expectedAxis.x(),   axis.x(),   absErr);
        ASSERT_NEAR(expectedAxis.y(),   axis.y(),   absErr);
        ASSERT_NEAR(expectedAxis.z(),   axis.z(),   absErr);
        ASSERT_NEAR(expectedAngleDeg,   (float)Math::toDegrees(angle),  absErrDeg);
    }
}


// C# function to generate data
// 
// static void PrintAxisAngleToQuat(Vector3D axis, double angle)
// {
//  CultureInfo ci = new CultureInfo("en-us");
// 
//  Quaternion q = new Quaternion(axis, angle);
//  String frm = "Vec3d({0:f1}, {1:f1}, {2:f1}), \t{3:f1},  \tQuat<double>({4:f4}, {5:f4}, {6:f4}, {7:f4})";
//  Debug.WriteLine(String.Format(ci, frm, axis.X, axis.Y, axis.Z, angle, q.X, q.Y, q.Z, q.W));
// }
AxisAngleQuat AAQ_ARRAY[] = 
{ 
    // Axis                 Angle   Quat
    { Vec3d(1.0, 0.0, 0.0),   45.0,   Quat<double>( 0.3827,  0.0000,  0.0000,  0.9239) },
    { Vec3d(1.0, 0.0, 0.0),   -45.0,  Quat<double>(-0.3827,  0.0000,  0.0000,  0.9239) },
    { Vec3d(1.0, 0.0, 0.0),   90.0,   Quat<double>( 0.7071,  0.0000,  0.0000,  0.7071) },
    { Vec3d(1.0, 0.0, 0.0),   -90.0,  Quat<double>(-0.7071,  0.0000,  0.0000,  0.7071) },
    { Vec3d(1.0, 0.0, 0.0),   180.0,  Quat<double>( 1.0000,  0.0000,  0.0000,  0.0000) },

    { Vec3d(0.0, 1.0, 0.0),   45.0,   Quat<double>( 0.0000,  0.3827,  0.0000,  0.9239) },
    { Vec3d(0.0, 1.0, 0.0),   90.0,   Quat<double>( 0.0000,  0.7071,  0.0000,  0.7071) },

    { Vec3d(0.0, 0.0, 1.0),   45.0,   Quat<double>( 0.0000,  0.0000,  0.3827,  0.9239) },
    { Vec3d(0.0, 0.0, 1.0),   90.0,   Quat<double>( 0.0000,  0.0000,  0.7071,  0.7071) },

    { Vec3d(1.0, 1.0, 1.0),   45.0,   Quat<double>( 0.2209,  0.2209,  0.2209,  0.9239) },
    { Vec3d(1.0, 1.0, 1.0),   -45.0,  Quat<double>(-0.2209, -0.2209, -0.2209,  0.9239) },
    { Vec3d(1.0, 1.0, 1.0),   90.0,   Quat<double>( 0.4082,  0.4082,  0.4082,  0.7071) },
    { Vec3d(1.0, 1.0, 1.0),   -90.0,  Quat<double>(-0.4082, -0.4082, -0.4082,  0.7071) },
    { Vec3d(1.0, 1.0, 1.0),   180.0,  Quat<double>( 0.5774,  0.5774,  0.5774,  0.0000) },
    
    { Vec3d(1.0, 0.0, 1.0),   45.0,   Quat<double>( 0.2706,  0.0000,  0.2706,  0.9239) },
    { Vec3d(1.0, 0.0, 1.0),   90.0,   Quat<double>( 0.5000,  0.0000,  0.5000,  0.7071) },

    { Vec3d(1.0, 1.0, 1.0),   200.0,  Quat<double>( 0.5686,  0.5686,  0.5686, -0.1736) },
    { Vec3d(1.0, 2.0, 3.0),   150.0,  Quat<double>( 0.2582,  0.5163,  0.7745,  0.2588) },
    { Vec3d(1.0, 0.1, 0.2),   160.0,  Quat<double>( 0.9611,  0.0961,  0.1922,  0.1736) },
    { Vec3d(0.2, 1.0, 0.1),   170.0,  Quat<double>( 0.1944,  0.9722,  0.0972,  0.0872) },
    { Vec3d(0.1, 0.2, 1.0),   150.0,  Quat<double>( 0.0943,  0.1885,  0.9426,  0.2588) }
};



INSTANTIATE_TEST_CASE_P(ParamInst, QuatTestAxisAngle, ::testing::ValuesIn(AAQ_ARRAY));


