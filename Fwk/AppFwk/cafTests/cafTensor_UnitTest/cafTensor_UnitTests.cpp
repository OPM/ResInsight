/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cafTensor3.h"

#include "gtest/gtest.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(cafTensor3Test, BasicTests)
{
    caf::Ten3f T1;
    caf::Ten3f T2(1, 2, 3, 4, 5, 6);
    caf::Ten3f T3(T2);

    EXPECT_EQ(1, T2[caf::Ten3f::SXX]);
    EXPECT_EQ(2, T2[caf::Ten3f::SYY]);
    EXPECT_EQ(3, T2[caf::Ten3f::SZZ]);
    EXPECT_EQ(4, T2[caf::Ten3f::SXY]);
    EXPECT_EQ(5, T2[caf::Ten3f::SYZ]);
    EXPECT_EQ(6, T2[caf::Ten3f::SZX]);

    T1 = T2;
    EXPECT_EQ(1, T1[caf::Ten3f::SXX]);
    EXPECT_EQ(2, T1[caf::Ten3f::SYY]);
    EXPECT_EQ(3, T1[caf::Ten3f::SZZ]);
    EXPECT_EQ(4, T1[caf::Ten3f::SXY]);
    EXPECT_EQ(5, T1[caf::Ten3f::SYZ]);
    EXPECT_EQ(6, T1[caf::Ten3f::SZX]);

    EXPECT_TRUE(T2 == T3);

    EXPECT_TRUE(T1 == T2);
    EXPECT_TRUE(T1.equals(T2));
    EXPECT_FALSE(T1 != T2);

    T1[caf::Ten3f::SXX] = 7;
    
    EXPECT_TRUE(T1 != T2);
    EXPECT_FALSE(T1 == T2);
    EXPECT_FALSE(T1.equals(T2));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(cafTensor3Test, setFromNativeArray)
{
    float tensData[6] = {11,12,13,14,15,16};
    caf::Ten3f T1;

    T1.setFromAbaqusLayout(tensData);
    EXPECT_EQ(11, T1[caf::Ten3f::SXX]);
    EXPECT_EQ(12, T1[caf::Ten3f::SYY]);
    EXPECT_EQ(13, T1[caf::Ten3f::SZZ]);
    EXPECT_EQ(14, T1[caf::Ten3f::SXY]);
    EXPECT_EQ(15, T1[caf::Ten3f::SZX]);
    EXPECT_EQ(16, T1[caf::Ten3f::SYZ]);

    caf::Ten3f T2;
    T2.setFromInternalLayout(tensData);
    EXPECT_EQ(11, T2[caf::Ten3f::SXX]);
    EXPECT_EQ(12, T2[caf::Ten3f::SYY]);
    EXPECT_EQ(13, T2[caf::Ten3f::SZZ]);
    EXPECT_EQ(14, T2[caf::Ten3f::SXY]);
    EXPECT_EQ(15, T2[caf::Ten3f::SYZ]);
    EXPECT_EQ(16, T2[caf::Ten3f::SZX]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(cafTensor3Test, zero)
{
    caf::Ten3f T0(0,0,0,0,0,0);

    cvf::Vec3f pDirs[3];
    cvf::Vec3f p0 = T0.calculatePrincipals(pDirs);

    EXPECT_TRUE(p0 == cvf::Vec3f::ZERO);
    EXPECT_TRUE(pDirs[0] == cvf::Vec3f::ZERO);
    EXPECT_TRUE(pDirs[1] == cvf::Vec3f::ZERO);
    EXPECT_TRUE(pDirs[2] == cvf::Vec3f::ZERO);

    float vm = T0.calculateVonMises();
    EXPECT_EQ(0.0f, vm );
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(cafTensor3Test, undef)
{
    float inf = std::numeric_limits<float>::infinity();
    caf::Ten3f T0(0,0,0,0,0,inf);

    cvf::Vec3f pDirs[3];
    cvf::Vec3f p0 = T0.calculatePrincipals(pDirs);

    EXPECT_TRUE(p0 == cvf::Vec3f(inf, inf, inf));
    EXPECT_TRUE(pDirs[0] == cvf::Vec3f::ZERO);
    EXPECT_TRUE(pDirs[1] == cvf::Vec3f::ZERO);
    EXPECT_TRUE(pDirs[2] == cvf::Vec3f::ZERO);

    float vm = T0.calculateVonMises();
    EXPECT_EQ(inf, vm );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(cafTensor3Test, realTensors1)
{
    caf::Ten3f T0(80,50,20,40,45,50);

    cvf::Vec3f pDirs[3];
    cvf::Vec3f p0 = T0.calculatePrincipals(pDirs);

    EXPECT_NEAR( 143.8f, p0[0], 0.1 );
    EXPECT_NEAR(  23.6f, p0[1], 0.1 );
    EXPECT_NEAR( -17.4f, p0[2], 0.1 );

    float vm = T0.calculateVonMises();
    EXPECT_NEAR(145.2f, vm, 0.1 );
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(cafTensor3Test, realTensors2)
{
    caf::Ten3f T0(20,50,80,50,45,40);

    cvf::Vec3f pDirs[3];
    cvf::Vec3f p0 = T0.calculatePrincipals(pDirs);

    EXPECT_NEAR( 143.8f, p0[0], 0.1 );
    EXPECT_NEAR(  23.9f, p0[1], 0.1 );
    EXPECT_NEAR( -17.6f, p0[2], 0.1 );

    float vm = T0.calculateVonMises();
    EXPECT_NEAR(145.2f, vm, 0.1 );
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(cafTensor3Test, realTensors3)
{
    caf::Ten3f T0(10,20,30,0,0,0);

    cvf::Vec3f pDirs[3];
    cvf::Vec3f p0 = T0.calculatePrincipals(pDirs);

    EXPECT_NEAR( 30.0f, p0[0], 0.1 );
    EXPECT_NEAR( 20.0f, p0[1], 0.1 );
    EXPECT_NEAR( 10.0f, p0[2], 0.1 );

    EXPECT_NEAR( 0.0f, pDirs[0][0], 0.1 );
    EXPECT_NEAR( 0.0f, pDirs[0][1], 0.1 );
    EXPECT_NEAR( 1.0f, pDirs[0][2], 0.1 );

    EXPECT_NEAR( 0.0f, pDirs[1][0], 0.1);
    EXPECT_NEAR( 1.0f, pDirs[1][1], 0.1 );
    EXPECT_NEAR( 0.0f, pDirs[1][2], 0.1 );

    EXPECT_NEAR( 1.0f, pDirs[2][0], 0.1 );
    EXPECT_NEAR( 0.0f, pDirs[2][1], 0.1 );
    EXPECT_NEAR( 0.0f, pDirs[2][2], 0.1 );


    float vm = T0.calculateVonMises();
    EXPECT_NEAR(17.3f, vm, 0.1 );
}


