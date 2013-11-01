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
#include "cvfModelBasicTree.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfCamera.h"
#include "cvfPart.h"
#include "cvfViewport.h"
#include "cvfRayIntersectSpec.h"
#include "cvfDrawableGeo.h"
#include "cvfCullSettings.h"
#include "cvfRay.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicTreeTest, BasicConstruction)
{
    ref<ModelBasicTree> myModel = new ModelBasicTree;
    EXPECT_EQ(1, myModel->refCount());
    EXPECT_EQ(NULL, myModel->root());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicTreeTest, BasicLifeCycle)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;

    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;

    myModel->setRoot(root);

    EXPECT_TRUE(myModel->root() == root);

    root->addPart(p1.p());
    root->addPart(p2.p());

    ModelBasicTreeNode* n1 =  new ModelBasicTreeNode;
    root->addChild(n1);
    ref<Part> p3 = new Part;
    ref<Part> p4 = new Part;
    n1->addPart(p3.p());
    n1->addPart(p4.p());

    ModelBasicTreeNode* n2 =  new ModelBasicTreeNode;
    root->addChild(n2);
    ref<Part> p5 = new Part;
    ref<Part> p6 = new Part;
    n2->addPart(p5.p());
    n2->addPart(p6.p());

    ModelBasicTreeNode* n3 =  new ModelBasicTreeNode;
    n1->addChild(n3);
    ref<Part> p7 = new Part;
    ref<Part> p8 = new Part;
    n2->addPart(p7.p());
    n2->addPart(p8.p());

    {
        Collection<Part> allParts;
        myModel->allParts(&allParts);
        ASSERT_EQ(8u, allParts.size());
    }

    EXPECT_EQ(2, p1->refCount());
    EXPECT_EQ(2, p3->refCount());
    EXPECT_EQ(2, p5->refCount());
    EXPECT_EQ(2, p7->refCount());

    myModel->setRoot(NULL);

    EXPECT_EQ(1, p1->refCount());
    EXPECT_EQ(1, p3->refCount());
    EXPECT_EQ(1, p5->refCount());
    EXPECT_EQ(1, p7->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicTreeTest, RemovePart)
{
    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;
    myModel->setRoot(root);

    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    root->addPart(p1.p());
    root->addPart(p2.p());

    ModelBasicTreeNode* n1 =  new ModelBasicTreeNode;
    root->addChild(n1);
    ref<Part> p3 = new Part;
    n1->addPart(p3.p());

    ModelBasicTreeNode* n2 =  new ModelBasicTreeNode;
    n1->addChild(n2);
    ref<Part> p4 = new Part;
    ref<Part> p5 = new Part;
    n2->addPart(p4.p());
    n2->addPart(p5.p());

    {
        Collection<Part> parts;
        myModel->allParts(&parts);
        ASSERT_EQ(5, parts.size());
    }

    EXPECT_EQ(2, p1->refCount());
    EXPECT_EQ(2, p2->refCount());
    EXPECT_EQ(2, p3->refCount());
    EXPECT_EQ(2, p4->refCount());
    EXPECT_EQ(2, p5->refCount());


    myModel->removePart(p2.p());
    {
        Collection<Part> parts;
        myModel->allParts(&parts);
        ASSERT_EQ(4, parts.size());
    }

    EXPECT_EQ(2, p1->refCount());
    EXPECT_EQ(1, p2->refCount());
    EXPECT_EQ(2, p3->refCount());
    EXPECT_EQ(2, p4->refCount());
    EXPECT_EQ(2, p5->refCount());


    myModel->removePart(p5.p());
    {
        Collection<Part> parts;
        myModel->allParts(&parts);
        ASSERT_EQ(3, parts.size());
    }

    EXPECT_EQ(2, p1->refCount());
    EXPECT_EQ(1, p2->refCount());
    EXPECT_EQ(2, p3->refCount());
    EXPECT_EQ(2, p4->refCount());
    EXPECT_EQ(1, p5->refCount());


    myModel->removePart(p1.p());
    myModel->removePart(p3.p());
    myModel->removePart(p4.p());
    {
        Collection<Part> parts;
        myModel->allParts(&parts);
        ASSERT_EQ(0, parts.size());
    }

    EXPECT_EQ(1, p1->refCount());
    EXPECT_EQ(1, p2->refCount());
    EXPECT_EQ(1, p3->refCount());
    EXPECT_EQ(1, p4->refCount());
    EXPECT_EQ(1, p5->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicTreeTest, AppendAllParts)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;

    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;

    myModel->setRoot(root);

    EXPECT_TRUE(myModel->root() == root);

    root->addPart(p1.p());
    root->addPart(p2.p());

    ModelBasicTreeNode* n1 =  new ModelBasicTreeNode;
    root->addChild(n1);
    ref<Part> p3 = new Part;
    ref<Part> p4 = new Part;
    n1->addPart(p3.p());
    n1->addPart(p4.p());


    Collection<Part> allParts;
    myModel->allParts(&allParts);

    ASSERT_EQ(4u, allParts.size());
    EXPECT_EQ(p3.p(), allParts.at(0));
    EXPECT_EQ(p4.p(), allParts.at(1));
    EXPECT_EQ(p1.p(), allParts.at(2));
    EXPECT_EQ(p2.p(), allParts.at(3));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicTreeTest, FindVisibleParts)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;

    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;

    myModel->setRoot(root);

    EXPECT_TRUE(myModel->root() == root);

    root->addPart(p1.p());
    root->addPart(p2.p());

    ModelBasicTreeNode* n1 =  new ModelBasicTreeNode;
    root->addChild(n1);
    ref<Part> p3 = new Part;
    ref<Part> p4 = new Part;
    n1->addPart(p3.p());
    n1->addPart(p4.p());


    PartRenderHintCollection visibleParts;
    Camera camera;
    CullSettings cullSettings;

    cullSettings.enableViewFrustumCulling(false);
    cullSettings.enablePixelSizeCulling(false);

    myModel->findVisibleParts(&visibleParts, camera, cullSettings, 0xffffffff);

    ASSERT_EQ(0u, visibleParts.count());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicTreeTest, RayIntersect)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;

    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;

    myModel->setRoot(root);

    EXPECT_TRUE(myModel->root() == root);

    root->addPart(p1.p());
    root->addPart(p2.p());

    ModelBasicTreeNode* n1 =  new ModelBasicTreeNode;
    root->addChild(n1);
    ref<Part> p3 = new Part;
    ref<Part> p4 = new Part;
    n1->addPart(p3.p());
    n1->addPart(p4.p());

    Ray* ray = new Ray;
    RayIntersectSpec rayIntersectSpec(ray, NULL);
    
    // Pure coverage, no real test
    bool res = myModel->rayIntersect(rayIntersectSpec, NULL);

    EXPECT_FALSE(res);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ModelBasicTreeTest, FindParts)
{
    ref<Part> p1 = new Part(1, "Part 1");
    ref<Part> p2 = new Part(2, "Part 2");

    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;

    myModel->setRoot(root);

    EXPECT_TRUE(myModel->root() == root);

    root->addPart(p1.p());
    root->addPart(p2.p());

    ModelBasicTreeNode* n1 =  new ModelBasicTreeNode;
    root->addChild(n1);
    ref<Part> p3 = new Part;
    ref<Part> p4 = new Part;
    n1->addPart(p3.p());
    n1->addPart(p4.p());

    ModelBasicTreeNode* n2 =  new ModelBasicTreeNode;
    root->addChild(n2);
    ref<Part> p5 = new Part(5, "Part 5");
    ref<Part> p6 = new Part(6, "Part 6");
    n2->addPart(p5.p());
    n2->addPart(p6.p());

    ModelBasicTreeNode* n3 =  new ModelBasicTreeNode;
    n1->addChild(n3);
    ref<Part> p7 = new Part;
    ref<Part> p8 = new Part;
    n2->addPart(p7.p());
    n2->addPart(p8.p());

    EXPECT_TRUE(myModel->findPartByID(1) != NULL);
    EXPECT_TRUE(myModel->findPartByID(1) != NULL);
    EXPECT_TRUE(myModel->findPartByID(5) != NULL);
    EXPECT_TRUE(myModel->findPartByID(6) != NULL);
    EXPECT_TRUE(myModel->findPartByID(-1) != NULL);

    EXPECT_TRUE(myModel->findPartByID(3) == NULL);
    EXPECT_TRUE(myModel->findPartByID(999) == NULL);
    EXPECT_TRUE(myModel->findPartByID(-999) == NULL);

    EXPECT_TRUE(myModel->findPartByName("Part 1") != NULL);
    EXPECT_TRUE(myModel->findPartByName("Part 1") != NULL);
    EXPECT_TRUE(myModel->findPartByName("Part 5") != NULL);
    EXPECT_TRUE(myModel->findPartByName("Part 6") != NULL);
    EXPECT_TRUE(myModel->findPartByName("") != NULL);
    EXPECT_TRUE(myModel->findPartByName("Part 3") == NULL);
    EXPECT_TRUE(myModel->findPartByName("Not Found") == NULL);
    EXPECT_TRUE(myModel->findPartByName("test") == NULL);
}


