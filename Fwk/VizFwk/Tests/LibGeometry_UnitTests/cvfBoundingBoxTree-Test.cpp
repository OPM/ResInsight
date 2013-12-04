//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cvfBoundingBoxTree.h"

#include "gtest/gtest.h"

using namespace cvf;

std::ostream& operator<<(std::ostream& stream, const std::vector<size_t>& array)
{
    for (size_t i = 0; i < array.size(); ++i)
    {
        stream << array[i] << " ";
    }
    stream << std::endl;
    return stream;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTree, Intersection)
{
    BoundingBoxTree bbtree;

    std::vector<cvf::BoundingBox> bbs;
    bbs.push_back(cvf::BoundingBox(Vec3d(0,0,0), Vec3d(1,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(1,0,0), Vec3d(2,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(2,0,0), Vec3d(3,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(3,0,0), Vec3d(4,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(4,0,0), Vec3d(5,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(0.5,0.5,0), Vec3d(5.5,1.5,1)));


    std::vector<size_t> ids;
    ids.push_back(10);
    ids.push_back(11);
    ids.push_back(12);
    ids.push_back(13);
    ids.push_back(14);
    ids.push_back(15);

    bbtree.buildTreeFromBoundingBoxes(bbs, &ids);

    {
        std::vector<size_t> intIds;
        bbtree.findIntersections(cvf::BoundingBox(Vec3d(0.25,0.25,0.25), Vec3d(4.5,0.4,0.4)), &intIds);
        size_t numBB = intIds.size();
        EXPECT_EQ(5, numBB);
        EXPECT_EQ(intIds[4], 13);
        //std::cout << intIds;
    }

    {
        std::vector<size_t> intIds;
        bbtree.findIntersections(cvf::BoundingBox(Vec3d(0.25,0.75,0.25), Vec3d(4.5,0.8,0.4)), &intIds);
        size_t numBB = intIds.size();
        EXPECT_EQ(6, numBB);
        EXPECT_EQ(intIds[5], 15);
        //std::cout << intIds;
    }

    {
        std::vector<size_t> intIds;
        bbtree.findIntersections(cvf::BoundingBox(Vec3d(2,0,0), Vec3d(3,1,1)), &intIds);
        size_t numBB = intIds.size();
        EXPECT_EQ(4, numBB);
        EXPECT_EQ(intIds[0], 11);
        //std::cout << intIds;
    }
}
