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
#include "cvfHitItem.h"
#include "cvfHitDetail.h"
#include "cvfPart.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(HitItemTest, Constructor)
{
    ref<HitItem> item = new HitItem(10.0, Vec3d(1, 2, 3));

    EXPECT_EQ(10.0, item->distanceAlongRay());
    EXPECT_TRUE(item->intersectionPoint() == Vec3d(1, 2, 3));
    EXPECT_EQ(NULL, item->part());
    EXPECT_EQ(NULL, item->detail());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(HitItemTest, SettersAndGetters)
{
    ref<HitItem> item = new HitItem(1.23, Vec3d(1,2,-3));
    ref<Part> part = new Part;
    ref<HitDetail> detail = new HitDetail;
    item->setPart(part.p());
    item->setDetail(detail.p());

    EXPECT_EQ(1.23, item->distanceAlongRay());
    EXPECT_TRUE(item->intersectionPoint() == Vec3d(1,2,-3));

    EXPECT_EQ(part.p(), item->part());
    EXPECT_EQ(2, part->refCount());

    EXPECT_EQ(detail.p(), item->detail());
    EXPECT_EQ(2, detail->refCount());
}