TEST(ModelBasicTreeTest, MergeParts)
{
    // TODO
    // Must fix merging of shared geos/transforms!
    /*
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(10, 10, 1));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));
    gen.setNumDrawableGeos(10);

    Collection<Part> parts;
    gen.generateSpheres(7, 7, &parts);

    gen.setPartDistribution(Vec3i(5, 1, 1));
    gen.generateBoxes(&parts);


    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;

    size_t i = 0;
    myModel->setRoot(root);
    root->addPart(parts[i++].p());

    ModelBasicTreeNode* c12 =  new ModelBasicTreeNode;
    root->addChild(c12);
    c12->addPart(parts[i++].p());
    c12->addPart(parts[i++].p());
    c12->addPart(parts[i++].p());
    c12->addPart(parts[i++].p());

    ModelBasicTreeNode* c22 =  new ModelBasicTreeNode;
    c12->addChild(c22);
    c22->addPart(parts[i++].p());

    ModelBasicTreeNode* c11 =  new ModelBasicTreeNode;
    root->addChild(c11);
    c11->addPart(parts[i++].p());

    ModelBasicTreeNode* c21 =  new ModelBasicTreeNode;
    c11->addChild(c21);

    size_t j;
    for (j = i; j < parts.size(); j++)
    {
        c21->addPart(parts[j].p());
    }

//     ModelStatistics stat = myModel->computeStatistics();
//     stat.showLog("before merge");

    myModel->mergeParts(10, 50);
//     stat = myModel->computeStatistics();
//     stat.showLog("after merge");

    Collection<Part> partCollection;
    myModel->allParts(&partCollection);
    EXPECT_EQ(4, partCollection.size());

    Part* part = root->part(0);
    DrawableGeo* geo = dynamic_cast<DrawableGeo*>(part->drawable());
    EXPECT_EQ(49, geo->openGLPrimitiveCount());

    part = c11->part(0);
    geo = dynamic_cast<DrawableGeo*>(part->drawable());
    EXPECT_EQ(49, geo->openGLPrimitiveCount());

    part = c21->part(0);
    geo = dynamic_cast<DrawableGeo*>(part->drawable());
    EXPECT_EQ(4587, geo->openGLPrimitiveCount());

    part = c12->part(0);
    geo = dynamic_cast<DrawableGeo*>(part->drawable());
    EXPECT_EQ(245, geo->openGLPrimitiveCount());

    EXPECT_EQ(0, c12->childCount());
    */
}

