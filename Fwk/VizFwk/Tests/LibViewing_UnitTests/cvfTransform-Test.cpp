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
#include "cvfTransform.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TransformTest, BasicConstruction)
{
    Transform t;
    ASSERT_EQ(0, t.refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TransformTest, TreeAddRemove)
{
    ref<Transform> root = new Transform;

    ref<Transform> leaf = new Transform;

    ASSERT_EQ(0, root->childCount());
    root->addChild(leaf.p());
    ASSERT_EQ(1, root->childCount());
    root->removeAllChildren();
    ASSERT_EQ(0, root->childCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TransformTest, SimpleTreeUpdateWorldTransform)
{
    ref<Transform> root = new Transform;
    root->setLocalTransform(cvf::Mat4d::fromTranslation(cvf::Vec3d(1,0,-1)));

    ref<Transform> leaf = new Transform;
    leaf->setLocalTransform(cvf::Mat4d::fromTranslation(cvf::Vec3d(1,3,5)));

    root->addChild(leaf.p());
    root->updateWorldTransform(NULL);

    cvf::Mat4d leafWT = leaf->worldTransform();
    cvf::Vec3d origo(0,0,0);

    origo.transformPoint(leafWT);

    ASSERT_DOUBLE_EQ(2, origo.x());
    ASSERT_DOUBLE_EQ(3, origo.y());
    ASSERT_DOUBLE_EQ(4, origo.z());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TransformTest, ThreeLevelsRot)
{
    ref<Transform> root = new Transform;
    root->setLocalTransform(cvf::Mat4d::fromTranslation(cvf::Vec3d(1,0,-1)));

    ref<Transform> mid = new Transform;
    mid->setLocalTransform(cvf::Mat4d::fromTranslation(cvf::Vec3d(1,3,5)));

    ref<Transform> leaf = new Transform;
    leaf->setLocalTransform(cvf::Mat4d::fromRotation(cvf::Vec3d(1,1,1), cvf::Math::toRadians(90.0)));

    root->addChild(mid.p());
    mid->addChild(leaf.p());
    root->updateWorldTransform(NULL);

    cvf::Mat4d leafWT = leaf->worldTransform();
    cvf::Vec3d origo(0,0,0);

    origo.transformPoint(leafWT);

    ASSERT_DOUBLE_EQ(2, origo.x());
    ASSERT_DOUBLE_EQ(3, origo.y());
    ASSERT_DOUBLE_EQ(4, origo.z());
}

