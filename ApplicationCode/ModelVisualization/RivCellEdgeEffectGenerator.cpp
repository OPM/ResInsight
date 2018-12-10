/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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


#include "RivCellEdgeEffectGenerator.h"

#include "RivTernaryScalarMapper.h"

#include "cvfRenderStateBlending.h"
#include "cvfRenderStateCullFace.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfSampler.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfTexture.h"
#include "cvfqtUtils.h"

#include <QFile>
#include <QTextStream>
#include "cvfUniform.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CellEdgeEffectGenerator::CellEdgeEffectGenerator(const cvf::ScalarMapper* edgeScalarMapper)
{
    CVF_ASSERT(edgeScalarMapper != nullptr);

    m_edgeScalarMapper = edgeScalarMapper;

    m_cullBackfaces = caf::FC_NONE;
    m_opacityLevel = 1.0f;
    m_defaultCellColor = cvf::Color3f(cvf::Color3::WHITE);
    m_disableLighting = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::setScalarMapper(const cvf::ScalarMapper* cellScalarMapper)
{
    m_cellScalarMapper = cellScalarMapper;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::setTernaryScalarMapper(const RivTernaryScalarMapper* ternaryScalarMapper)
{
    m_ternaryCellScalarMapper = ternaryScalarMapper;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CellEdgeEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const CellEdgeEffectGenerator* otherCellFaceEffectGenerator = dynamic_cast<const CellEdgeEffectGenerator*>(other);
    
    if (otherCellFaceEffectGenerator 
        && m_cellScalarMapper.p() == otherCellFaceEffectGenerator->m_cellScalarMapper.p() 
        && m_edgeScalarMapper.p() == otherCellFaceEffectGenerator->m_edgeScalarMapper.p()
        && m_ternaryCellScalarMapper.p() == otherCellFaceEffectGenerator->m_ternaryCellScalarMapper.p()
        && m_cullBackfaces == otherCellFaceEffectGenerator->m_cullBackfaces
        && m_opacityLevel         == otherCellFaceEffectGenerator->m_opacityLevel
        && m_undefinedColor       == otherCellFaceEffectGenerator->m_undefinedColor
        && m_defaultCellColor     == otherCellFaceEffectGenerator->m_defaultCellColor
        && m_disableLighting      == otherCellFaceEffectGenerator->m_disableLighting
        )
    {
        cvf::ref<cvf::TextureImage> texImg2 = new cvf::TextureImage;

        if (otherCellFaceEffectGenerator->m_edgeScalarMapper.notNull())
        {
            otherCellFaceEffectGenerator->m_edgeScalarMapper->updateTexture(texImg2.p());
            if (!caf::ScalarMapperEffectGenerator::isImagesEqual(m_edgeTextureImage.p(), texImg2.p())) return false;
        }

        if (otherCellFaceEffectGenerator->m_cellScalarMapper.notNull())
        {
            otherCellFaceEffectGenerator->m_cellScalarMapper->updateTexture(texImg2.p());
            if (!caf::ScalarMapperEffectGenerator::isImagesEqual(m_cellTextureImage.p(), texImg2.p())) return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::EffectGenerator* CellEdgeEffectGenerator::copy() const
{
    CellEdgeEffectGenerator * newEffect = new CellEdgeEffectGenerator(m_edgeScalarMapper.p());
    newEffect->setScalarMapper(m_cellScalarMapper.p());
    newEffect->setTernaryScalarMapper(m_ternaryCellScalarMapper.p());
    newEffect->m_edgeTextureImage = m_edgeTextureImage;
    newEffect->m_cellTextureImage = m_cellTextureImage;

    newEffect->setOpacityLevel(m_opacityLevel);
    newEffect->setFaceCulling(m_cullBackfaces);
    newEffect->setUndefinedColor(m_undefinedColor);
    newEffect->setDefaultCellColor(m_defaultCellColor);
    newEffect->disableLighting(m_disableLighting);

    return newEffect;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
    cvf::ref<cvf::Effect> eff = effect;

    // Set up shader program

    cvf::ShaderProgramGenerator shaderGen("CellEdgeFaceShaderProgramGenerator", cvf::ShaderSourceProvider::instance());
    {
        QFile file(":/Shader/fs_CellFace.glsl");
        if (file.open(QFile::ReadOnly))
        {
            QTextStream in(&file);

            QString data = in.readAll();
            cvf::String cvfString = cvfqt::Utils::toString(data);

            shaderGen.addFragmentCode(cvfString);
        }
    }

    if (m_ternaryCellScalarMapper.notNull())
    {
        {
            QFile file(":/Shader/vs_2dTextureCellFace.glsl");
            if (file.open(QFile::ReadOnly))
            {
                QTextStream in(&file);

                QString data = in.readAll();
                cvf::String cvfString = cvfqt::Utils::toString(data);

                shaderGen.addVertexCode(cvfString);
            }
        }
    }
    else
    {
        {
            QFile file(":/Shader/vs_CellFace.glsl");
            if (file.open(QFile::ReadOnly))
            {
                QTextStream in(&file);

                QString data = in.readAll();
                cvf::String cvfString = cvfqt::Utils::toString(data);

                shaderGen.addVertexCode(cvfString);
            }
        }
    }

    if (m_disableLighting)
    {
        shaderGen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    }
    else
    {
        shaderGen.addFragmentCode(caf::CommonShaderSources::light_AmbientDiffuse());
        shaderGen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);
    }


    cvf::ref<cvf::ShaderProgram> prog = shaderGen.generate();
    eff->setShaderProgram(prog.p());
 
    if(!m_disableLighting) prog->setDefaultUniform(new cvf::UniformFloat("u_ecLightPosition", cvf::Vec3f(0.5, 5.0, 7.0)));

    // Set up textures

    m_edgeTextureImage = new cvf::TextureImage;
    m_cellTextureImage = new cvf::TextureImage;

    cvf::ref<cvf::TextureImage> modifiedCellTextImage;
    m_edgeScalarMapper->updateTexture(m_edgeTextureImage.p());
    if (m_ternaryCellScalarMapper.notNull())
    {
        m_ternaryCellScalarMapper->updateTexture(m_cellTextureImage.p(), m_opacityLevel);
        modifiedCellTextImage = m_cellTextureImage;
    }
    else if (m_cellScalarMapper.notNull()) 
    {
        m_cellScalarMapper->updateTexture(m_cellTextureImage.p());
        modifiedCellTextImage = caf::ScalarMapperEffectGenerator::addAlphaAndUndefStripes(m_cellTextureImage.p(), m_undefinedColor, m_opacityLevel);
    }
    else 
    {
        modifiedCellTextImage = new cvf::TextureImage;
        modifiedCellTextImage->allocate(2,1);
        modifiedCellTextImage->fill(cvf::Color4ub(cvf::Color4f(m_defaultCellColor, m_opacityLevel)));
    }

    cvf::ref<cvf::Texture> edgeTexture = new cvf::Texture(m_edgeTextureImage.p());
    cvf::ref<cvf::Texture> cellTexture = new cvf::Texture(modifiedCellTextImage.p());

    cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    cvf::ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(edgeTexture.p(), sampler.p(), "u_edgeTexture2D");
    texBind->addBinding(cellTexture.p(), sampler.p(), "u_cellTexture2D");
    eff->setRenderState(texBind.p());

    // Polygon offset

    if (true)
    {
        cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;
        polyOffset->configurePolygonPositiveOffset();
        eff->setRenderState(polyOffset.p());
    }

    // Simple transparency
    if (m_opacityLevel < 1.0f)
    {
        cvf::ref<cvf::RenderStateBlending> blender = new cvf::RenderStateBlending;
        blender->configureTransparencyBlending();
        eff->setRenderState(blender.p());
    }

    // Face culling
    if (m_cullBackfaces != caf::FC_NONE)
    {
        cvf::ref<cvf::RenderStateCullFace> faceCulling = new cvf::RenderStateCullFace;
        if (m_cullBackfaces == caf::FC_BACK)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::BACK);
        }
        else if (m_cullBackfaces == caf::FC_FRONT)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::FRONT);
        }
        else if (m_cullBackfaces == caf::FC_FRONT_AND_BACK)
        {
            faceCulling->setMode(cvf::RenderStateCullFace::FRONT_AND_BACK);
        }

        effect->setRenderState(faceCulling.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(cvf::Color3f::CRIMSON), caf::PO_1);

    surfaceGen.updateEffect(effect);
}


