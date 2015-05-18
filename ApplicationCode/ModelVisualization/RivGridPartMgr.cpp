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
#include "RigCaseData.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimTernaryLegendConfig.h"
#include "RimWellCollection.h"

#include "RivCellEdgeEffectGenerator.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivSourceInfo.h"
#include "RivTernaryScalarMapperEffectGenerator.h"
#include "RivTernaryTextureCoordsCreator.h"
#include "RivTextureCoordsCreator.h"

#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafProgressInfo.h"

#include "cvfDrawableGeo.h"
#include "cvfMath.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfRenderState_FF.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfStructGrid.h"
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
            part->setId(m_gridIdx);       // !! For now, use grid index as part ID (needed for pick info)
            part->setDrawable(geo.p());
            part->setTransform(m_scaleTransform.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivSourceInfo> si = new RivSourceInfo;
            si->m_cellFaceFromTriangleMapper = geoBuilder.triangleToCellFaceMapper();

            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            
            // Set default effect
            caf::SurfaceEffectGenerator geometryEffgen(cvf::Color4f(cvf::Color3f::WHITE), caf::PO_1);
            cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateEffect();
            part->setEffect(geometryOnlyEffect.p());
            part->setEnableMask(surfaceBit);
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
            eff = effGen.generateEffect();

            // Set priority to make sure fault lines are rendered first
            part->setPriority(10);

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
    CVF_ASSERT(model != NULL);

    if(m_surfaceFaces.notNull()    ) model->addPart(m_surfaceFaces.p()    );
    if(m_surfaceGridLines.notNull()) model->addPart(m_surfaceGridLines.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellColor(cvf::Color4f color)
{
    if (m_surfaceFaces.isNull()) return;

    // Set default effect
    caf::SurfaceEffectGenerator geometryEffgen(color, caf::PO_1);
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateEffect();

    if (m_surfaceFaces.notNull()) m_surfaceFaces->setEffect(geometryOnlyEffect.p());

    if (color.a() < 1.0f)
    {
        // Set priority to make sure this transparent geometry are rendered last
        if (m_surfaceFaces.notNull()) m_surfaceFaces->setPriority(100);
    }

    m_opacityLevel = color.a();
    m_defaultColor = color.toColor3f();

    // Update mesh colors as well, in case of change
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect> eff;
    if (m_surfaceFaces.notNull())
    {
        caf::MeshEffectGenerator effGen(prefs->defaultGridLineColors());
        eff = effGen.generateEffect();
        m_surfaceGridLines->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot)
{
    CVF_ASSERT(cellResultSlot);

    RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();

    cvf::ref<cvf::Color3ubArray> surfaceFacesColorArray;

    // Outer surface
	if (m_surfaceFaces.notNull())
	{
		if (cellResultSlot->isTernarySaturationSelected())
		{
			RivTernaryTextureCoordsCreator texturer(cellResultSlot, cellResultSlot->ternaryLegendConfig(),
				timeStepIndex,
				m_grid->gridIndex(),
				m_surfaceGenerator.quadToCellFaceMapper());

			texturer.createTextureCoords(m_surfaceFacesTextureCoords.p());

			const RivTernaryScalarMapper* mapper = cellResultSlot->ternaryLegendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_surfaceFaces.p(), m_surfaceFacesTextureCoords.p(), mapper, m_opacityLevel, caf::FC_NONE);
		}
		else
		{
			RivTextureCoordsCreator texturer(cellResultSlot,
				timeStepIndex,
				m_grid->gridIndex(),
				m_surfaceGenerator.quadToCellFaceMapper());
			if (!texturer.isValid())
			{
				return;
			}

			texturer.createTextureCoords(m_surfaceFacesTextureCoords.p());

			const cvf::ScalarMapper* mapper = cellResultSlot->legendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTextureResultsToPart(m_surfaceFaces.p(), m_surfaceFacesTextureCoords.p(), mapper, m_opacityLevel, caf::FC_NONE);
		}
	}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot)
{
	if (m_surfaceFaces.notNull())
	{
		cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_surfaceFaces->drawable());
		if (dg)
		{
			cvf::ref<cvf::Effect> eff = RivScalarMapperUtils::createCellEdgeEffect(dg, m_surfaceGenerator.quadToCellFaceMapper(), m_grid->gridIndex(),
				timeStepIndex, cellResultSlot, cellEdgeResultSlot, m_opacityLevel, m_defaultColor, caf::FC_NONE);

			m_surfaceFaces->setEffect(eff.p());
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


