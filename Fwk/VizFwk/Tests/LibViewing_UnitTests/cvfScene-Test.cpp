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
#include "cvfScene.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfGeometryUtils.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfDrawableGeo.h"


#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SceneTest, SimpleLifeCycle)
{
    ref<Model> m1 = new ModelBasicList;
    ref<Model> m2 = new ModelBasicList;
    EXPECT_EQ(1, m1->refCount());
    EXPECT_EQ(1, m2->refCount());

    {
        ref<Scene> myScene = new Scene;
        ASSERT_EQ(0, myScene->modelCount());

        myScene->addModel(m1.p());
        myScene->addModel(m2.p());
        EXPECT_EQ(2, m1->refCount());
        EXPECT_EQ(2, m2->refCount());

        ASSERT_EQ(2, myScene->modelCount());

        const Model* pm1 = myScene->model(0);
        const Model* pm2 = myScene->model(1);

        EXPECT_EQ(pm1, m1.p());
        EXPECT_EQ(pm2, m2.p());
    }

    EXPECT_EQ(1, m1->refCount());
    EXPECT_EQ(1, m2->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SceneTest, BoundingBox)
{
    GeometryBuilderFaceList builder;
    GeometryUtils::createSphere(1, 3, 4, &builder);

    {
        ref<DrawableGeo> drawable = new DrawableGeo;
        drawable->setVertexArray(builder.vertices().p());

        ref<Part> part  = new Part;
        part->setDrawable(drawable.p());

        ref<ModelBasicList> m1 = new ModelBasicList;
        m1->addPart(part.p());

        ref<Scene> myScene = new Scene;
        myScene->addModel(m1.p());

        BoundingBox bb = myScene->boundingBox();
        EXPECT_FALSE(bb.isValid());
    }
}

