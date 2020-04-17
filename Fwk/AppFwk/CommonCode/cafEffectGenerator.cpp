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


#include "cafEffectGenerator.h"

#include "cafEffectCache.h"
#include "cafUtils.h"

#include "cvfMatrixState.h"
#include "cvfRenderState_FF.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStateColorMask.h"
#include "cvfRenderStateCullFace.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateLine.h"
#include "cvfRenderStatePolygonMode.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfSampler.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfTexture.h"
#include "cvfTexture2D_FF.h"
#include "cvfTextureImage.h"
#include "cvfUniform.h"

#include <QtOpenGL/QGLFormat>

namespace caf {

//#############################################################################################################################
//#############################################################################################################################
static const char checkDiscard_Transparent_Fragments_inl[] =
        "                                                                                                      \n"
        "#define CVF_CHECK_DISCARD_FRAGMENT_IMPL                                                               \n"
        "                                                                                                      \n"
        "//--------------------------------------------------------------------------------------------------  \n"
        "/// Check if fragment should be discarded based on alpha fragment value                               \n"
        "//--------------------------------------------------------------------------------------------------  \n"
        "void checkDiscardFragment()                                                                           \n"
        "{                                                                                                     \n"
        "    vec4 color = srcFragment();                                                                       \n"
        "    if (color.a < 1.0) discard;                                                                       \n"
        "}                                                                                                     \n";


//=============================================================================================================================
//=============================================================================================================================
static const char light_AmbientDiffuse_inl[] =
    "                                                                                                      \n"
    "varying vec3 v_ecPosition;                                                                            \n"
    "varying vec3 v_ecNormal;                                                                              \n"
    "uniform vec3 u_ecLightPosition;                                                                       \n"
    "                                                                                                      \n"
    "//--------------------------------------------------------------------------------------------------  \n"
    "/// lightFragment() - Simple Positional Headlight without Phong/specular component                    \n"
    "///                                                                                                   \n"
    "//--------------------------------------------------------------------------------------------------  \n"
    "vec4 lightFragment(vec4 srcFragColor, float not_in_use_shadowFactor)                                  \n"
    "{                                                                                                     \n"
    "    const float ambientIntensity = 0.2;                                                               \n"
    "                                                                                                      \n"
    "    // Light vector (from point to light source)                                                      \n"
    "    vec3 L = normalize(u_ecLightPosition - v_ecPosition);                                             \n"
    "                                                                                                      \n"
    "    // Viewing vector (from point to eye)                                                             \n"
    "    // Since we are in eye space, the eye pos is at (0, 0, 0)                                         \n"
    "    vec3 V = normalize(-v_ecPosition);                                                                \n"
    "                                                                                                      \n"
    "    vec3 N = normalize(v_ecNormal);                                                                   \n"
    "    vec3 R = normalize(reflect(-L, N));                                                               \n"
    "                                                                                                      \n"
    "    vec3 ambient = srcFragColor.rgb*ambientIntensity;                                                 \n"
    "    vec3 diffuse = srcFragColor.rgb*(1.0 - ambientIntensity)*abs(dot(N, L));                          \n"
    "                                                                                                      \n"
    "    return vec4(ambient + diffuse, srcFragColor.a);                                                   \n"
    "}                                                                                                     \n";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::String CommonShaderSources::light_AmbientDiffuse()
{
    return cvf::String(light_AmbientDiffuse_inl);
}

//--------------------------------------------------------------------------------------------------
/// Static helper to configure polygon offset render state from enum
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::RenderStatePolygonOffset> EffectGenerator::createAndConfigurePolygonOffsetRenderState(PolygonOffset polygonOffset)
{
    cvf::ref<cvf::RenderStatePolygonOffset> rs = new cvf::RenderStatePolygonOffset;
    if (polygonOffset == PO_NONE)
    {
        return rs;
    }

    rs->enableFillMode(true);
    
    switch (polygonOffset)
    {
        case PO_1:          rs->setFactor(1.0f);  rs->setUnits(1.0f); break;
        case PO_2:          rs->setFactor(2.0f);  rs->setUnits(2.0f); break;
        case PO_POS_LARGE:  rs->setFactor(3.0f);  rs->setUnits(50.0f); break;
        case PO_NEG_LARGE:  rs->setFactor(-1.0f); rs->setUnits(-30.0f); break;
        default:
            CVF_FAIL_MSG("Unhandled polygon offset enum");
    }

    return rs;
}




//==================================================================================================
//
// EffectGenerator Base class
//
//==================================================================================================

EffectGenerator::RenderingModeType EffectGenerator::sm_renderingMode = SHADER_BASED;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EffectGenerator::setRenderingMode(RenderingModeType effectType)
{
    sm_renderingMode = effectType;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EffectGenerator::RenderingModeType EffectGenerator::renderingMode()
{
    return sm_renderingMode;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> EffectGenerator::generateCachedEffect() const
{
    cvf::ref<cvf::Effect> eff = caf::EffectCache::instance()->findEffect(this);

    if (eff.notNull()) return eff.p();

    eff = generateUnCachedEffect();
    caf::EffectCache::instance()->addEffect(this, eff.p());

    return eff;
}

//--------------------------------------------------------------------------------------------------
/// Creates a new effect using the settings in the inherited generator. 
/// Creates a new effect and calls the correct update-Effect method dep. on the effect type (software/shader)
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> EffectGenerator::generateUnCachedEffect() const
{
    cvf::ref<cvf::Effect> eff = new cvf::Effect;

    if (sm_renderingMode == SHADER_BASED)
    {
        updateForShaderBasedRendering(eff.p());
    }
    else
    {
        updateForFixedFunctionRendering(eff.p());
    }

    return eff;
}



//--------------------------------------------------------------------------------------------------
/// Updates the effect to the state defined by the inherited effect generator.
/// This can be used to update an effect used several places in the scene. 
/// Will first reset the effect, and then call the correct updateEffect method implemented in the inherited generator.
//--------------------------------------------------------------------------------------------------
void EffectGenerator::updateEffect(cvf::Effect* effect) const
{
    CVF_ASSERT(effect != nullptr);

    // Clear effect

    effect->setRenderStateSet(nullptr);
    effect->setUniformSet(nullptr);
    effect->setShaderProgram(nullptr);

    if (sm_renderingMode == SHADER_BASED)
    {
        updateForShaderBasedRendering(effect);
    }
    else
    {
        updateForFixedFunctionRendering(effect);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EffectGenerator::clearEffectCache()
{
    EffectCache::instance()->clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EffectGenerator::releaseUnreferencedEffects()
{
    EffectCache::instance()->releaseUnreferencedEffects();
}

//==================================================================================================
//
// SurfaceEffectGenerator
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SurfaceEffectGenerator::SurfaceEffectGenerator(const cvf::Color4f& color, PolygonOffset polygonOffset)
{
    CVF_ASSERT(color.isValid());

    m_color = color;
    m_polygonOffset = polygonOffset;
    m_cullBackfaces = FC_NONE;
    m_enableColorMask = true;
    m_enableDepthTest = true;
    m_enableDepthWrite = true;
    m_enableLighting = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SurfaceEffectGenerator::SurfaceEffectGenerator(const cvf::Color3f& color, PolygonOffset polygonOffset)
{
    CVF_ASSERT(color.isValid());

    m_color = cvf::Color4f(color, 1.0f);
    m_polygonOffset = polygonOffset;
    m_cullBackfaces = FC_NONE;
    m_enableColorMask = true;
    m_enableDepthTest = true;
    m_enableDepthWrite = true;
    m_enableLighting = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SurfaceEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ShaderProgramGenerator gen("SurfaceEffectGenerator", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);

    if (m_enableLighting)
    {
        gen.addFragmentCode(CommonShaderSources::light_AmbientDiffuse());
        gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);
    }
    else
    {
        gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    }

    cvf::ref<cvf::ShaderProgram> shaderProg = gen.generate();
    if (m_enableLighting) shaderProg->setDefaultUniform(new cvf::UniformFloat("u_ecLightPosition", cvf::Vec3f(0.5, 5.0, 7.0)));

    cvf::ref<cvf::Effect> eff = effect;
    eff->setShaderProgram(shaderProg.p());
    eff->setUniform(new cvf::UniformFloat("u_color", m_color));

 
    this->updateCommonEffect(effect);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SurfaceEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    cvf::ref<cvf::RenderStateMaterial_FF> mat = new cvf::RenderStateMaterial_FF(m_color.toColor3f());
    mat->setAlpha(m_color.a());

    eff->setRenderState(mat.p());

    cvf::ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
    lighting->enableTwoSided(true);
    lighting->enable(m_enableLighting);
    eff->setRenderState(lighting.p());

    this->updateCommonEffect(effect);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SurfaceEffectGenerator::updateCommonEffect(cvf::Effect* effect) const
{
    if (m_polygonOffset != PO_NONE)
    {
        cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = EffectGenerator::createAndConfigurePolygonOffsetRenderState(m_polygonOffset);
        effect->setRenderState(polyOffset.p());
    }

    // Simple transparency
    if (m_color.a() < 1.0f)
    {
        cvf::ref<cvf::RenderStateBlending> blender = new cvf::RenderStateBlending;
        blender->configureTransparencyBlending();
        effect->setRenderState(blender.p());
    }

    // Face culling
    if (m_cullBackfaces != FC_NONE)
    {
        cvf::ref<cvf::RenderStateCullFace> faceCulling = new cvf::RenderStateCullFace;
        if (m_cullBackfaces == FC_BACK)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::BACK);
        }
        else if (m_cullBackfaces == FC_FRONT)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::FRONT);
        }
        else if (m_cullBackfaces == FC_FRONT_AND_BACK)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::FRONT_AND_BACK);
        }

        effect->setRenderState(faceCulling.p());
    }

    if (!m_enableColorMask)
    {
        cvf::ref<cvf::RenderStateColorMask> color = new cvf::RenderStateColorMask(m_enableColorMask);
        effect->setRenderState(color.p());
    }

    if (!m_enableDepthTest || !m_enableDepthWrite)
    {
        cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
        depth->enableDepthTest(m_enableDepthTest);
        depth->enableDepthWrite(m_enableDepthWrite);
        effect->setRenderState(depth.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool SurfaceEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const SurfaceEffectGenerator* otherSurfaceEffect = dynamic_cast<const SurfaceEffectGenerator*>(other);

    if (otherSurfaceEffect)
    {
        if (m_color == otherSurfaceEffect->m_color
            && m_polygonOffset == otherSurfaceEffect->m_polygonOffset
            && m_cullBackfaces == otherSurfaceEffect->m_cullBackfaces
            && m_enableColorMask == otherSurfaceEffect->m_enableColorMask
            && m_enableDepthTest == otherSurfaceEffect->m_enableDepthTest
            && m_enableDepthWrite == otherSurfaceEffect->m_enableDepthWrite
            && m_enableLighting == otherSurfaceEffect->m_enableLighting)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EffectGenerator* SurfaceEffectGenerator::copy() const
{
    SurfaceEffectGenerator* effGen = new SurfaceEffectGenerator(m_color, m_polygonOffset);
    effGen->m_cullBackfaces = m_cullBackfaces;
    effGen->m_enableColorMask = m_enableColorMask;
    effGen->m_enableDepthTest = m_enableDepthTest;
    effGen->m_enableDepthWrite = m_enableDepthWrite;
    effGen->m_enableLighting = m_enableLighting;
    return effGen;
}




//==================================================================================================
//
// ScalarMapperEffectGenerator
//
//==================================================================================================



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ScalarMapperEffectGenerator::ScalarMapperEffectGenerator(const cvf::ScalarMapper* scalarMapper, PolygonOffset polygonOffset)
    : m_undefinedColor(cvf::Color3::GRAY)
{
    m_scalarMapper = scalarMapper;
    m_polygonOffset = polygonOffset;
    m_opacityLevel = 1.0f;
    m_faceCulling = FC_NONE;
    m_enableDepthWrite = true;
    m_disableLighting = false;
    m_discardTransparentFragments = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    cvf::ShaderProgramGenerator gen("ScalarMapperEffectGenerator", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Texture);

    if (m_discardTransparentFragments)
    {
        gen.addFragmentCode(checkDiscard_Transparent_Fragments_inl);
    }
    
    if (m_disableLighting)
    {
        gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    }
    else
    {
        gen.addFragmentCode(CommonShaderSources::light_AmbientDiffuse());
        gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);
    }


    cvf::ref<cvf::ShaderProgram> prog = gen.generate();
    eff->setShaderProgram(prog.p());

    if(!m_disableLighting) prog->setDefaultUniform(new cvf::UniformFloat("u_ecLightPosition", cvf::Vec3f(0.5, 5.0, 7.0)));

    // Result mapping texture

    m_textureImage = new cvf::TextureImage();
    m_scalarMapper->updateTexture(m_textureImage.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(m_textureImage.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture> texture = new cvf::Texture(modTexImg.p());
    cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    cvf::ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(texture.p(), sampler.p(), "u_texture2D");
    eff->setRenderState(texBind.p());

    // Hardware independent:
    
    updateCommonEffect(eff.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    cvf::ref<cvf::RenderStateMaterial_FF> mat = new cvf::RenderStateMaterial_FF(cvf::Color3::WHITE);
    eff->setRenderState(mat.p());

    cvf::ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
    lighting->enableTwoSided(true);
    lighting->enable(!m_disableLighting);
    eff->setRenderState(lighting.p());

    // Result mapping texture

    m_textureImage = new cvf::TextureImage;
    m_scalarMapper->updateTexture(m_textureImage.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(m_textureImage.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture2D_FF> texture = new cvf::Texture2D_FF(modTexImg.p());
    texture->setWrapMode(cvf::Texture2D_FF::CLAMP);
    texture->setMinFilter(cvf::Texture2D_FF::NEAREST);
    texture->setMagFilter(cvf::Texture2D_FF::NEAREST);
    cvf::ref<cvf::RenderStateTextureMapping_FF> texMapping = new cvf::RenderStateTextureMapping_FF(texture.p());
    eff->setRenderState(texMapping.p());

    // Hardware independent:

    updateCommonEffect(eff.p());

}

//--------------------------------------------------------------------------------------------------
/// It also modifies the texture, and adds two more pixel lines
/// one with a transparent version of the legend color, and one with color for undefined values
//--------------------------------------------------------------------------------------------------
void ScalarMapperEffectGenerator::updateCommonEffect(cvf::Effect* effect) const
{
    CVF_ASSERT(effect);

    if (m_polygonOffset != PO_NONE)
    {
        cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = EffectGenerator::createAndConfigurePolygonOffsetRenderState(m_polygonOffset);
        effect->setRenderState(polyOffset.p());
    }

    // Simple transparency
    if (m_opacityLevel < 1.0f)
    {
        cvf::ref<cvf::RenderStateBlending> blender = new cvf::RenderStateBlending;
        blender->configureTransparencyBlending();
        effect->setRenderState(blender.p());
    }

    // Backface culling
    if (m_faceCulling != FC_NONE)
    {
        cvf::ref<cvf::RenderStateCullFace> faceCulling = new cvf::RenderStateCullFace;
        if (m_faceCulling == FC_BACK)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::BACK);
        }
        else if (m_faceCulling == FC_FRONT)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::FRONT);
        }
        else if (m_faceCulling == FC_FRONT_AND_BACK)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::FRONT_AND_BACK);
        }

        effect->setRenderState(faceCulling.p());
    }

    if (!m_enableDepthWrite)
    {
        cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
        depth->enableDepthWrite(false);
        effect->setRenderState(depth.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapperEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const ScalarMapperEffectGenerator* otherTextureResultEffect = dynamic_cast<const ScalarMapperEffectGenerator*>(other);

    if (otherTextureResultEffect)
    {
        if (m_scalarMapper.p() == otherTextureResultEffect->m_scalarMapper 
            && m_polygonOffset == otherTextureResultEffect->m_polygonOffset
            && m_opacityLevel == otherTextureResultEffect->m_opacityLevel
            && m_undefinedColor == otherTextureResultEffect->m_undefinedColor
            && m_faceCulling == otherTextureResultEffect->m_faceCulling
            && m_enableDepthWrite == otherTextureResultEffect->m_enableDepthWrite
            && m_disableLighting == otherTextureResultEffect->m_disableLighting
            && m_discardTransparentFragments == otherTextureResultEffect->m_discardTransparentFragments)
        {
            cvf::ref<cvf::TextureImage> texImg2 = new cvf::TextureImage;
            otherTextureResultEffect->m_scalarMapper->updateTexture(texImg2.p());

            return ScalarMapperEffectGenerator::isImagesEqual(m_textureImage.p(), texImg2.p());
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EffectGenerator* ScalarMapperEffectGenerator::copy() const
{
    ScalarMapperEffectGenerator* scEffGen = new ScalarMapperEffectGenerator(m_scalarMapper.p(), m_polygonOffset);
    scEffGen->m_textureImage = m_textureImage;
    scEffGen->m_opacityLevel = m_opacityLevel;
    scEffGen->m_undefinedColor = m_undefinedColor;
    scEffGen->m_faceCulling = m_faceCulling;
    scEffGen->m_enableDepthWrite = m_enableDepthWrite;
    scEffGen->m_disableLighting = m_disableLighting;
    scEffGen->m_discardTransparentFragments = m_discardTransparentFragments;

    return scEffGen;
}

//--------------------------------------------------------------------------------------------------
/// Modifies the supplied one line texture by adding two more pixel lines
/// one with a transparent version of the legend color, and one with color for undefined values
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> 
ScalarMapperEffectGenerator::addAlphaAndUndefStripes(const cvf::TextureImage* texImg, const cvf::Color3f& undefScalarColor, float opacityLevel)
{
    CVF_ASSERT(texImg != nullptr);
    CVF_ASSERT(texImg->height() == 1);

    cvf::ref<cvf::TextureImage> modTexImg = new cvf::TextureImage;
    modTexImg->allocate(texImg->width(), texImg->height() + 3); // Make the texture a power of two to avoid behind the scenes scaling and the following artefacts
    modTexImg->fill(cvf::Color4ub(cvf::Color3ub(undefScalarColor), 255)); // Undefined color

    for (cvf::uint i = 0 ; i < texImg->width(); ++i)
    {
        cvf::Color4ub legendColor = texImg->pixel(i, 0);
        modTexImg->setPixel(i, 0, legendColor);
        legendColor.a() = static_cast<cvf::ubyte>(opacityLevel * 255);
        modTexImg->setPixel(i, 1, legendColor);
        modTexImg->setPixel(i, 2, legendColor);
    }

    return modTexImg;
}

//--------------------------------------------------------------------------------------------------
/// Tests whether two texture images are equal. It might in some rare cases not detect the difference
/// but to make the comparison fast only some sampling points are used. If both pointers are NULL,
/// they are considered equal.
//--------------------------------------------------------------------------------------------------
bool ScalarMapperEffectGenerator::isImagesEqual(const cvf::TextureImage* texImg1, const cvf::TextureImage* texImg2)
{
    if (texImg1 == nullptr && texImg2 == nullptr ) return true;

    if (   texImg1 != nullptr && texImg2 != nullptr 
        && texImg1->height() == texImg2->height() 
        && texImg1->width()  == texImg2->width()
        && texImg1->width() > 0 && texImg1->height() > 0
        && texImg1->pixel(0,0) == texImg2->pixel(0,0)
        && texImg1->pixel( texImg1->width()-1, texImg1->height()-1) == texImg2->pixel(texImg1->width()-1, texImg1->height()-1)
        && texImg1->pixel( texImg1->width()/2, texImg1->height()/2) == texImg2->pixel(texImg1->width()/2, texImg1->height()/2)
        && texImg1->pixel( texImg1->width()/4, texImg1->height()/4) == texImg2->pixel(texImg1->width()/4, texImg1->height()/4)
        && texImg1->pixel( texImg1->width()/2 + texImg1->width()/4, texImg1->height()/2 + texImg1->height()/4) == texImg2->pixel(texImg1->width()/2 + texImg1->width()/4, texImg1->height()/2 + texImg1->height()/4 )
        )
    {
        return true;
    }

    return false;
}



//==================================================================================================
//
// ScalarMapperMeshEffectGenerator
//
//==================================================================================================



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ScalarMapperMeshEffectGenerator::ScalarMapperMeshEffectGenerator(const cvf::ScalarMapper* scalarMapper)
    : m_undefinedColor(cvf::Color3::GRAY)
{
    m_scalarMapper = scalarMapper;
    m_opacityLevel = 1.0f;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperMeshEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    cvf::ShaderProgramGenerator gen("ScalarMapperMeshEffectGenerator", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Texture);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);

    cvf::ref<cvf::ShaderProgram> prog = gen.generate();
    eff->setShaderProgram(prog.p());

    // Result mapping texture

    m_textureImage = new cvf::TextureImage;
    m_scalarMapper->updateTexture(m_textureImage.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(m_textureImage.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture> texture = new cvf::Texture(modTexImg.p());
    cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    cvf::ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(texture.p(), sampler.p(), "u_texture2D");
    eff->setRenderState(texBind.p());

    // Hardware independent:

    updateCommonEffect(eff.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperMeshEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::WHITE));
    eff->setRenderState(new cvf::RenderStatePolygonMode(cvf::RenderStatePolygonMode::LINE));
    eff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));
    eff->setRenderState(new cvf::RenderStateLighting_FF(false));

    // Result mapping texture

    m_textureImage = new cvf::TextureImage;
    m_scalarMapper->updateTexture(m_textureImage.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(m_textureImage.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture2D_FF> texture = new cvf::Texture2D_FF(modTexImg.p());
    texture->setWrapMode(cvf::Texture2D_FF::CLAMP);
    texture->setMinFilter(cvf::Texture2D_FF::NEAREST);
    texture->setMagFilter(cvf::Texture2D_FF::NEAREST);

    cvf::ref<cvf::RenderStateTextureMapping_FF> texMapping = new cvf::RenderStateTextureMapping_FF(texture.p());
    texMapping->setTextureFunction(cvf::RenderStateTextureMapping_FF::DECAL);
    eff->setRenderState(texMapping.p());

    // Hardware independent:

    updateCommonEffect(eff.p());

}

//--------------------------------------------------------------------------------------------------
/// It also modifies the texture, and adds two more pixel lines
/// one with a transparent version of the legend color, and one with color for undefined values
//--------------------------------------------------------------------------------------------------
void ScalarMapperMeshEffectGenerator::updateCommonEffect(cvf::Effect* effect) const
{
    CVF_ASSERT(effect);

    // Simple transparency
    if (m_opacityLevel < 1.0f)
    {
        cvf::ref<cvf::RenderStateBlending> blender = new cvf::RenderStateBlending;
        blender->configureTransparencyBlending();
        effect->setRenderState(blender.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapperMeshEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const ScalarMapperMeshEffectGenerator* otherTextureResultEffect = dynamic_cast<const ScalarMapperMeshEffectGenerator*>(other);

    if (otherTextureResultEffect)
    {
        if (m_scalarMapper.p() == otherTextureResultEffect->m_scalarMapper 
            && m_opacityLevel == otherTextureResultEffect->m_opacityLevel
            && m_undefinedColor == otherTextureResultEffect->m_undefinedColor)
        {
            cvf::ref<cvf::TextureImage> texImg2 = new cvf::TextureImage;
            otherTextureResultEffect->m_scalarMapper->updateTexture(texImg2.p());

            return ScalarMapperEffectGenerator::isImagesEqual(m_textureImage.p(), texImg2.p());
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EffectGenerator* ScalarMapperMeshEffectGenerator::copy() const
{
    ScalarMapperMeshEffectGenerator* scEffGen = new ScalarMapperMeshEffectGenerator(m_scalarMapper.p());
    scEffGen->m_textureImage = m_textureImage;
    scEffGen->m_opacityLevel = m_opacityLevel;
    scEffGen->m_undefinedColor = m_undefinedColor;

    return scEffGen;
}


//==================================================================================================
//
// MeshEffectGenerator
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MeshEffectGenerator::MeshEffectGenerator(const cvf::Color3f& color)
{
    m_color = color;
    m_lineStipple = false;
    m_lineWidth = cvf::UNDEFINED_FLOAT;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MeshEffectGenerator::setLineWidth(float lineWidth)
{
    m_lineWidth = lineWidth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MeshEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ShaderProgramGenerator spGen("Mesh", cvf::ShaderSourceProvider::instance());
    spGen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    spGen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);
    spGen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);

    cvf::ref<cvf::ShaderProgram> shaderProg = spGen.generate();

    cvf::ref<cvf::Effect> eff = effect;
    eff->setShaderProgram(shaderProg.p());
    eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(m_color, 1.0)));
    
    if (m_lineStipple)
    {
        // TODO: Use when VizFwk is updated
        //eff->setRenderState(new cvf::RenderStateLineStipple_FF);
    }

    if (m_lineWidth < cvf::UNDEFINED_FLOAT)
    {
        eff->setRenderState(new cvf::RenderStateLine(m_lineWidth));
    }

    eff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MeshEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    eff->setRenderState(new cvf::RenderStateMaterial_FF(m_color));
    eff->setRenderState(new cvf::RenderStatePolygonMode(cvf::RenderStatePolygonMode::LINE));
    eff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));
    eff->setRenderState(new cvf::RenderStateLighting_FF(false));

    if (m_lineStipple)
    {
        // TODO: Use when VizFwk is updated
        //eff->setRenderState(new cvf::RenderStateLineStipple_FF);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool MeshEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const MeshEffectGenerator* otherMesh = dynamic_cast<const MeshEffectGenerator*>(other);

    if (otherMesh)
    {
        if (m_color != otherMesh->m_color)
        {
            return false;
        }

        if (m_lineStipple != otherMesh->m_lineStipple)
        {
            return false;
        }

        if (m_lineWidth != otherMesh->m_lineWidth)
        {
            return false;
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EffectGenerator* MeshEffectGenerator::copy() const
{
    MeshEffectGenerator* effGen = new MeshEffectGenerator(m_color);
    effGen->setLineStipple(m_lineStipple);
    effGen->setLineWidth(m_lineWidth);

    return effGen;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextEffectGenerator::TextEffectGenerator()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TextEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const TextEffectGenerator* otherSurfaceEffect = dynamic_cast<const TextEffectGenerator*>(other);
    if (otherSurfaceEffect)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EffectGenerator* TextEffectGenerator::copy() const
{
    TextEffectGenerator* effGen = new TextEffectGenerator;

    return effGen;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    // See OpenGLResourceManager::getLinkedTextShaderProgram for code to be used here
    // Detected some issues on RHEL 6 related to text, so use an empty effect for now
    // Will fall back to fixed function rendering
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VectorEffectGenerator::VectorEffectGenerator()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VectorEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const VectorEffectGenerator* otherSurfaceEffect = dynamic_cast<const VectorEffectGenerator*>(other);
    if (otherSurfaceEffect)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EffectGenerator* VectorEffectGenerator::copy() const
{
    VectorEffectGenerator* effGen = new VectorEffectGenerator;

    return effGen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VectorEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ShaderProgramGenerator gen("VectorDrawerShaderProgram", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_VectorDrawer);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_VectorDrawer);

    cvf::ref<cvf::ShaderProgram> shaderProg = gen.generate();
    shaderProg->disableUniformTrackingForUniform("u_transformationMatrix");
    shaderProg->disableUniformTrackingForUniform("u_color");

    cvf::ref<cvf::Effect> eff = effect;
    eff->setShaderProgram(shaderProg.p());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VectorEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const {}

} // End namespace caf
