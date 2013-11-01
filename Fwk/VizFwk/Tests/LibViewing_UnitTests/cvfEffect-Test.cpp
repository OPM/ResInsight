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
#include "cvfEffect.h"
#include "cvfUniform.h"
#include "cvfUniformSet.h"
#include "cvfShaderProgram.h"
#include "cvfRenderStatePolygonMode.h"
#include "cvfRenderState_FF.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EffectTest, BasicConstruction)
{
    ref<Effect> myEff = new Effect;
    ASSERT_EQ(1, myEff->refCount());

    ASSERT_EQ(NULL, myEff->shaderProgram());
    ASSERT_EQ(NULL, myEff->uniformSet());

    ASSERT_EQ(NULL, myEff->renderStateSet());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EffectTest, AddRemoveShaderProgram)
{
    ref<ShaderProgram> prog = new ShaderProgram;
    EXPECT_EQ(1, prog->refCount());

    {
        Effect eff;
        eff.setShaderProgram(prog.p());
        EXPECT_EQ(2, prog->refCount());

        eff.setShaderProgram(NULL);
        EXPECT_EQ(1, prog->refCount());
    }

    {
        Effect eff;
        eff.setShaderProgram(prog.p());
        EXPECT_EQ(2, prog->refCount());
    }

    EXPECT_EQ(1, prog->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EffectTest, SetUniform)
{
    ref<Effect> eff = new Effect;
    ASSERT_EQ(NULL, eff->uniformSet());

    ref<Uniform> u1 = new UniformFloat("u1");
    ref<Uniform> u2 = new UniformFloat("u2");
    eff->setUniform(u1.p());
    eff->setUniform(u2.p());

    const UniformSet* us = eff->uniformSet();
    ASSERT_EQ(2, us->count());
    
    EXPECT_EQ(u1.p(), us->uniform(0));
    EXPECT_EQ(u2.p(), us->uniform(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EffectTest, SetUniformsFromUniformSet)
{
    ref<Uniform> u1 = new UniformFloat("u1", 1.0);
    ref<Uniform> u2 = new UniformFloat("u2", 2.0);

    ref<UniformSet> myus = new UniformSet;
    myus->setUniform(u1.p());
    myus->setUniform(u2.p());

    ref<Effect> eff = new Effect;
    ASSERT_EQ(NULL, eff->uniformSet());

    ref<Uniform> u = new UniformFloat("u1", 99.0f);
    eff->setUniform(u.p());

    const UniformSet* effus0 = eff->uniformSet();
    ASSERT_EQ(1, effus0->count());
    EXPECT_EQ(u.p(), effus0->uniform(0));

    eff->setUniformsFromUniformSet(myus.p());

    {
        const UniformSet* effus = eff->uniformSet();
        ASSERT_EQ(effus, effus0);
        ASSERT_NE(effus, myus);
        ASSERT_EQ(2, effus->count());
        EXPECT_EQ(u1.p(), effus->uniform(0));
        EXPECT_EQ(u2.p(), effus->uniform(1));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EffectTest, AddRemoveRenderState)
{
    Effect eff;

    ref<RenderStateMaterial_FF> material1  = new RenderStateMaterial_FF;
    ref<RenderStatePolygonMode> polyMode   = new RenderStatePolygonMode;

    eff.setRenderState(material1.p());
    eff.setRenderState(polyMode.p());
    EXPECT_EQ(2, material1->refCount());
    EXPECT_EQ(2, polyMode->refCount());

    EXPECT_EQ(material1.p(), eff.renderStateOfType(RenderState::MATERIAL_FF));
    EXPECT_EQ(polyMode.p(), eff.renderStateOfType(RenderState::POLYGON_MODE));

    eff.removeRenderState(RenderState::MATERIAL_FF);
    EXPECT_EQ(1, material1->refCount());
    EXPECT_EQ(2, polyMode->refCount());

    EXPECT_EQ(NULL, eff.renderStateOfType(RenderState::MATERIAL_FF));
    EXPECT_EQ(polyMode.p(), eff.renderStateOfType(RenderState::POLYGON_MODE));
}
