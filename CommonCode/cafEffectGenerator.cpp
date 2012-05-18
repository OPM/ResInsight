//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cafEffectGenerator.h"
#include "cafUtils.h"

#include "cvfRenderState_FF.h"
#include "cvfTextureImage.h"
#include "cvfTexture2D_FF.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfUniform.h"
#include "cvfShaderProgram.h"
#include "cvfMatrixState.h"
#include "cvfTexture.h"
#include "cvfSampler.h"

#include <QtOpenGL/QGLFormat>

namespace caf {


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
/// Creates a new effect using the settings in the inherited generator. 
/// Creates a new effect and calls the correct update-Effect method dep. on the effect type (software/shader)
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> EffectGenerator::generateEffect() const
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
    CVF_ASSERT(effect != NULL);

    // Clear effect

    effect->setRenderStateSet(NULL);
    effect->setUniformSet(NULL);
    effect->setShaderProgram(NULL);

    if (sm_renderingMode == SHADER_BASED)
    {
        updateForShaderBasedRendering(effect);
    }
    else
    {
        updateForFixedFunctionRendering(effect);
    }
}

//==================================================================================================
//
// SurfaceEffectGenerator
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SurfaceEffectGenerator::SurfaceEffectGenerator(const cvf::Color4f& color, bool polygonOffset)
{
    m_color = color;
    m_polygonOffset = polygonOffset;
    m_cullBackfaces = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SurfaceEffectGenerator::SurfaceEffectGenerator(const cvf::Color3f& color, bool polygonOffset)
{
    m_color = cvf::Color4f(color, 1.0f);
    m_polygonOffset = polygonOffset;
    m_cullBackfaces = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SurfaceEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ShaderProgramGenerator gen("SurfaceEffectGenerator", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);
    gen.addFragmentCode(cvf::ShaderSourceRepository::light_AmbientDiffuse);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);

    cvf::ref<cvf::ShaderProgram> shaderProg = gen.generate();

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

    cvf::ref<cvf::Material_FF> mat = new cvf::Material_FF(m_color.toColor3f());
    mat->setAlpha(m_color.a());

    eff->setRenderState(mat.p());

    cvf::ref<cvf::Lighting_FF> lighting = new cvf::Lighting_FF;
    lighting->enableTwoSided(true);
    eff->setRenderState(lighting.p());

    this->updateCommonEffect(effect);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SurfaceEffectGenerator::updateCommonEffect(cvf::Effect* effect) const
{
    if (m_polygonOffset)
    {
        cvf::ref<cvf::PolygonOffset> polyOffset = new cvf::PolygonOffset;
        polyOffset->configurePolygonPositiveOffset();
        effect->setRenderState(polyOffset.p());
    }

    // Simple transparency
    if (m_color.a() < 1.0f)
    {
        cvf::ref<cvf::Blending> blender = new cvf::Blending;
        blender->configureTransparencyBlending();
        effect->setRenderState(blender.p());
    }

    // Backface culling

    if (m_cullBackfaces)
    {
        cvf::ref<cvf::CullFace> faceCulling = new cvf::CullFace;
        effect->setRenderState(faceCulling.p());
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
            && m_cullBackfaces == otherSurfaceEffect->m_cullBackfaces)
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
ScalarMapperEffectGenerator::ScalarMapperEffectGenerator(const cvf::ScalarMapper* scalarMapper, bool polygonOffset)
    : m_undefinedColor(cvf::Color3::GRAY)
{
    m_scalarMapper = scalarMapper;
    m_polygonOffset = polygonOffset;
    m_opacityLevel = 1.0f;
    m_cullBackfaces = false;
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
    gen.addFragmentCode(cvf::ShaderSourceRepository::light_AmbientDiffuse);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);

    cvf::ref<cvf::ShaderProgram> prog = gen.generate();
    eff->setShaderProgram(prog.p());

    // Result mapping texture

    cvf::ref<cvf::TextureImage> texImg = new cvf::TextureImage;
    m_scalarMapper->updateTexture(texImg.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(texImg.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture> texture = new cvf::Texture(modTexImg.p());
    cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    cvf::ref<cvf::TextureBindings> texBind = new cvf::TextureBindings;
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

    cvf::ref<cvf::Material_FF> mat = new cvf::Material_FF(cvf::Color3::WHITE);
    eff->setRenderState(mat.p());

    cvf::ref<cvf::Lighting_FF> lighting = new cvf::Lighting_FF;
    lighting->enableTwoSided(true);
    eff->setRenderState(lighting.p());

    // Result mapping texture

    cvf::ref<cvf::TextureImage> texImg = new cvf::TextureImage;
    m_scalarMapper->updateTexture(texImg.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(texImg.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture2D_FF> texture = new cvf::Texture2D_FF(modTexImg.p());
    texture->setWrapMode(cvf::Texture2D_FF::CLAMP);
    texture->setMinFilter(cvf::Texture2D_FF::NEAREST);
    texture->setMagFilter(cvf::Texture2D_FF::NEAREST);
    cvf::ref<cvf::TextureMapping_FF> texMapping = new cvf::TextureMapping_FF(texture.p());
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

    if (m_polygonOffset)
    {
        cvf::ref<cvf::PolygonOffset> polyOffset = new cvf::PolygonOffset;
        polyOffset->configurePolygonPositiveOffset();
        effect->setRenderState(polyOffset.p());
    }

    // Simple transparency
    if (m_opacityLevel < 1.0f)
    {
        cvf::ref<cvf::Blending> blender = new cvf::Blending;
        blender->configureTransparencyBlending();
        effect->setRenderState(blender.p());
    }

    // Backface culling

    if (m_cullBackfaces)
    {
        cvf::ref<cvf::CullFace> faceCulling = new cvf::CullFace;
        effect->setRenderState(faceCulling.p());
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
            && m_cullBackfaces == otherTextureResultEffect->m_cullBackfaces)
        {
            return true;
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
    scEffGen->m_opacityLevel = m_opacityLevel;
    scEffGen->m_undefinedColor = m_undefinedColor;
    scEffGen->m_cullBackfaces = m_cullBackfaces;

    return scEffGen;
}

//--------------------------------------------------------------------------------------------------
/// Modifies the supplied one line texture by adding two more pixel lines
/// one with a transparent version of the legend color, and one with color for undefined values
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> 
ScalarMapperEffectGenerator::addAlphaAndUndefStripes(const cvf::TextureImage* texImg, const cvf::Color3f& undefScalarColor, float opacityLevel)
{
    CVF_ASSERT(texImg != NULL);
    CVF_ASSERT(texImg->height() == 1);

    cvf::ref<cvf::TextureImage> modTexImg = new cvf::TextureImage;
    modTexImg->allocate(texImg->width(), texImg->height() + 2);
    modTexImg->fill(cvf::Color4ub(cvf::Color3ub(undefScalarColor), 255)); // Undefined color

    for (size_t i = 0 ; i < texImg->width(); ++i)
    {
        cvf::Color4ub legendColor = texImg->pixel(i, 0);
        modTexImg->setPixel(i, 0, legendColor);
        legendColor.a() = static_cast<cvf::ubyte>(opacityLevel * 255);
        modTexImg->setPixel(i, 1, legendColor);
    }

    return modTexImg;
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

    cvf::ref<cvf::TextureImage> texImg = new cvf::TextureImage;
    m_scalarMapper->updateTexture(texImg.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(texImg.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture> texture = new cvf::Texture(modTexImg.p());
    cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    cvf::ref<cvf::TextureBindings> texBind = new cvf::TextureBindings;
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

    eff->setRenderState(new cvf::Material_FF(cvf::Color3::WHITE));
    eff->setRenderState(new cvf::PolygonMode(cvf::PolygonMode::LINE));
    eff->setRenderState(new cvf::Depth(true, cvf::Depth::LEQUAL));
    eff->setRenderState(new cvf::Lighting_FF(false));

    // Result mapping texture

    cvf::ref<cvf::TextureImage> texImg = new cvf::TextureImage;
    m_scalarMapper->updateTexture(texImg.p());

    cvf::ref<cvf::TextureImage> modTexImg = ScalarMapperEffectGenerator::addAlphaAndUndefStripes(texImg.p(), m_undefinedColor, m_opacityLevel);

    cvf::ref<cvf::Texture2D_FF> texture = new cvf::Texture2D_FF(modTexImg.p());
    texture->setWrapMode(cvf::Texture2D_FF::CLAMP);
    texture->setMinFilter(cvf::Texture2D_FF::NEAREST);
    texture->setMagFilter(cvf::Texture2D_FF::NEAREST);

    cvf::ref<cvf::TextureMapping_FF> texMapping = new cvf::TextureMapping_FF(texture.p());
    texMapping->setTextureFunction(cvf::TextureMapping_FF::DECAL);
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
        cvf::ref<cvf::Blending> blender = new cvf::Blending;
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
            return true;
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

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MeshEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    eff->setRenderState(new cvf::Material_FF(m_color));
    eff->setRenderState(new cvf::PolygonMode(cvf::PolygonMode::LINE));
    eff->setRenderState(new cvf::Depth(true, cvf::Depth::LEQUAL));
    eff->setRenderState(new cvf::Lighting_FF(false));

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool MeshEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const MeshEffectGenerator* otherMesh = dynamic_cast<const MeshEffectGenerator*>(other);

    if (otherMesh)
    {
        if (m_color == otherMesh->m_color)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EffectGenerator* MeshEffectGenerator::copy() const
{
    return new MeshEffectGenerator(m_color);
}



} // End namespace caf
