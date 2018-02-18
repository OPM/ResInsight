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

#include "RivGridPartMgr.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigResultAccessorFactory.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTernaryLegendConfig.h"

#include "RivCellEdgeEffectGenerator.h"
#include "RivPartPriority.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivSourceInfo.h"
#include "RivTernaryScalarMapperEffectGenerator.h"
#include "RivTernaryTextureCoordsCreator.h"
#include "RivTextureCoordsCreator.h"
#include "RivCompletionTypeResultToTextureMapper.h"

#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafProgressInfo.h"

#include "cvfDrawableGeo.h"
#include "cvfMath.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRenderState_FF.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfStructGrid.h"
#include "cvfTransform.h"
#include "cvfUniform.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGridPartMgr::RivGridPartMgr(const RigGridBase* grid, size_t gridIdx)
:   m_surfaceGenerator(grid), 
    m_gridIdx(gridIdx),
    m_grid(grid),
    m_surfaceFaceFilter(grid), 
    m_opacityLevel(1.0f),
    m_defaultColor(cvf::Color3::WHITE)
{
    CVF_ASSERT(grid);
    m_cellVisibility = new cvf::UByteArray;
    m_surfaceFacesTextureCoords = new cvf::Vec2fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::setTransform(cvf::Transform* scaleTransform)
{
    m_scaleTransform = scaleTransform;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::setCellVisibility(cvf::UByteArray* cellVisibilities)
{
    CVF_ASSERT(m_scaleTransform.notNull());
    CVF_ASSERT(cellVisibilities);

    m_cellVisibility = cellVisibilities;

    m_surfaceGenerator.setCellVisibility(cellVisibilities);
    m_surfaceGenerator.addFaceVisibilityFilter(&m_surfaceFaceFilter);

    generatePartGeometry(m_surfaceGenerator);
}

void RivGridPartMgr::generatePartGeometry(cvf::StructGridGeometryGenerator& geoBuilder)
{
    bool useBufferObjects = true;

    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = geoBuilder.generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid " + cvf::String(static_cast<int>(m_gridIdx)));
            part->setDrawable(geo.p());
            part->setTransform(m_scaleTransform.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivSourceInfo> si = new RivSourceInfo(m_gridIdx);
            si->m_cellFaceFromTriangleMapper = geoBuilder.triangleToCellFaceMapper();

            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            
            // Set default effect
            caf::SurfaceEffectGenerator geometryEffgen(cvf::Color4f(cvf::Color3f::WHITE), caf::PO_1);
            cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();
            part->setEffect(geometryOnlyEffect.p());
            part->setEnableMask(surfaceBit);

            if (m_opacityLevel < 1.0f)
            {
                part->setPriority(RivPartPriority::PartType::Transparent);
            }

            m_surfaceFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = geoBuilder.createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid mesh " + cvf::String(static_cast<int>(m_gridIdx)));
            part->setDrawable(geoMesh.p());

            part->setTransform(m_scaleTransform.p());
            part->updateBoundingBox();

            RiaPreferences* prefs = RiaApplication::instance()->preferences();

            cvf::ref<cvf::Effect> eff;
            caf::MeshEffectGenerator effGen(prefs->defaultGridLineColors());
            eff = effGen.generateCachedEffect();

            part->setPriority(RivPartPriority::PartType::MeshLines);

            part->setEnableMask(meshSurfaceBit);
            part->setEffect(eff.p());
            m_surfaceGridLines = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::appendPartsToModel(cvf::ModelBasicList* model)
{
    CVF_ASSERT(model != nullptr);

    if(m_surfaceFaces.notNull()    ) model->addPart(m_surfaceFaces.p()    );
    if(m_surfaceGridLines.notNull()) model->addPart(m_surfaceGridLines.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellColor(cvf::Color4f color)
{
    m_opacityLevel = color.a();
    m_defaultColor = color.toColor3f();

    if (m_surfaceFaces.isNull()) return;

    // Set default effect
    caf::SurfaceEffectGenerator geometryEffgen(color, caf::PO_1);
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();
    m_surfaceFaces->setEffect(geometryOnlyEffect.p());

    if (m_opacityLevel < 1.0f)  m_surfaceFaces->setPriority(RivPartPriority::PartType::Transparent);
    else                        m_surfaceFaces->setPriority(RivPartPriority::PartType::BaseLevel);

    // Update mesh colors as well, in case of change
    if (m_surfaceGridLines.notNull())
    {
        RiaPreferences* prefs = RiaApplication::instance()->preferences();
        caf::MeshEffectGenerator effGen(prefs->defaultGridLineColors());
        cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();
        m_surfaceGridLines->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    CVF_ASSERT(cellResultColors);

    // Outer surface
    if (m_surfaceFaces.notNull())
    {
        float effectiveOpacityLevel = m_opacityLevel;

        if (cellResultColors->isTernarySaturationSelected())
        {
            RivTernaryTextureCoordsCreator texturer(cellResultColors, cellResultColors->ternaryLegendConfig(),
                                                    timeStepIndex,
                                                    m_grid->gridIndex(),
                                                    m_surfaceGenerator.quadToCellFaceMapper());

            texturer.createTextureCoords(m_surfaceFacesTextureCoords.p());

            const RivTernaryScalarMapper* mapper = cellResultColors->ternaryLegendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_surfaceFaces.p(), 
                                                                   m_surfaceFacesTextureCoords.p(), 
                                                                   mapper, 
                                                                   effectiveOpacityLevel,
                                                                   caf::FC_NONE, 
                                                                   cellResultColors->reservoirView()->isLightingDisabled());
        }
        else
        {
            RivTextureCoordsCreator texturer(cellResultColors,
                                             timeStepIndex,
                                             m_grid->gridIndex(),
                                             m_surfaceGenerator.quadToCellFaceMapper());
            if (!texturer.isValid())
            {
                return;
            }

            if (cellResultColors->isCompletionTypeSelected())
            {
                cvf::ref<RigPipeInCellEvaluator> pipeInCellEval = RivTextureCoordsCreator::createPipeInCellEvaluator(cellResultColors, timeStepIndex, m_grid->gridIndex());
                const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();

                texturer.setResultToTextureMapper(new RivCompletionTypeResultToTextureMapper(mapper, pipeInCellEval.p()));
                effectiveOpacityLevel = 0.5;
            }

            texturer.createTextureCoords(m_surfaceFacesTextureCoords.p());

            const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTextureResultsToPart(m_surfaceFaces.p(), 
                                                            m_surfaceFacesTextureCoords.p(), 
                                                            mapper, 
                                                            effectiveOpacityLevel, 
                                                            caf::FC_NONE, 
                                                            cellResultColors->reservoirView()->isLightingDisabled());
        }

        if (effectiveOpacityLevel < 1.0f)   m_surfaceFaces->setPriority(RivPartPriority::PartType::Transparent);
        else                                m_surfaceFaces->setPriority(RivPartPriority::PartType::BaseLevel);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors)
{
    if (m_surfaceFaces.notNull())
    {
        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_surfaceFaces->drawable());
        if (dg)
        {
            cvf::ref<cvf::Effect> eff = RivScalarMapperUtils::createCellEdgeEffect(dg, m_surfaceGenerator.quadToCellFaceMapper(), m_grid->gridIndex(),
                timeStepIndex, cellResultColors, cellEdgeResultColors, m_opacityLevel, m_defaultColor, caf::FC_NONE, cellResultColors->reservoirView()->isLightingDisabled());

            m_surfaceFaces->setEffect(eff.p());

            if (m_opacityLevel < 1.0f)  m_surfaceFaces->setPriority(RivPartPriority::PartType::Transparent);
            else                        m_surfaceFaces->setPriority(RivPartPriority::PartType::BaseLevel);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGridPartMgr::~RivGridPartMgr()
{
#if 0
    if (m_faultFaces.notNull()) m_faultFaces->deleteOrReleaseOpenGLResources();
    if (m_faultGridLines.notNull()) m_faultGridLines->deleteOrReleaseOpenGLResources();
    if (m_surfaceGridLines.notNull()) m_surfaceGridLines->deleteOrReleaseOpenGLResources();
    if (m_surfaceFaces.notNull()) m_surfaceFaces->deleteOrReleaseOpenGLResources();
#endif
}


