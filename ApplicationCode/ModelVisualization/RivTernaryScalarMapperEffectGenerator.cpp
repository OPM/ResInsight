/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RivTernaryScalarMapperEffectGenerator.h"

#include "RiaColorTables.h"

#include "RivTernaryScalarMapper.h"

#include "cvfRenderStateBlending.h"
#include "cvfRenderStateCullFace.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfRenderState_FF.h"
#include "cvfSampler.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfTexture.h"
#include "cvfTexture2D_FF.h"
#include "cvfUniform.h"



//==================================================================================================
//
// RivTernaryScalarMapperEffectGenerator
//
//==================================================================================================



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTernaryScalarMapperEffectGenerator::RivTernaryScalarMapperEffectGenerator(const RivTernaryScalarMapper* scalarMapper, caf::PolygonOffset polygonOffset)
    : m_undefinedColor(RiaColorTables::undefinedCellColor())
{
    m_scalarMapper = scalarMapper;
    m_polygonOffset = polygonOffset;
    m_opacityLevel = 1.0f;
    m_faceCulling = caf::FC_NONE;
    m_enableDepthWrite = true;
    m_disableLighting = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernaryScalarMapperEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    cvf::ShaderProgramGenerator gen("ScalarMapperMeshEffectGenerator", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Texture);

    if (m_disableLighting)
    {
        gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    }
    else
    {
        gen.addFragmentCode(caf::CommonShaderSources::light_AmbientDiffuse());
        gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);
    }

    cvf::ref<cvf::ShaderProgram> prog = gen.generate();
    eff->setShaderProgram(prog.p());

    if(!m_disableLighting) prog->setDefaultUniform(new cvf::UniformFloat("u_ecLightPosition", cvf::Vec3f(0.5, 5.0, 7.0)));

    // Result mapping texture

    m_textureImage = new cvf::TextureImage();
    m_scalarMapper->updateTexture(m_textureImage.p(), m_opacityLevel);

    cvf::ref<cvf::Texture> texture = new cvf::Texture(m_textureImage.p());
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
void RivTernaryScalarMapperEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
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
    m_scalarMapper->updateTexture(m_textureImage.p(), m_opacityLevel);

    cvf::ref<cvf::Texture2D_FF> texture = new cvf::Texture2D_FF(m_textureImage.p());
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
void RivTernaryScalarMapperEffectGenerator::updateCommonEffect(cvf::Effect* effect) const
{
    CVF_ASSERT(effect);

    if (m_polygonOffset != caf::PO_NONE)
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
    if (m_faceCulling != caf::FC_NONE)
    {
        cvf::ref<cvf::RenderStateCullFace> faceCulling = new cvf::RenderStateCullFace;
        if (m_faceCulling == caf::FC_BACK)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::BACK);
        }
        else if (m_faceCulling == caf::FC_FRONT)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::FRONT);
        }
        else if (m_faceCulling == caf::FC_FRONT_AND_BACK)
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
bool RivTernaryScalarMapperEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const RivTernaryScalarMapperEffectGenerator* otherTextureResultEffect = dynamic_cast<const RivTernaryScalarMapperEffectGenerator*>(other);

    if (otherTextureResultEffect)
    {
        if (m_scalarMapper.p() == otherTextureResultEffect->m_scalarMapper
            && m_polygonOffset == otherTextureResultEffect->m_polygonOffset
            && m_opacityLevel == otherTextureResultEffect->m_opacityLevel
            && m_undefinedColor == otherTextureResultEffect->m_undefinedColor
            && m_faceCulling == otherTextureResultEffect->m_faceCulling
            && m_enableDepthWrite == otherTextureResultEffect->m_enableDepthWrite
            && m_disableLighting == otherTextureResultEffect->m_disableLighting)
        {
            cvf::ref<cvf::TextureImage> texImg2 = new cvf::TextureImage;
            otherTextureResultEffect->m_scalarMapper->updateTexture(texImg2.p(), m_opacityLevel);

            return RivTernaryScalarMapperEffectGenerator::isImagesEqual(m_textureImage.p(), texImg2.p());
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::EffectGenerator* RivTernaryScalarMapperEffectGenerator::copy() const
{
    RivTernaryScalarMapperEffectGenerator* scEffGen = new RivTernaryScalarMapperEffectGenerator(m_scalarMapper.p(), m_polygonOffset);
    scEffGen->m_textureImage = m_textureImage;
    scEffGen->m_opacityLevel = m_opacityLevel;
    scEffGen->m_undefinedColor = m_undefinedColor;
    scEffGen->m_faceCulling = m_faceCulling;
    scEffGen->m_enableDepthWrite = m_enableDepthWrite;
    scEffGen->m_disableLighting = m_disableLighting;

    return scEffGen;
}


//--------------------------------------------------------------------------------------------------
/// Tests whether two texture images are equal. It might in some rare cases not detect the difference
/// but to make the comparison fast only some sampling points are used. If both pointers are nullptr,
/// they are considered equal.
//--------------------------------------------------------------------------------------------------
bool RivTernaryScalarMapperEffectGenerator::isImagesEqual(const cvf::TextureImage* texImg1, const cvf::TextureImage* texImg2)
{
    if (texImg1 == nullptr && texImg2 == nullptr) return true;

    if (texImg1 != nullptr && texImg2 != nullptr
        && texImg1->height() == texImg2->height()
        && texImg1->width() == texImg2->width()
        && texImg1->width() > 0 && texImg1->height() > 0
        && texImg1->pixel(0, 0) == texImg2->pixel(0, 0)
        && texImg1->pixel(texImg1->width() - 1, texImg1->height() - 1) == texImg2->pixel(texImg1->width() - 1, texImg1->height() - 1)
        && texImg1->pixel(texImg1->width() / 2, texImg1->height() / 2) == texImg2->pixel(texImg1->width() / 2, texImg1->height() / 2)
        && texImg1->pixel(texImg1->width() / 4, texImg1->height() / 4) == texImg2->pixel(texImg1->width() / 4, texImg1->height() / 4)
        && texImg1->pixel(texImg1->width() / 2 + texImg1->width() / 4, texImg1->height() / 2 + texImg1->height() / 4) == texImg2->pixel(texImg1->width() / 2 + texImg1->width() / 4, texImg1->height() / 2 + texImg1->height() / 4)
        )
    {
        return true;
    }

    return false;
}

