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
#include "cvfPrimitiveSetDirect.h"

#include "gtest/gtest.h"

#include "cvfOpenGL.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetDirect, BasicConstructionAndEmptyObject)
{
    ref<PrimitiveSetDirect> myPrim = new PrimitiveSetDirect(PT_POINTS);

    EXPECT_EQ(GL_POINTS, static_cast<int>(myPrim->primitiveTypeOpenGL()));
    EXPECT_EQ(0u, myPrim->indexCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(PrimitiveSetDirectDeathTest, AccessOutOfBounds)
{
    ref<PrimitiveSetDirect> myPrim = new PrimitiveSetDirect(PT_POINTS);
    EXPECT_EQ(0u, myPrim->indexCount());

    EXPECT_DEATH(myPrim->index(0), "Assertion");

    myPrim->setIndexCount(5);
    ASSERT_EQ(5, myPrim->indexCount());
    EXPECT_DEATH(myPrim->index(5), "Assertion");
}
#endif



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetDirect, SettingAndGettingIndices)
{
    ref<PrimitiveSetDirect> myPrim = new PrimitiveSetDirect(PT_POINTS);
    EXPECT_EQ(0, myPrim->indexCount());

    myPrim->setIndexCount(5);
    ASSERT_EQ(5, myPrim->indexCount());
    EXPECT_EQ(0, myPrim->index(0));
    EXPECT_EQ(4, myPrim->index(4));

    myPrim->setStartIndex(10);
    ASSERT_EQ(5, myPrim->indexCount());
    EXPECT_EQ(10, myPrim->index(0));
    EXPECT_EQ(14, myPrim->index(4));
}


