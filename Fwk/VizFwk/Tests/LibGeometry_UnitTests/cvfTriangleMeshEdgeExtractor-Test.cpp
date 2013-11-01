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
#include "cvfTriangleMeshEdgeExtractor.h"
#include "cvfMath.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleMeshEdgeExtractorTest, EmptyOrNoInput)
{
    TriangleMeshEdgeExtractor ee;

    {
        ref<UIntArray> ind = ee.lineIndices();
        ASSERT_TRUE(ind.notNull());
        EXPECT_EQ(0, ind->size());
    }

    UIntArray emptyArr;
    ee.addTriangles(emptyArr, emptyArr);

    {
        ref<UIntArray> ind = ee.lineIndices();
        ASSERT_TRUE(ind.notNull());
        EXPECT_EQ(0, ind->size());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleMeshEdgeExtractorTest, ThreeQuadsFromUIntArray)
{
    // 3------4------5   9------8
    // |    / | \    |   |    / | 
    // |  /   |   \  |   | /    | 
    // 0------1------2   6------7
    //   ID=1   ID=2       ID=33
    //
    TriangleMeshEdgeExtractor ee;

    // Two connected quads
    {
        const cvf::uint conn[] = 
        { 
            0, 1, 4, 
            0, 4, 3, 
            1, 2, 4, 
            2, 5, 4, 
        };

        UIntArray indices(conn, sizeof(conn)/sizeof(cvf::uint));

        const cvf::uint aid[] = { 1, 1, 2, 2 }; 
        UIntArray ids(aid, sizeof(aid)/sizeof(cvf::uint));

        ee.addTriangles(indices, ids);
    }

    // Single loose quad
    {
        const cvf::uint conn[] = 
        { 
            6, 7, 8, 
            6, 9, 8, 
        };

        UIntArray indices(conn, sizeof(conn)/sizeof(cvf::uint));

        const cvf::uint aid[] = { 33, 33 }; 
        UIntArray ids(aid, sizeof(aid)/sizeof(cvf::uint));

        ee.addTriangles(indices, ids);
    }

    ref<UIntArray> li = ee.lineIndices();
    ASSERT_EQ(2*11, li->size());

    /*
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
    */
}

/*

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

*/
