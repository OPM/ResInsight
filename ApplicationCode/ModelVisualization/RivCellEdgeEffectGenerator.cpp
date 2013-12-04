/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "RivCellEdgeEffectGenerator.h"

#include "cvfBase.h"
#include "cvfAssert.h"
#include "cvfDrawableGeo.h"
#include "cvfVertexAttribute.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfScalarMapperUniformLevels.h"

#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfqtUtils.h"
#include "cvfShaderProgram.h"
#include "cvfRenderStateCullFace.h"

#include "cvfTextureImage.h"
#include "cvfTexture.h"
#include "cvfSampler.h"
#include "cvfScalarMapper.h"
#include "cafEffectGenerator.h"

#include <vector>

#include <QFile>
#include <QTextStream>

#include "RimCase.h"

#include "RimReservoirView.h"
#include "RimResultSlot.h"

#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigActiveCellInfo.h"

#include "RimReservoirCellResultsCacher.h"

#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCellEdgeGeometryGenerator::addCellEdgeResultsToDrawableGeo(
    size_t timeStepIndex, 
    RimResultSlot* cellResultSlot, 
    RimCellEdgeResultSlot* cellEdgeResultSlot, 
    cvf::StructGridGeometryGenerator* generator, 
    cvf::DrawableGeo* geo, 
    size_t gridIndex, 
    float opacityLevel)
{
    const std::vector<size_t>& quadToCell = generator->quadToGridCellIndices();
    const std::vector<cvf::StructGridInterface::FaceType>& quadToFace = generator->quadToFace();

    size_t vertexCount = geo->vertexArray()->size();
    size_t quadCount = vertexCount / 4;

    cvf::ref<cvf::Vec2fArray> localCoords = new cvf::Vec2fArray;
    localCoords->resize(vertexCount);

    cvf::ref<cvf::IntArray> faceIndexArray = new cvf::IntArray;
    faceIndexArray->resize(vertexCount);

    cvf::ref<cvf::FloatArray> cellColorTextureCoordArray = new cvf::FloatArray;
    cellColorTextureCoordArray->resize(vertexCount);

    // Build six cell face color arrays
    cvf::Collection<cvf::FloatArray> cellEdgeColorTextureCoordsArrays;
    size_t idx;
    for (idx = 0; idx < 6; idx++)
    {
        cvf::ref<cvf::FloatArray> colorArray = new cvf::FloatArray;
        colorArray->resize(vertexCount);
        cellEdgeColorTextureCoordsArrays.push_back(colorArray.p());
    }

    cvf::ScalarMapper* cellResultScalarMapper = cellResultSlot->legendConfig()->scalarMapper();
    cvf::ScalarMapper* edgeResultScalarMapper = cellEdgeResultSlot->legendConfig()->scalarMapper();

    const RigGridBase* grid = dynamic_cast<const RigGridBase*>(generator->activeGrid());
    CVF_ASSERT(grid != NULL);

    RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();
    CVF_ASSERT(eclipseCase != NULL);

    cvf::ref<cvf::StructGridScalarDataAccess> cellCenterDataAccessObject = NULL;
    if (cellResultSlot->hasResult())
    {
        if (!cellResultSlot->hasDynamicResult())
        {
            // Static result values are located at time step 0
            timeStepIndex = 0;
        }

        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());
        cellCenterDataAccessObject = eclipseCase->dataAccessObject(grid, porosityModel, timeStepIndex, cellResultSlot->gridScalarIndex());
    }

    CVF_ASSERT(cellEdgeResultSlot->hasResult());

    size_t resultIndices[6];
    cellEdgeResultSlot->gridScalarIndices(resultIndices);

    cvf::Collection<cvf::StructGridScalarDataAccess> cellEdgeDataAccessObjects;

    size_t cubeFaceIdx;
    for (cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
    {
        cvf::ref<cvf::StructGridScalarDataAccess> daObj;

        if (resultIndices[cubeFaceIdx] != cvf::UNDEFINED_SIZE_T)
        {
            // Assuming static values to be mapped onto cell edge, always using time step zero
            // TODO: Now hardcoded matrix results, should it be possible to use fracture results?
            daObj = eclipseCase->dataAccessObject(grid, RifReaderInterface::MATRIX_RESULTS, 0, resultIndices[cubeFaceIdx]);
        }

        cellEdgeDataAccessObjects.push_back(daObj.p());
    }

    double ignoredScalarValue = cellEdgeResultSlot->ignoredScalarValue();

    const std::vector<cvf::ubyte>* isWellPipeVisible = NULL;     
    cvf::ref<cvf::UIntArray>       gridCellToWellindexMap;

    if (opacityLevel < 1.0f)
    {
        isWellPipeVisible = &(cellResultSlot->reservoirView()->wellCollection()->isWellPipesVisible(timeStepIndex));
        gridCellToWellindexMap = eclipseCase->gridCellToWellIndex( gridIndex );
    }

#pragma omp parallel for
    for (int quadIdx = 0; quadIdx < static_cast<int>(quadCount); quadIdx++)
    {
        localCoords->set(quadIdx * 4 + 0, cvf::Vec2f(0, 0));
        localCoords->set(quadIdx * 4 + 1, cvf::Vec2f(1, 0));
        localCoords->set(quadIdx * 4 + 2, cvf::Vec2f(1, 1));
        localCoords->set(quadIdx * 4 + 3, cvf::Vec2f(0, 1));

        faceIndexArray->set(quadIdx * 4 + 0, quadToFace[quadIdx] );
        faceIndexArray->set(quadIdx * 4 + 1, quadToFace[quadIdx] );
        faceIndexArray->set(quadIdx * 4 + 2, quadToFace[quadIdx] );
        faceIndexArray->set(quadIdx * 4 + 3, quadToFace[quadIdx] );

        float cellColorTextureCoord = 0.5f; // If no results exists, the texture will have a special color
        size_t cellIndex = quadToCell[quadIdx];

        {
            double scalarValue = HUGE_VAL;

            if (cellCenterDataAccessObject.notNull())
            {
                scalarValue = cellCenterDataAccessObject->cellScalar(cellIndex);
            }

            if (scalarValue != HUGE_VAL)
            {

                cellColorTextureCoord = cellResultScalarMapper->mapToTextureCoord(scalarValue)[0];
                // If we are dealing with wellcells, the default is transparent.
                // we need to make cells opaque if there are no wellpipe through them.
                if (opacityLevel < 1.0f)
                {
                    cvf::uint wellIndex = gridCellToWellindexMap->get(cellIndex);
                    if (wellIndex != cvf::UNDEFINED_UINT)
                    {
                        if ( !(*isWellPipeVisible)[wellIndex]) 
                        {
                            cellColorTextureCoord += 2.0f; // The shader must interpret values in the range 2-3 as "opaque"
                        }
                    }
                }
            }
            else
            {
                cellColorTextureCoord = -1.0f; // Undefined texture coord. Shader handles this.
            }
        }

        cellColorTextureCoordArray->set(quadIdx * 4 + 0, cellColorTextureCoord);
        cellColorTextureCoordArray->set(quadIdx * 4 + 1, cellColorTextureCoord);
        cellColorTextureCoordArray->set(quadIdx * 4 + 2, cellColorTextureCoord);
        cellColorTextureCoordArray->set(quadIdx * 4 + 3, cellColorTextureCoord);

        size_t cubeFaceIdx;
        float edgeColor;
        for (cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
        {
            edgeColor = -1.0f; // Undefined texture coord. Shader handles this.

            double scalarValue = HUGE_VAL;
            if (cellEdgeDataAccessObjects[cubeFaceIdx].notNull())
            {
                scalarValue = cellEdgeDataAccessObjects[cubeFaceIdx]->cellScalar(cellIndex);
            }

            if (scalarValue != HUGE_VAL && scalarValue != ignoredScalarValue)
            {
                edgeColor = edgeResultScalarMapper->mapToTextureCoord(scalarValue)[0];
            }

            cvf::FloatArray* colArr = cellEdgeColorTextureCoordsArrays.at(cubeFaceIdx);

            colArr->set(quadIdx * 4 + 0, edgeColor);
            colArr->set(quadIdx * 4 + 1, edgeColor);
            colArr->set(quadIdx * 4 + 2, edgeColor);
            colArr->set(quadIdx * 4 + 3, edgeColor);
        }
    }

    geo->setVertexAttribute(new cvf::Vec2fVertexAttribute("a_localCoord", localCoords.p()));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorCell", cellColorTextureCoordArray.p()));

    cvf::ref<cvf::IntVertexAttributeDirect> faceIntAttribute =  new cvf::IntVertexAttributeDirect("a_face", faceIndexArray.p());
    //faceIntAttribute->setIntegerTypeConversion(cvf::VertexAttribute::DIRECT_FLOAT);
    geo->setVertexAttribute(faceIntAttribute.p());

    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosI", cellEdgeColorTextureCoordsArrays.at(0)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegI", cellEdgeColorTextureCoordsArrays.at(1)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosJ", cellEdgeColorTextureCoordsArrays.at(2)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegJ", cellEdgeColorTextureCoordsArrays.at(3)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosK", cellEdgeColorTextureCoordsArrays.at(4)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegK", cellEdgeColorTextureCoordsArrays.at(5)));
}






