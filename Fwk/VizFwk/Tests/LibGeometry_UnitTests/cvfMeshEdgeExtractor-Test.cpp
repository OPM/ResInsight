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
#include "cvfMeshEdgeExtractor.h"
#include "cvfMath.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MeshEdgeExtractorTest, EmptyOrNoInput)
{
    MeshEdgeExtractor ee;

    {
        ref<UIntArray> ind = ee.lineIndices();
        ASSERT_TRUE(ind.notNull());
        EXPECT_EQ(0, ind->size());
    }

    UIntArray emptyArr;
    ee.addPrimitives(4, emptyArr);

    {
        ref<UIntArray> ind = ee.lineIndices();
        ASSERT_TRUE(ind.notNull());
        EXPECT_EQ(0, ind->size());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MeshEdgeExtractorTest, ThreeQuadsFromUIntArray)
{
    // 3------4------5   9------8
    // |      |      |   |      | 
    // |      |      |   |      | 
    // 0------1------2   6------7

    MeshEdgeExtractor ee;

    // Two connected quads
    {
        UIntArray q;
        q.reserve(2*4);

        // Observe different winding
        q.add(0);   q.add(1);   q.add(4);   q.add(3);
        q.add(1);   q.add(4);   q.add(5);   q.add(2);

        ee.addPrimitives(4, q);
    }

    // Single loose quad
    {
        UIntArray q;
        q.reserve(4);
        q.add(6);   q.add(7);   q.add(8);   q.add(9);

        ee.addPrimitives(4, q);
    }

    ref<UIntArray> li = ee.lineIndices();
    ASSERT_EQ(2*11, li->size());

    EXPECT_EQ(0, li->get(0));    EXPECT_EQ(1, li->get(1));
    EXPECT_EQ(0, li->get(2));    EXPECT_EQ(3, li->get(3));
    EXPECT_EQ(1, li->get(4));    EXPECT_EQ(2, li->get(5));
    EXPECT_EQ(1, li->get(6));    EXPECT_EQ(4, li->get(7));
    EXPECT_EQ(2, li->get(8));    EXPECT_EQ(5, li->get(9));
    EXPECT_EQ(3, li->get(10));   EXPECT_EQ(4, li->get(11));
    EXPECT_EQ(4, li->get(12));   EXPECT_EQ(5, li->get(13));

    EXPECT_EQ(6, li->get(14));   EXPECT_EQ(7, li->get(15));
    EXPECT_EQ(6, li->get(16));   EXPECT_EQ(9, li->get(17));
    EXPECT_EQ(7, li->get(18));   EXPECT_EQ(8, li->get(19));
    EXPECT_EQ(8, li->get(20));   EXPECT_EQ(9, li->get(21));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MeshEdgeExtractorTest, PrimitiveMixFromRawArrays)
{
    //           6----5
    //          /      \      *11
    //         /        \         *12
    // 8------7          4-----9   
    // |      |\        /      |
    // |      | \      /       |
    // 0------1--2----3       10   

    const cvf::uint points[2] = { 11, 12 };
    const cvf::uint lines[4]  = { 4, 9, 10, 9 };
    const cvf::uint tri[3]    = { 1, 2, 7 };
    const cvf::uint quad[4]   = { 0, 1, 7, 8 };
    const cvf::uint poly[6]   = { 2, 3, 4, 5, 6, 7 };

    MeshEdgeExtractor ee;
    ee.addPrimitives(1, points, 2);
    ee.addPrimitives(2, lines, 4);
    ee.addPrimitives(3, tri, 3);
    ee.addPrimitives(4, quad, 4);
    ee.addPrimitives(6, poly, 6);

    UIntArray li = *ee.lineIndices();
    ASSERT_EQ(2*13, li.size());

    EXPECT_EQ(0, li[ 0]);    EXPECT_EQ(1, li[ 1]);
    EXPECT_EQ(0, li[ 2]);    EXPECT_EQ(8, li[ 3]);
    EXPECT_EQ(1, li[ 4]);    EXPECT_EQ(2, li[ 5]);
    EXPECT_EQ(1, li[ 6]);    EXPECT_EQ(7, li[ 7]);
    EXPECT_EQ(2, li[ 8]);    EXPECT_EQ(3, li[ 9]);
    EXPECT_EQ(2, li[10]);    EXPECT_EQ(7, li[11]);
    EXPECT_EQ(3, li[12]);    EXPECT_EQ(4, li[13]);
    EXPECT_EQ(4, li[14]);    EXPECT_EQ(5, li[15]);
    EXPECT_EQ(4, li[16]);    EXPECT_EQ(9, li[17]);
    EXPECT_EQ(5, li[18]);    EXPECT_EQ(6, li[19]);
    EXPECT_EQ(6, li[20]);    EXPECT_EQ(7, li[21]);
    EXPECT_EQ(7, li[22]);    EXPECT_EQ(8, li[23]);
    EXPECT_EQ(9, li[24]);    EXPECT_EQ(10,li[25]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MeshEdgeExtractorTest, PrimitiveMixFromFaceList)
{
    //           6----5
    //          /      \      *11
    //         /        \         *12
    // 8------7          4-----9   
    // |      |\        /      |
    // |      | \      /       |
    // 0------1--2----3       10   

    UIntArray fl;
    fl.reserve(26);

    fl.add(1);  fl.add(11);  
    fl.add(1);  fl.add(12);
    fl.add(2);  fl.add(4);  fl.add(9);  
    fl.add(2);  fl.add(10); fl.add(9);  
    fl.add(3);  fl.add(1);  fl.add(2);  fl.add(7); 
    fl.add(4);  fl.add(0);  fl.add(1);  fl.add(7);  fl.add(8); 
    fl.add(6);  fl.add(2);  fl.add(3);  fl.add(4);  fl.add(5);  fl.add(6);  fl.add(7); 
    ASSERT_EQ(26, fl.size());


    MeshEdgeExtractor ee;
    ee.addFaceList(fl);

    UIntArray li = *ee.lineIndices();
    ASSERT_EQ(2*13, li.size());

    EXPECT_EQ(0, li[ 0]);    EXPECT_EQ(1, li[ 1]);
    EXPECT_EQ(0, li[ 2]);    EXPECT_EQ(8, li[ 3]);
    EXPECT_EQ(1, li[ 4]);    EXPECT_EQ(2, li[ 5]);
    EXPECT_EQ(1, li[ 6]);    EXPECT_EQ(7, li[ 7]);
    EXPECT_EQ(2, li[ 8]);    EXPECT_EQ(3, li[ 9]);
    EXPECT_EQ(2, li[10]);    EXPECT_EQ(7, li[11]);
    EXPECT_EQ(3, li[12]);    EXPECT_EQ(4, li[13]);
    EXPECT_EQ(4, li[14]);    EXPECT_EQ(5, li[15]);
    EXPECT_EQ(4, li[16]);    EXPECT_EQ(9, li[17]);
    EXPECT_EQ(5, li[18]);    EXPECT_EQ(6, li[19]);
    EXPECT_EQ(6, li[20]);    EXPECT_EQ(7, li[21]);
    EXPECT_EQ(7, li[22]);    EXPECT_EQ(8, li[23]);
    EXPECT_EQ(9, li[24]);    EXPECT_EQ(10,li[25]);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MeshEdgeExtractorTest, CollapsedQuads)
{
    // 4------3        6,6
    // |      |\        |
    // |      | \       |
    // 0------1--2,2   5,5

    const cvf::uint quads[12] = { 0, 1, 3, 4,   1, 2, 2, 3,   6, 5, 5, 6 };

    MeshEdgeExtractor ee;
    ee.addPrimitives(4, quads, 12);

    UIntArray li = *ee.lineIndices();
    ASSERT_EQ(2*7, li.size());

    EXPECT_EQ(0, li[ 0]);    EXPECT_EQ(1, li[ 1]);
    EXPECT_EQ(0, li[ 2]);    EXPECT_EQ(4, li[ 3]);
    EXPECT_EQ(1, li[ 4]);    EXPECT_EQ(2, li[ 5]);
    EXPECT_EQ(1, li[ 6]);    EXPECT_EQ(3, li[ 7]);
    EXPECT_EQ(2, li[ 8]);    EXPECT_EQ(3, li[ 9]);
    EXPECT_EQ(3, li[10]);    EXPECT_EQ(4, li[11]);
    EXPECT_EQ(5, li[12]);    EXPECT_EQ(6, li[13]);
}



