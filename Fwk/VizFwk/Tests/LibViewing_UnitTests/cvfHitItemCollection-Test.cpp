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
#include "cvfHitItemCollection.h"
#include "cvfRay.h"
#include "cvfPart.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(HitItemCollectionTest, BasicConstructionAndEmptyCollection)
{
    HitItemCollection hic;
    EXPECT_EQ(0, hic.count());
    EXPECT_EQ(NULL, hic.firstItem());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(HitItemCollectionTest, ItemCollectionAndSort)
{
    HitItemCollection hic;

    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;

    ref<HitItem> i1 = new HitItem(1.23, Vec3d(1,1,1));
    i1->setPart(p1.p());
    hic.add(i1.p());
    EXPECT_EQ(2, i1->refCount());
    EXPECT_EQ(1, hic.count());

    ref<HitItem> i2 = new HitItem(0.5, Vec3d(2,2,2));
    i2->setPart(p2.p());
    hic.add(i2.p());

    EXPECT_EQ(2, hic.count());

    EXPECT_EQ(1.23,   hic.item(0)->distanceAlongRay());
    EXPECT_EQ(p1.p(), hic.item(0)->part());
    EXPECT_EQ(0.5,    hic.item(1)->distanceAlongRay());
    EXPECT_EQ(p2.p(), hic.item(1)->part());

    hic.sort();

    EXPECT_EQ(0.5,    hic.item(0)->distanceAlongRay());
    EXPECT_EQ(p2.p(), hic.item(0)->part());
    EXPECT_EQ(1.23,   hic.item(1)->distanceAlongRay());
    EXPECT_EQ(p1.p(), hic.item(1)->part());
}
