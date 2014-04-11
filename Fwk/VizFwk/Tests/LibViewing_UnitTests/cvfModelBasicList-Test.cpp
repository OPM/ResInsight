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
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfDrawableGeo.h"
#include "cvfTransform.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicListTest, BasicConstruction)
{
    ref<ModelBasicList> myModel = new ModelBasicList;
    EXPECT_EQ(1, myModel->refCount());

    EXPECT_EQ(0xffffffff, myModel->partEnableMask());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicListTest, BasicLifeCycle)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    ref<Part> p3 = new Part;
    ref<Part> p4 = new Part;
    EXPECT_EQ(1, p2->refCount());

    {
        ref<ModelBasicList> myModel = new ModelBasicList;
        ASSERT_EQ(1, myModel->refCount());
        ASSERT_EQ(0u, myModel->partCount());

        myModel->addPart(p1.p());
        myModel->addPart(p2.p());
        myModel->addPart(p3.p());
        myModel->addPart(p4.p());
        EXPECT_EQ(2, p2->refCount());

        ASSERT_EQ(4, myModel->partCount());
        EXPECT_EQ(p1.p(), myModel->part(0));
        EXPECT_EQ(p2.p(), myModel->part(1));
        EXPECT_EQ(p3.p(), myModel->part(2));
        EXPECT_EQ(p4.p(), myModel->part(3));

        myModel->removePart(p2.p());
        EXPECT_EQ(1, p2->refCount());
        ASSERT_EQ(3, myModel->partCount());
        EXPECT_EQ(p4.p(), myModel->part(2));
    }

    EXPECT_EQ(1, p1->refCount());
    EXPECT_EQ(1, p2->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(ModelBasicListDeathTest, AddRemoveWithNullPointerPart)
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    EXPECT_DEATH(myModel->addPart(NULL), "Assertion");
    EXPECT_DEATH(myModel->removePart(NULL), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(ModelBasicListDeathTest, IllegalIndexing)
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    EXPECT_DEATH(myModel->part(0), "Assertion");

    ref<Part> p = new Part;
    myModel->addPart(p.p());

    EXPECT_DEATH(myModel->part(1), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicListTest, shrinkPartCount)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    ref<Part> p3 = new Part;

    {
        ref<ModelBasicList> myModel = new ModelBasicList;
        myModel->addPart(p1.p());
        myModel->addPart(p2.p());
        myModel->addPart(p3.p());
        ASSERT_EQ(3, myModel->partCount());
        EXPECT_EQ(2, p1->refCount());
        EXPECT_EQ(2, p2->refCount());
        EXPECT_EQ(2, p3->refCount());

        myModel->shrinkPartCount(3);
        ASSERT_EQ(3, myModel->partCount());
        EXPECT_EQ(2, p1->refCount());
        EXPECT_EQ(2, p2->refCount());
        EXPECT_EQ(2, p3->refCount());

        myModel->shrinkPartCount(2);
        ASSERT_EQ(2, myModel->partCount());
        EXPECT_EQ(2, p1->refCount());
        EXPECT_EQ(2, p2->refCount());
        EXPECT_EQ(1, p3->refCount());

        myModel->shrinkPartCount(0);
        ASSERT_EQ(0, myModel->partCount());
        EXPECT_EQ(1, p1->refCount());
        EXPECT_EQ(1, p2->refCount());
        EXPECT_EQ(1, p3->refCount());
    }

    EXPECT_EQ(1, p1->refCount());
    EXPECT_EQ(1, p2->refCount());
    EXPECT_EQ(1, p3->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicListTest, AllParts)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;

    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(p1.p());
    myModel->addPart(p2.p());

    Collection<Part> allParts;
    myModel->allParts(&allParts);

    ASSERT_EQ(2, allParts.size());
    EXPECT_EQ(p1.p(), allParts.at(0));
    EXPECT_EQ(p2.p(), allParts.at(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicListTest, MergeParts)
{
    Vec3fArray* verts = new Vec3fArray;
    verts->reserve(3);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(1, 0, 0));
    verts->add(Vec3f(1, 1, 0));

    Vec3fArray* norms = new Vec3fArray;
    norms->resize(3);
    norms->set(0, Vec3f::Z_AXIS);
    norms->set(1, Vec3f::Z_AXIS);
    norms->set(2, Vec3f::Z_AXIS);

    DrawableGeo* myGeo = new DrawableGeo;
    myGeo->setFromTriangleVertexArray(verts);
    myGeo->setNormalArray(norms);

    Part* myPart = new Part;
    myPart->setDrawable(myGeo);

    Part* myPart2 = new Part;
    myPart2->setDrawable(myGeo);

    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(myPart);
    myModel->addPart(myPart2);
    EXPECT_EQ(2, myModel->partCount());

    myModel->mergeParts(1000, 1000);
    EXPECT_EQ(1, myModel->partCount());
    
    Part* mergedPart = myModel->part(0);

    DrawableGeo* mergedGeo = dynamic_cast<DrawableGeo*>(mergedPart->drawable());
    const Vec3fArray* vertices = mergedGeo->vertexArray();
    EXPECT_EQ(6, vertices->size());

    Vec3f v5 = vertices->get(5);
    EXPECT_EQ(1, v5.x());
    EXPECT_EQ(1, v5.y());
    EXPECT_EQ(0, v5.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicListTest, MergePartsWithTransformation)
{
    Vec3fArray* verts = new Vec3fArray;
    verts->reserve(3);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(1, 0, 0));
    verts->add(Vec3f(1, 1, 0));

    Vec3fArray* norms = new Vec3fArray;
    norms->resize(3);
    norms->set(0, Vec3f::Z_AXIS);
    norms->set(1, Vec3f::Z_AXIS);
    norms->set(2, Vec3f::Z_AXIS);

    DrawableGeo* myGeo = new DrawableGeo;
    myGeo->setFromTriangleVertexArray(verts);
    myGeo->setNormalArray(norms);

    Part* myPart = new Part;
    myPart->setDrawable(myGeo);

    Part* myPart2 = new Part;
    myPart2->setDrawable(myGeo);

    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(myPart);
    myModel->addPart(myPart2);
    EXPECT_EQ(2, myModel->partCount());

    Mat4d matrix;
    matrix.setTranslation(Vec3d(10, 20, 30));
    Transform* transform = new Transform;
    transform->setLocalTransform(matrix);
    myPart2->setTransform(transform);
 

    myModel->mergeParts(1000, 1000);
    EXPECT_EQ(1, myModel->partCount());
    
    Part* mergedPart = myModel->part(0);
    DrawableGeo* mergedGeo = dynamic_cast<DrawableGeo*>(mergedPart->drawable());
    const Vec3fArray* vertices = mergedGeo->vertexArray();
    EXPECT_EQ(6, vertices->size());

    Vec3f v5 = vertices->get(5);
    EXPECT_EQ(11, v5.x());
    EXPECT_EQ(21, v5.y());
    EXPECT_EQ(30, v5.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicListTest, MergePartsCheckBB)
{
    Vec3fArray* verts = new Vec3fArray;
    verts->reserve(3);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(1, 0, 0));
    verts->add(Vec3f(1, 1, 0));

    Vec3fArray* norms = new Vec3fArray;
    norms->resize(3);
    norms->set(0, Vec3f::Z_AXIS);
    norms->set(1, Vec3f::Z_AXIS);
    norms->set(2, Vec3f::Z_AXIS);

    DrawableGeo* myGeo = new DrawableGeo;
    myGeo->setFromTriangleVertexArray(verts);
    myGeo->setNormalArray(norms);


    ref<ModelBasicList> myModel = new ModelBasicList;

    {
        Part* myPart = new Part;
        myPart->setDrawable(myGeo);
        Mat4d matrix;
        matrix.setTranslation(Vec3d(10, 20, 30));
        Transform* transform = new Transform;
        transform->setLocalTransform(matrix);
        myPart->setTransform(transform);
        myModel->addPart(myPart);
    }

    {
        Part* myPart2 = new Part;
        myPart2->setDrawable(myGeo);
        Mat4d matrix;
        matrix.setTranslation(Vec3d(20, 20, 30));
        Transform* transform2 = new Transform;
        transform2->setLocalTransform(matrix);
        myPart2->setTransform(transform2);
        myModel->addPart(myPart2);
    }

    {
        Part* myPart3 = new Part;
        myPart3->setDrawable(myGeo);
        Mat4d matrix;
        matrix.setTranslation(Vec3d(100, 20, 30));
        Transform* transform3 = new Transform;
        transform3->setLocalTransform(matrix);
        myPart3->setTransform(transform3);
        myModel->addPart(myPart3);
    }

    {
        Part* myPart4 = new Part;
        myPart4->setDrawable(myGeo);
        Mat4d matrix;
        matrix.setTranslation(Vec3d(110, 20, 30));
        Transform* transform4 = new Transform;
        transform4->setLocalTransform(matrix);
        myPart4->setTransform(transform4);
        myModel->addPart(myPart4);
    }

    EXPECT_EQ(4, myModel->partCount());

    myModel->mergeParts(1, 1000);
    EXPECT_EQ(4, myModel->partCount());

    myModel->mergeParts(20, 1000);
    EXPECT_EQ(2, myModel->partCount());

    myModel->mergeParts(200, 1000);
    EXPECT_EQ(1, myModel->partCount());

}
