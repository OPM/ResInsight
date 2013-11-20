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
#include "cvfOutlineEdgeExtractor.h"
#include "cvfMath.h"
#include "cvfEdgeKey.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OutlineEdgeExtractorTest, Constructor)
{
    ref<Vec3fArray> va = new Vec3fArray;
    OutlineEdgeExtractor ee(0, *va);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OutlineEdgeExtractorTest, SinglePrimitives)
{
    ref<Vec3fArray> va = new Vec3fArray;
    va->reserve(4);
    va->add(Vec3f(0, 0, 0));
    va->add(Vec3f(1, 0, 0));
    va->add(Vec3f(1, 1, 0));
    va->add(Vec3f(0, 1, 0));

    // Point
    {
        const cvf::uint conn[1] = { 0 };
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(1, conn, 1);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(0, li.size());
    }

    // Line
    {
        const cvf::uint conn[2] = { 0, 1 };
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(2, conn, 2);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(2, li.size());
        ASSERT_EQ(0, li[0]); 
        ASSERT_EQ(1, li[1]);
    }

    // Tri
    {
        const cvf::uint conn[3] = { 0, 1, 2 };
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(3, conn, 3);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(6, li.size());
        ASSERT_EQ(0, li[0]); 
        ASSERT_EQ(1, li[1]);
        ASSERT_EQ(0, li[2]); 
        ASSERT_EQ(2, li[3]);
        ASSERT_EQ(1, li[4]); 
        ASSERT_EQ(2, li[5]);
    }

    // Quad
    {
        const cvf::uint conn[4] = { 0, 1, 2, 3};
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(4, conn, 4);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(8, li.size());
        ASSERT_EQ(0, li[0]); 
        ASSERT_EQ(1, li[1]);
        ASSERT_EQ(0, li[2]); 
        ASSERT_EQ(3, li[3]);
        ASSERT_EQ(1, li[4]); 
        ASSERT_EQ(2, li[5]);
        ASSERT_EQ(2, li[6]); 
        ASSERT_EQ(3, li[7]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OutlineEdgeExtractorTest, CollapsedPrimitives)
{
    ref<Vec3fArray> va = new Vec3fArray;
    va->reserve(4);
    va->add(Vec3f(0, 0, 0));
    va->add(Vec3f(1, 0, 0));
    va->add(Vec3f(1, 1, 0));

    // Collapsed tris
    {
        const cvf::uint conn[3] = { 0, 1, 0 };
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(3, conn, 3);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(2, li.size());
        EXPECT_EQ(0, li[0]); 
        EXPECT_EQ(1, li[1]);
    }

    {
        const cvf::uint conn[3] = { 0, 0, 1 };
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(3, conn, 3);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(2, li.size());
        EXPECT_EQ(0, li[0]); 
        EXPECT_EQ(1, li[1]);
    }

    {
        const cvf::uint conn[3] = { 1, 1, 1 };
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(3, conn, 3);

        UIntArray li = *ee.lineIndices();
        EXPECT_EQ(0, li.size());
    }


    // Collapsed quads
    {
        const cvf::uint conn[4] = { 0, 1, 1, 2};
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(4, conn, 4);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(6, li.size());
        EXPECT_EQ(0, li[0]); 
        EXPECT_EQ(1, li[1]);
        EXPECT_EQ(0, li[2]); 
        EXPECT_EQ(2, li[3]);
        EXPECT_EQ(1, li[4]); 
        EXPECT_EQ(2, li[5]);
    }

    {
        const cvf::uint conn[4] = { 0, 1, 2, 0};
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(4, conn, 4);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(6, li.size());
        EXPECT_EQ(0, li[0]); 
        EXPECT_EQ(1, li[1]);
        EXPECT_EQ(0, li[2]); 
        EXPECT_EQ(2, li[3]);
        EXPECT_EQ(1, li[4]); 
        EXPECT_EQ(2, li[5]);
    }

    {
        const cvf::uint conn[4] = { 0, 1, 0, 1};
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(4, conn, 4);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(2, li.size());
        EXPECT_EQ(0, li[0]); 
        EXPECT_EQ(1, li[1]);
    }

    {
        const cvf::uint conn[4] = { 1, 1, 0, 1};
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(4, conn, 4);

        UIntArray li = *ee.lineIndices();
        ASSERT_EQ(2, li.size());
        EXPECT_EQ(0, li[0]); 
        EXPECT_EQ(1, li[1]);
    }

    {
        const cvf::uint conn[4] = { 2, 2, 2, 2};
        OutlineEdgeExtractor ee(0, *va);
        ee.addPrimitives(4, conn, 4);

        UIntArray li = *ee.lineIndices();
        EXPECT_EQ(0, li.size());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OutlineEdgeExtractorTest, MultipleAdds)
{
    ref<Vec3fArray> va = new Vec3fArray;
    va->reserve(4);
    va->add(Vec3f(0, 0, 0));
    va->add(Vec3f(1, 0, 0));
    va->add(Vec3f(1, 1, 0));
    va->add(Vec3f(0, 1, 0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OutlineEdgeExtractorTest, ThreeQuads)
{
    ref<Vec3fArray> va = new Vec3fArray;
    va->reserve(8);
    va->add(Vec3f(0, 0, 0));
    va->add(Vec3f(1, 0, 0));
    va->add(Vec3f(2, 0, 0));
    va->add(Vec3f(3, 0, 0));
    va->add(Vec3f(0, 1, 0));
    va->add(Vec3f(1, 1, 0));
    va->add(Vec3f(2, 1, 0));
    va->add(Vec3f(3, 1, 0));

    // 4------5------6------7
    // |      |      |      | 
    // |      |      |      | 
    // 0------1------2------3

    OutlineEdgeExtractor ee(0, *va);

    {
        const cvf::uint conn[8] = { 0, 1, 5, 4,  1, 2, 6, 5 };
        ee.addPrimitives(4, conn, 8);
    }

    {
        const cvf::uint conn[4] = { 2, 3, 7, 6 };
        ee.addPrimitives(4, conn, 4);
    }

    UIntArray li = *ee.lineIndices();
    ASSERT_EQ(2*8, li.size());

    EXPECT_TRUE( EdgeKey(0, 1) == EdgeKey(li[ 0], li[ 1]) );
    EXPECT_TRUE( EdgeKey(0, 4) == EdgeKey(li[ 2], li[ 3]) );
    EXPECT_TRUE( EdgeKey(1, 2) == EdgeKey(li[ 4], li[ 5]) );
    EXPECT_TRUE( EdgeKey(2, 3) == EdgeKey(li[ 6], li[ 7]) );
    EXPECT_TRUE( EdgeKey(3, 7) == EdgeKey(li[ 8], li[ 9]) );
    EXPECT_TRUE( EdgeKey(4, 5) == EdgeKey(li[10], li[11]) );
    EXPECT_TRUE( EdgeKey(5, 6) == EdgeKey(li[12], li[13]) );
    EXPECT_TRUE( EdgeKey(6, 7) == EdgeKey(li[14], li[15]) );
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OutlineEdgeExtractorTest, PrimitiveMixFromFaceList)
{
    ref<Vec3fArray> va = new Vec3fArray;
    va->resize(13);
    va->set( 0, Vec3f(0, 0, 0));
    va->set( 1, Vec3f(1, 0, 0));
    va->set( 2, Vec3f(2, 0, 0));
    va->set( 3, Vec3f(3, 0, 0));
    va->set( 4, Vec3f(4, 1, 0));
    va->set( 5, Vec3f(3, 2, 0));
    va->set( 6, Vec3f(2, 2, 0));
    va->set( 7, Vec3f(1, 1, 0));
    va->set( 8, Vec3f(0, 1, 0));
    va->set( 9, Vec3f(5, 1, 0));
    va->set(10, Vec3f(5, 0, 0));
    va->set(11, Vec3f(6, 6, 0));
    va->set(12, Vec3f(7, 7, 0));

    //                 
    //                        *11
    //                            *12
    // 8------7          4-----9   
    // |      |\               |
    // |      | \              |
    // 0------1--2            10   

    UIntArray fl;
    fl.reserve(26);

    fl.add(1);  fl.add(11);  
    fl.add(1);  fl.add(12);
    fl.add(2);  fl.add(4);  fl.add(9);  
    fl.add(2);  fl.add(10); fl.add(9);  
    fl.add(3);  fl.add(1);  fl.add(2);  fl.add(7); 
    fl.add(4);  fl.add(0);  fl.add(1);  fl.add(7);  fl.add(8); 
    ASSERT_EQ(19, fl.size());

    OutlineEdgeExtractor ee(0, *va);
    ee.addFaceList(fl);

    UIntArray li = *ee.lineIndices();
    ASSERT_EQ(2*7, li.size());

    EXPECT_TRUE( EdgeKey(0, 1) == EdgeKey(li[ 0], li[ 1]) );
    EXPECT_TRUE( EdgeKey(0, 8) == EdgeKey(li[ 2], li[ 3]) );
    EXPECT_TRUE( EdgeKey(1, 2) == EdgeKey(li[ 4], li[ 5]) );
    EXPECT_TRUE( EdgeKey(2, 7) == EdgeKey(li[ 6], li[ 7]) );
    EXPECT_TRUE( EdgeKey(4, 9) == EdgeKey(li[ 8], li[ 9]) );
    EXPECT_TRUE( EdgeKey(7, 8) == EdgeKey(li[10], li[11]) );
    EXPECT_TRUE( EdgeKey(9,10) == EdgeKey(li[12], li[13]) );

    /*
    //           6----5
    //          /      \      *11
    //         /        \         *12
    // 8------7          4-----9   
    // |      |\        /      |
    // |      | \      /       |
    // 0------1--2----3       10   

    fl.add(1);  fl.add(11);  
    fl.add(1);  fl.add(12);
    fl.add(2);  fl.add(4);  fl.add(9);  
    fl.add(2);  fl.add(10); fl.add(9);  
    fl.add(3);  fl.add(1);  fl.add(2);  fl.add(7); 
    fl.add(4);  fl.add(0);  fl.add(1);  fl.add(7);  fl.add(8); 
    fl.add(6);  fl.add(2);  fl.add(3);  fl.add(4);  fl.add(5);  fl.add(6);  fl.add(7); 
    ASSERT_EQ(26, fl.size());

    OutlineEdgeExtractor ee(0, *va);
    ee.addFaceList(fl);

    UIntArray li = *ee.lineIndices();
    ASSERT_EQ(2*11, li.size());

    EXPECT_TRUE( EdgeKey(0, 1) == EdgeKey(li[ 0], li[ 1]) );
    EXPECT_TRUE( EdgeKey(0, 8) == EdgeKey(li[ 2], li[ 3]) );
    EXPECT_TRUE( EdgeKey(1, 2) == EdgeKey(li[ 4], li[ 5]) );
    EXPECT_TRUE( EdgeKey(2, 3) == EdgeKey(li[ 6], li[ 7]) );
    EXPECT_TRUE( EdgeKey(3, 4) == EdgeKey(li[ 8], li[ 9]) );
    EXPECT_TRUE( EdgeKey(4, 5) == EdgeKey(li[10], li[11]) );
    EXPECT_TRUE( EdgeKey(4, 9) == EdgeKey(li[12], li[13]) );
    EXPECT_TRUE( EdgeKey(5, 6) == EdgeKey(li[14], li[15]) );
    EXPECT_TRUE( EdgeKey(6, 7) == EdgeKey(li[16], li[17]) );
    EXPECT_TRUE( EdgeKey(7, 8) == EdgeKey(li[18], li[19]) );
    EXPECT_TRUE( EdgeKey(9,10) == EdgeKey(li[20], li[21]) );
    */
}



