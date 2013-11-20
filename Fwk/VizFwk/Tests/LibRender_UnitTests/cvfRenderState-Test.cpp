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
#include "cvfRenderState.h"
#include "cvfRenderStateSet.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStateColorMask.h"
#include "cvfRenderStateCullFace.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateFrontFace.h"
#include "cvfRenderStateLine.h"
#include "cvfRenderStatePoint.h"
#include "cvfRenderStatePolygonMode.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfRenderStateStencil.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfRenderState_FF.h"

#include "cvfTexture.h"
#include "cvfTextureImage.h"
#include "cvfSampler.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateSetTest, BasicLifeCycle)
{
    RenderStateSet rss;
    ASSERT_EQ(0, rss.count());

    ref<RenderStateMaterial_FF> rm1 = new RenderStateMaterial_FF;
    EXPECT_EQ(1, rm1->refCount());

    rss.setRenderState(rm1.p());
    ASSERT_EQ(1, rss.count());
    EXPECT_EQ(2, rm1->refCount());

    EXPECT_EQ(rm1.p(), rss.renderStateOfType(RenderState::MATERIAL_FF));
    EXPECT_EQ(rm1.p(), rss.renderState(0));

    ref<RenderStateMaterial_FF> rm2 = new RenderStateMaterial_FF;
    EXPECT_EQ(1, rm2->refCount());

    rss.setRenderState(rm2.p());
    ASSERT_EQ(1, rss.count());
    EXPECT_EQ(1, rm1->refCount());
    EXPECT_EQ(2, rm2->refCount());

    EXPECT_EQ(rm2.p(), rss.renderStateOfType(RenderState::MATERIAL_FF));
    EXPECT_EQ(rm2.p(), rss.renderState(0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateSetTest, AccessEmptyStateSet)
{
    RenderStateSet rss;
    ASSERT_EQ(0, rss.count());

    EXPECT_EQ(NULL, rss.renderStateOfType(RenderState::DEPTH));
    EXPECT_EQ(NULL, rss.renderStateOfType(RenderState::MATERIAL_FF));

    EXPECT_EQ(NULL, rss.renderStateOfType(RenderState::COUNT));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(RenderStateSetDeathTest, AccessEmptyStateSet)
{
    RenderStateSet rss;
    ASSERT_EQ(0, rss.count());

    EXPECT_DEATH(rss.renderState(0), "Assertion");
    EXPECT_DEATH(rss.renderState(1), "Assertion");

    rss.setRenderState(new RenderStateMaterial_FF);
    EXPECT_DEATH(rss.renderState(1), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateSetTest, AddAllStateTypes)
{
    RenderStateSet rss;
    ASSERT_EQ(0, rss.count());

    RenderStateBlending* pBlending                  = new RenderStateBlending;
    RenderStateColorMask* pColorMask                = new RenderStateColorMask;
    RenderStateCullFace* pCullFace                  = new RenderStateCullFace;
    RenderStateDepth* pDepth                        = new RenderStateDepth;
    RenderStateFrontFace* pFrontFace                = new RenderStateFrontFace;
    RenderStateLine* pLine                          = new RenderStateLine;
    RenderStatePoint* pPoint                        = new RenderStatePoint;
    RenderStatePolygonMode* pPolyMode               = new RenderStatePolygonMode;
    RenderStatePolygonOffset* pPolyOffset           = new RenderStatePolygonOffset;
    RenderStateStencil* pStencil                    = new RenderStateStencil;
    RenderStateTextureBindings* pTextureBindings    = new RenderStateTextureBindings(new Texture(Texture::TEXTURE_2D, Texture::RGBA), new Sampler, "mySampler");

    RenderStateLighting_FF* pLighting               = new RenderStateLighting_FF;
    RenderStateMaterial_FF* pMaterial               = new RenderStateMaterial_FF;
    RenderStateNormalize_FF* pNormalize             = new RenderStateNormalize_FF;
    RenderStateTextureMapping_FF* pTextureMapping   = new RenderStateTextureMapping_FF();
    RenderStateClipPlanes_FF* pClipPlanes           = new RenderStateClipPlanes_FF();

    rss.setRenderState(pBlending);
    rss.setRenderState(pColorMask);
    rss.setRenderState(pCullFace);
    rss.setRenderState(pDepth);
    rss.setRenderState(pFrontFace);
    rss.setRenderState(pLine);
    rss.setRenderState(pPoint);
    rss.setRenderState(pPolyMode);
    rss.setRenderState(pPolyOffset);
    rss.setRenderState(pStencil);
    rss.setRenderState(pTextureBindings);
    rss.setRenderState(pLighting);
    rss.setRenderState(pMaterial);
    rss.setRenderState(pNormalize);
    rss.setRenderState(pTextureMapping);
    rss.setRenderState(pClipPlanes);

    EXPECT_EQ(RenderState::COUNT, rss.count());

    EXPECT_EQ(pBlending,        rss.renderState(0));
    EXPECT_EQ(pColorMask,       rss.renderState(1));
    EXPECT_EQ(pCullFace,        rss.renderState(2));
    EXPECT_EQ(pDepth,           rss.renderState(3));
    EXPECT_EQ(pFrontFace,       rss.renderState(4));
    EXPECT_EQ(pLine,            rss.renderState(5));
    EXPECT_EQ(pPoint,           rss.renderState(6));
    EXPECT_EQ(pPolyMode,        rss.renderState(7));
    EXPECT_EQ(pPolyOffset,      rss.renderState(8));
    EXPECT_EQ(pStencil,         rss.renderState(9));
    EXPECT_EQ(pTextureBindings, rss.renderState(10));
    EXPECT_EQ(pLighting,        rss.renderState(11));
    EXPECT_EQ(pMaterial,        rss.renderState(12));
    EXPECT_EQ(pNormalize,       rss.renderState(13));
    EXPECT_EQ(pTextureMapping,  rss.renderState(14));

    EXPECT_EQ(pBlending,        rss.renderStateOfType(RenderState::BLENDING));
    EXPECT_EQ(pColorMask,       rss.renderStateOfType(RenderState::COLOR_MASK));
    EXPECT_EQ(pCullFace,        rss.renderStateOfType(RenderState::CULL_FACE));
    EXPECT_EQ(pDepth,           rss.renderStateOfType(RenderState::DEPTH));
    EXPECT_EQ(pLine,            rss.renderStateOfType(RenderState::LINE));
    EXPECT_EQ(pPoint,           rss.renderStateOfType(RenderState::POINT));
    EXPECT_EQ(pPolyMode,        rss.renderStateOfType(RenderState::POLYGON_MODE));
    EXPECT_EQ(pPolyOffset,      rss.renderStateOfType(RenderState::POLYGON_OFFSET));
    EXPECT_EQ(pStencil,         rss.renderStateOfType(RenderState::STENCIL));
    EXPECT_EQ(pTextureBindings, rss.renderStateOfType(RenderState::TEXTURE_BINDINGS));
    EXPECT_EQ(pLighting,        rss.renderStateOfType(RenderState::LIGHTING_FF));
    EXPECT_EQ(pMaterial,        rss.renderStateOfType(RenderState::MATERIAL_FF));
    EXPECT_EQ(pNormalize,       rss.renderStateOfType(RenderState::NORMALIZE_FF));
    EXPECT_EQ(pTextureMapping,  rss.renderStateOfType(RenderState::TEXTURE_MAPPING_FF));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateSetTest, AddRemoveRenderState)
{
    RenderStateSet rss;

    ref<RenderStateMaterial_FF> material1     = new RenderStateMaterial_FF;
    ref<RenderStateMaterial_FF> material2     = new RenderStateMaterial_FF;
    ref<RenderStatePolygonMode> polyMode      = new RenderStatePolygonMode;
    ref<RenderStatePolygonOffset> polyOffset  = new RenderStatePolygonOffset;

    rss.setRenderState(material1.p());
    rss.setRenderState(polyMode.p());
    rss.setRenderState(polyOffset.p());
    EXPECT_EQ(3, rss.count());
    EXPECT_EQ(2, material1->refCount());
    EXPECT_EQ(1, material2->refCount());
    EXPECT_EQ(2, polyMode->refCount());
    EXPECT_EQ(2, polyOffset->refCount());

    rss.setRenderState(material2.p());
    EXPECT_EQ(3, rss.count());
    EXPECT_EQ(1, material1->refCount());
    EXPECT_EQ(2, material2->refCount());

    rss.removeRenderState(polyMode.p());
    EXPECT_EQ(2, rss.count());
    EXPECT_EQ(1, polyMode->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, ColorMaskDefaults)
{
    RenderStateColorMask c;
    EXPECT_EQ(RenderState::COLOR_MASK, c.type());

    EXPECT_TRUE(c.isRedEnabled());
    EXPECT_TRUE(c.isGreenEnabled());
    EXPECT_TRUE(c.isBlueEnabled());
    EXPECT_TRUE(c.isAlphaEnabled());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, ColorMaskConstructor)
{
    {
        RenderStateColorMask c(false);

        EXPECT_FALSE(c.isRedEnabled());
        EXPECT_FALSE(c.isGreenEnabled());
        EXPECT_FALSE(c.isBlueEnabled());
        EXPECT_FALSE(c.isAlphaEnabled());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, ColorMaskSetGet)
{
    RenderStateColorMask c;

    c.enableWriteAllComponents(false);
    EXPECT_FALSE(c.isRedEnabled());
    EXPECT_FALSE(c.isGreenEnabled());
    EXPECT_FALSE(c.isBlueEnabled());
    EXPECT_FALSE(c.isAlphaEnabled());

    c.enable(true, false, true, false);
    EXPECT_TRUE( c.isRedEnabled());
    EXPECT_FALSE(c.isGreenEnabled());
    EXPECT_TRUE( c.isBlueEnabled());
    EXPECT_FALSE(c.isAlphaEnabled());

    c.enable(false, true, false, true);
    EXPECT_FALSE(c.isRedEnabled());
    EXPECT_TRUE( c.isGreenEnabled());
    EXPECT_FALSE(c.isBlueEnabled());
    EXPECT_TRUE( c.isAlphaEnabled());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, CullFaceDefaults)
{
    RenderStateCullFace c;
    EXPECT_EQ(RenderState::CULL_FACE, c.type());

    EXPECT_EQ(RenderStateCullFace::BACK, c.mode());
    EXPECT_TRUE(c.isEnabled());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, CullFaceSetGet)
{
    RenderStateCullFace c;
    
    c.setMode(RenderStateCullFace::FRONT);
    c.enable(false);
    EXPECT_EQ(RenderStateCullFace::FRONT, c.mode());
    EXPECT_FALSE(c.isEnabled());

    c.setMode(RenderStateCullFace::FRONT_AND_BACK);
    c.enable(true);
    EXPECT_EQ(RenderStateCullFace::FRONT_AND_BACK, c.mode());
    EXPECT_TRUE(c.isEnabled());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, FrontFaceDefaults)
{
    RenderStateFrontFace c;
    EXPECT_EQ(RenderState::FRONT_FACE, c.type());

    EXPECT_EQ(RenderStateFrontFace::CCW, c.mode());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, FrontFaceSetGet)
{
    RenderStateFrontFace c;

    c.setMode(RenderStateFrontFace::CW);
    EXPECT_EQ(RenderStateFrontFace::CW, c.mode());

    c.setMode(RenderStateFrontFace::CCW);
    EXPECT_EQ(RenderStateFrontFace::CCW, c.mode());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, DepthDefaults)
{
    RenderStateDepth d;
    EXPECT_EQ(RenderState::DEPTH, d.type());

    EXPECT_EQ(RenderStateDepth::LESS, d.function());
    EXPECT_TRUE(d.isDepthTestEnabled());
    EXPECT_TRUE(d.isDepthWriteEnabled());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, DepthSetGet)
{
    RenderStateDepth d;
    d.setFunction(RenderStateDepth::LEQUAL);
    d.enableDepthTest(false);
    d.enableDepthWrite(false);
    EXPECT_EQ(RenderStateDepth::LEQUAL, d.function());
    EXPECT_FALSE(d.isDepthTestEnabled());
    EXPECT_FALSE(d.isDepthWriteEnabled());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, LightingDefaults)
{
    RenderStateLighting_FF l;
    EXPECT_EQ(RenderState::LIGHTING_FF, l.type());

    const Color3f expectedAmbient(0.2f, 0.2f, 0.2f);

    EXPECT_TRUE(l.isEnabled());
    EXPECT_FALSE(l.isTwoSidedEnabled());
    EXPECT_FALSE(l.isLocalViewerEnabled());
    EXPECT_TRUE(l.ambientIntensity() == expectedAmbient);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, LightingSetGet)
{
    RenderStateLighting_FF l;

    l.enable(false);
    EXPECT_FALSE(l.isEnabled());

    l.enableTwoSided(true);
    l.enableLocalViewer(false);
    EXPECT_TRUE(l.isTwoSidedEnabled());
    EXPECT_FALSE(l.isLocalViewerEnabled());

    l.enableLocalViewer(true);
    l.enableTwoSided(false);
    EXPECT_TRUE(l.isLocalViewerEnabled());
    EXPECT_FALSE(l.isTwoSidedEnabled());


    const Color3f expectedAmbient(0.5f, 0.6f, 0.7f);
    l.setAmbientIntensity(expectedAmbient);
    EXPECT_TRUE(l.ambientIntensity() == expectedAmbient);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, MaterialDefaults)
{
    RenderStateMaterial_FF m;
    EXPECT_EQ(RenderState::MATERIAL_FF, m.type());

    // OpenGL defaults
    const Color3f defaultAmbient (0.2f, 0.2f, 0.2f);
    const Color3f defaultDiffuse (0.8f, 0.8f, 0.8f);
    const Color3f defaultSpecular(0.0f, 0.0f, 0.0f);
    const Color3f defaultEmission(0.0f, 0.0f, 0.0f);
    const float   defaultShininess = 0;
    const float   defaultAlpha = 1.0f;

    EXPECT_TRUE(defaultAmbient  == m.frontAmbient());
    EXPECT_TRUE(defaultDiffuse  == m.frontDiffuse());
    EXPECT_TRUE(defaultSpecular == m.frontSpecular());
    EXPECT_TRUE(defaultEmission == m.frontEmission());
    EXPECT_FLOAT_EQ(defaultShininess,   m.frontShininess());
    EXPECT_FLOAT_EQ(defaultAlpha,       m.frontAlpha());
    EXPECT_FALSE(m.isColorMaterialEnabled());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, MaterialSetGet)
{
    RenderStateMaterial_FF m;
    m.setAmbientAndDiffuse(Color3f(0.1f, 0.2f, 0.3f));
    m.setSpecular(Color3f(0.5f, 0.5f, 0.5f));
    m.setAlpha(0.6f);
    m.enableColorMaterial(true);

    const Color3f expectAmbient (0.1f, 0.2f, 0.3f);
    const Color3f expectDiffuse (0.1f, 0.2f, 0.3f);
    const Color3f expectSpecular(0.5f, 0.5f, 0.5f);
    const Color3f expectEmission(0.0f, 0.0f, 0.0f);
    const float   expectShininess = 0;
    const float   expectAlpha = 0.6f;

    EXPECT_TRUE(    expectAmbient  ==   m.frontAmbient());
    EXPECT_TRUE(    expectDiffuse  ==   m.frontDiffuse());
    EXPECT_TRUE(    expectSpecular ==   m.frontSpecular());
    EXPECT_TRUE(    expectEmission ==   m.frontEmission());
    EXPECT_FLOAT_EQ(expectShininess,    m.frontShininess());
    EXPECT_FLOAT_EQ(expectAlpha,        m.frontAlpha());
    EXPECT_TRUE(m.isColorMaterialEnabled());

    m.setDiffuse(Color3f(0.4f, 0.5f, 0.6f));

    const Color3f expectDiffuse2(0.4f, 0.5f, 0.6f);

    EXPECT_TRUE(    expectAmbient               ==  m.frontAmbient());
    EXPECT_TRUE(    Color3f(0.4f, 0.5f, 0.6f)   ==  m.frontDiffuse());
    EXPECT_TRUE(    expectSpecular              ==  m.frontSpecular());
    EXPECT_TRUE(    expectEmission              ==  m.frontEmission());
    EXPECT_FLOAT_EQ(expectShininess,                m.frontShininess());
    EXPECT_FLOAT_EQ(expectAlpha,                    m.frontAlpha());
    EXPECT_TRUE(m.isColorMaterialEnabled());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, AmbienDiffuseConstructor)
{
    RenderStateMaterial_FF m(Color3f(0.1f, 0.2f, 0.3f));

    const Color3f expectAmbient(0.1f, 0.2f, 0.3f);
    const Color3f expectDiffuse(0.1f, 0.2f, 0.3f);
    const Color3f defaultSpecular(0.0f, 0.0f, 0.0f);
    const Color3f defaultEmission(0.0f, 0.0f, 0.0f);
    const float   defaultShininess = 0;
    const float   defaultAlpha = 1.0f;

    EXPECT_TRUE(expectAmbient   == m.frontAmbient());
    EXPECT_TRUE(expectDiffuse   == m.frontDiffuse());
    EXPECT_TRUE(defaultSpecular == m.frontSpecular());
    EXPECT_TRUE(defaultEmission == m.frontEmission());
    EXPECT_FLOAT_EQ(defaultShininess,  m.frontShininess());
    EXPECT_FLOAT_EQ(defaultAlpha,      m.frontAlpha());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, ColorIdentConstructor)
{
    {
        RenderStateMaterial_FF m(RenderStateMaterial_FF::PURE_WHITE);

        const Color3f expectAmbient (1.0f, 1.0f, 1.0f);
        const Color3f expectDiffuse (1.0f, 1.0f, 1.0f);
        const Color3f expectSpecular(0.0f, 0.0f, 0.0f);
        const Color3f expectEmission(0.0f, 0.0f, 0.0f);
        const float   expectShininess = 0;
        const float   expectAlpha = 1.0f;

        EXPECT_TRUE(    expectAmbient  ==   m.frontAmbient());
        EXPECT_TRUE(    expectDiffuse  ==   m.frontDiffuse());
        EXPECT_TRUE(    expectSpecular ==   m.frontSpecular());
        EXPECT_TRUE(    expectEmission ==   m.frontEmission());
        EXPECT_FLOAT_EQ(expectShininess,    m.frontShininess());
        EXPECT_FLOAT_EQ(expectAlpha,        m.frontAlpha());
    }

    {
        RenderStateMaterial_FF m(RenderStateMaterial_FF::BRASS);

        const Color3f expectAmbient (0.329412f, 0.223529f, 0.027451f);
        const Color3f expectDiffuse (0.780392f, 0.568627f, 0.113725f);
        const Color3f expectSpecular(0.992157f, 0.941176f, 0.807843f);
        const Color3f expectEmission(0.0f, 0.0f, 0.0f);
        const float   expectShininess = 27.8974f;
        const float   expectAlpha = 1.0f;

        EXPECT_FLOAT_EQ(expectAmbient.r(), m.frontAmbient().r());
        EXPECT_FLOAT_EQ(expectAmbient.g(), m.frontAmbient().g());
        EXPECT_FLOAT_EQ(expectAmbient.b(), m.frontAmbient().b());

        EXPECT_FLOAT_EQ(expectDiffuse.r(), m.frontDiffuse().r());
        EXPECT_FLOAT_EQ(expectDiffuse.g(), m.frontDiffuse().g());
        EXPECT_FLOAT_EQ(expectDiffuse.b(), m.frontDiffuse().b());

        EXPECT_FLOAT_EQ(expectSpecular.r(), m.frontSpecular().r());
        EXPECT_FLOAT_EQ(expectSpecular.g(), m.frontSpecular().g());
        EXPECT_FLOAT_EQ(expectSpecular.b(), m.frontSpecular().b());

        EXPECT_FLOAT_EQ(expectEmission.r(), m.frontEmission().r());
        EXPECT_FLOAT_EQ(expectEmission.g(), m.frontEmission().g());
        EXPECT_FLOAT_EQ(expectEmission.b(), m.frontEmission().b());

        EXPECT_FLOAT_EQ(expectShininess, m.frontShininess());
        EXPECT_FLOAT_EQ(expectAlpha,     m.frontAlpha());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, PolygonModeDefaults)
{
    RenderStatePolygonMode p;
    EXPECT_EQ(RenderState::POLYGON_MODE, p.type());

    EXPECT_EQ(RenderStatePolygonMode::FILL, p.frontFace());
    EXPECT_EQ(RenderStatePolygonMode::FILL, p.backFace());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, PolygonModeSetGet)
{
    RenderStatePolygonMode p(RenderStatePolygonMode::POINT);
    EXPECT_EQ(RenderStatePolygonMode::POINT, p.frontFace());
    EXPECT_EQ(RenderStatePolygonMode::POINT, p.backFace());

    p.set(RenderStatePolygonMode::FILL);
    EXPECT_EQ(RenderStatePolygonMode::FILL, p.frontFace());
    EXPECT_EQ(RenderStatePolygonMode::FILL, p.backFace());

    p.setFrontFace(RenderStatePolygonMode::LINE);
    p.setBackFace(RenderStatePolygonMode::POINT);
    EXPECT_EQ(RenderStatePolygonMode::LINE, p.frontFace());
    EXPECT_EQ(RenderStatePolygonMode::POINT, p.backFace());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, PolygonOffsetDefaults)
{
    RenderStatePolygonOffset p;
    EXPECT_FALSE(p.isFillModeEnabled());
    EXPECT_FALSE(p.isLineModeEnabled());
    EXPECT_FALSE(p.isPointModeEnabled());

    EXPECT_FLOAT_EQ(0.0f, p.factor());
    EXPECT_FLOAT_EQ(0.0f, p.units());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, PolygonOffsetSetGet)
{
    RenderStatePolygonOffset p;

    p.enableFillMode(true);
    EXPECT_TRUE(p.isFillModeEnabled());
    EXPECT_FALSE(p.isLineModeEnabled());
    EXPECT_FALSE(p.isPointModeEnabled());

    p.enableLineMode(true);
    EXPECT_TRUE(p.isFillModeEnabled());
    EXPECT_TRUE(p.isLineModeEnabled());
    EXPECT_FALSE(p.isPointModeEnabled());

    p.enablePointMode(true);
    EXPECT_TRUE(p.isFillModeEnabled());
    EXPECT_TRUE(p.isLineModeEnabled());
    EXPECT_TRUE(p.isPointModeEnabled());

    p.setFactor(0.2f);
    p.setUnits(0.3f);
    EXPECT_FLOAT_EQ(0.2f, p.factor());
    EXPECT_FLOAT_EQ(0.3f, p.units());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, PolygonOffsetConfigure)
{
    {
        RenderStatePolygonOffset p;
        p.configurePolygonPositiveOffset();

        EXPECT_TRUE(p.isFillModeEnabled());
        EXPECT_FALSE(p.isLineModeEnabled());
        EXPECT_FALSE(p.isPointModeEnabled());
        EXPECT_FLOAT_EQ(1.0f, p.factor());
        EXPECT_FLOAT_EQ(1.0f, p.units());
    }

    {
        RenderStatePolygonOffset p;
        p.configureLineNegativeOffset();

        EXPECT_FALSE(p.isFillModeEnabled());
        EXPECT_TRUE(p.isLineModeEnabled());
        EXPECT_FALSE(p.isPointModeEnabled());
        EXPECT_FLOAT_EQ(-1.0f, p.factor());
        EXPECT_FLOAT_EQ(-1.0f, p.units());
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, TextureBindingsDefaults)
{
    RenderStateTextureBindings b;

    EXPECT_EQ(0, b.bindingCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, TextureBindingsSingle)
{
    ref<TextureImage> img = new TextureImage;
    ref<Texture> t = new Texture(img.p());
    ref<Sampler> s = new Sampler;
    RenderStateTextureBindings b(t.p(), s.p(), "mySampler");
    
    EXPECT_EQ(1, b.bindingCount());
    EXPECT_EQ(t.p(), b.texture(0));
    EXPECT_EQ(s.p(), b.sampler(0));
    EXPECT_STREQ("mySampler", b.samplerUniformName(0).toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderStateTest, TextureBindingsMultiple)
{
    ref<TextureImage> img = new TextureImage;
    ref<Texture> t = new Texture(img.p());
    ref<Sampler> s = new Sampler;
    RenderStateTextureBindings b(t.p(), s.p(), "mySampler");

    {
        ref<TextureImage> img = new TextureImage;
        ref<Texture> t = new Texture(img.p());
        ref<Sampler> s = new Sampler;
        b.addBinding(t.p(), s.p(), "mySampler1");
    }

    ref<TextureImage> img1 = new TextureImage;
    ref<Texture> t1 = new Texture(img1.p());
    ref<Sampler> s1 = new Sampler;
    b.addBinding(t1.p(), s1.p(), "mySampler2");

    EXPECT_EQ(3, b.bindingCount());
    EXPECT_EQ(t.p(), b.texture(0));
    EXPECT_EQ(s.p(), b.sampler(0));
    EXPECT_EQ(t1.p(), b.texture(2));
    EXPECT_EQ(s1.p(), b.sampler(2));
    EXPECT_STREQ("mySampler", b.samplerUniformName(0).toAscii().ptr());
    EXPECT_STREQ("mySampler1", b.samplerUniformName(1).toAscii().ptr());
    EXPECT_STREQ("mySampler2", b.samplerUniformName(2).toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(RenderStateSetDeathTest, TextureBindingsIllegalAccess)
{
    RenderStateTextureBindings bEmpty;

    ASSERT_EQ(0, bEmpty.bindingCount());

    EXPECT_DEATH(bEmpty.texture(0), "Assertion");
    EXPECT_DEATH(bEmpty.sampler(0), "Assertion");
    EXPECT_DEATH(bEmpty.samplerUniformName(0), "Assertion");


    ref<TextureImage> img = new TextureImage;
    ref<Texture> t = new Texture(img.p());
    ref<Sampler> s = new Sampler;
    RenderStateTextureBindings b(t.p(), s.p(), "mySampler");

    ASSERT_EQ(1, b.bindingCount());

    EXPECT_DEATH(b.texture(1), "Assertion");
    EXPECT_DEATH(b.sampler(1), "Assertion");
    EXPECT_DEATH(b.samplerUniformName(1), "Assertion");

    EXPECT_DEATH(b.texture(-1), "Assertion");
    EXPECT_DEATH(b.sampler(-1), "Assertion");
    EXPECT_DEATH(b.samplerUniformName(-1), "Assertion");
}
#endif