//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CellEdgeEffectGenerator::CellEdgeEffectGenerator(const cvf::ScalarMapper* edgeScalarMapper, const cvf::ScalarMapper* cellScalarMapper)
{
    CVF_ASSERT(edgeScalarMapper != NULL);

    m_cellScalarMapper = cellScalarMapper;
    m_edgeScalarMapper = edgeScalarMapper;

    m_cullBackfaces = false;
    m_opacityLevel = 1.0f;
    m_defaultCellColor = cvf::Color3f(cvf::Color3::WHITE);
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
        && m_cullBackfaces        == otherCellFaceEffectGenerator->m_cullBackfaces
        && m_opacityLevel         == otherCellFaceEffectGenerator->m_opacityLevel
        && m_undefinedColor       == otherCellFaceEffectGenerator->m_undefinedColor
        && m_defaultCellColor     == otherCellFaceEffectGenerator->m_defaultCellColor
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
    CellEdgeEffectGenerator * newEffect = new CellEdgeEffectGenerator(m_edgeScalarMapper.p(), m_cellScalarMapper.p());
    newEffect->m_edgeTextureImage = m_edgeTextureImage;
    newEffect->m_cellTextureImage = m_cellTextureImage;

    newEffect->setOpacityLevel(m_opacityLevel);
    newEffect->setCullBackfaces(m_cullBackfaces);
    newEffect->setUndefinedColor(m_undefinedColor);
    newEffect->setDefaultCellColor(m_defaultCellColor);

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
        QFile data(":/Shader/fs_CellFace.glsl");
        if (data.open(QFile::ReadOnly))
        {
            QTextStream in(&data);

            QString data = in.readAll();
            cvf::String cvfString = cvfqt::Utils::toString(data);

            shaderGen.addFragmentCode(cvfString);
        }
    }

    {
        QFile data(":/Shader/vs_CellFace.glsl");
        if (data.open(QFile::ReadOnly))
        {
            QTextStream in(&data);

            QString data = in.readAll();
            cvf::String cvfString = cvfqt::Utils::toString(data);

            shaderGen.addVertexCode(cvfString);
        }
    }

    shaderGen.addFragmentCode(caf::CommonShaderSources::light_AmbientDiffuse());
    shaderGen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);

    cvf::ref<cvf::ShaderProgram> prog = shaderGen.generate();
    eff->setShaderProgram(prog.p());
 
    // Set up textures

    m_edgeTextureImage = new cvf::TextureImage;
    m_cellTextureImage = new cvf::TextureImage;

    cvf::ref<cvf::TextureImage> modifiedCellTextImage;
    m_edgeScalarMapper->updateTexture(m_edgeTextureImage.p());
    if (m_cellScalarMapper.notNull()) 
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

    // Backface culling

    if (m_cullBackfaces)
    {
        cvf::ref<cvf::RenderStateCullFace> faceCulling = new cvf::RenderStateCullFace;
        eff->setRenderState(faceCulling.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(cvf::Color3f::CRIMSON), true);

    surfaceGen.updateEffect(effect);
}

