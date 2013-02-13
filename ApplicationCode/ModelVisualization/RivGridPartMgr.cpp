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

#include "RIStdInclude.h"
#include "RivGridPartMgr.h"
#include "cvfPart.h"
#include "cafEffectGenerator.h"
#include "cvfStructGrid.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "RivCellEdgeEffectGenerator.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RigGridScalarDataAccess.h"
#include "RigReservoirCellResults.h"
#include "RigEclipseCase.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGridPartMgr::RivGridPartMgr(const RigGridBase* grid, size_t gridIdx)
:   m_surfaceGenerator(grid), 
    m_faultGenerator(grid), 
    m_gridIdx(gridIdx),
    m_grid(grid),
    m_surfaceFaceFilter(grid), 
    m_faultFaceFilter(grid),
    m_opacityLevel(1.0f),
    m_defaultColor(cvf::Color3::WHITE)
{
    CVF_ASSERT(grid);
    m_cellVisibility = new cvf::UByteArray;
    m_surfaceFacesTextureCoords = new cvf::Vec2fArray;
    m_faultFacesTextureCoords = new cvf::Vec2fArray;
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
    m_surfaceFaceFilter.m_showExternalFaces = true;
    m_surfaceFaceFilter.m_showFaultFaces = false;
    m_surfaceGenerator.addFaceVisibilityFilter(&m_surfaceFaceFilter);

    m_faultGenerator.setCellVisibility(cellVisibilities);
    m_faultFaceFilter.m_showExternalFaces = false;
    m_faultFaceFilter.m_showFaultFaces = true;
    m_faultGenerator.addFaceVisibilityFilter(&m_faultFaceFilter);

    generatePartGeometry(m_surfaceGenerator, false);
    generatePartGeometry(m_faultGenerator, true);
}

void RivGridPartMgr::generatePartGeometry(cvf::StructGridGeometryGenerator& geoBuilder, bool faultGeometry)
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
            part->setSourceInfo(geoBuilder.triangleToSourceGridCellMap().p());

            part->updateBoundingBox();
            
            // Set default effect
            caf::SurfaceEffectGenerator geometryEffgen(cvf::Color4f(cvf::Color3f::WHITE), true);
            cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateEffect();
            part->setEffect(geometryOnlyEffect.p());

            if (faultGeometry)
            {
                part->setEnableMask(faultBit);
                m_faultFaces = part;
            }
            else
            {
                part->setEnableMask(surfaceBit);
                m_surfaceFaces = part;
            }
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

            cvf::ref<cvf::Effect> eff;
            if (faultGeometry)
            {
                caf::MeshEffectGenerator effGen(cvf::Color3f::BLACK);
                eff = effGen.generateEffect();

                part->setEnableMask(meshFaultBit);
                part->setEffect(eff.p());
                m_faultGridLines = part;
            }
            else
            {
                caf::MeshEffectGenerator effGen(cvf::Color3f::WHITE);
                eff = effGen.generateEffect();

                // Set priority to make sure fault lines are rendered first
                part->setPriority(10);

                part->setEnableMask(meshSurfaceBit);
                part->setEffect(eff.p());
                m_surfaceGridLines = part;
            }
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
    if(m_faultFaces.notNull()      ) model->addPart(m_faultFaces.p()      );
    if(m_faultGridLines.notNull()  ) model->addPart(m_faultGridLines.p()  );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellColor(cvf::Color4f color)
{
    if (m_surfaceFaces.isNull() && m_faultFaces.isNull()) return;

    // Set default effect
    caf::SurfaceEffectGenerator geometryEffgen(color, true);
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateEffect();

    if (m_surfaceFaces.notNull()) m_surfaceFaces->setEffect(geometryOnlyEffect.p());
    if (m_faultFaces.notNull())   m_faultFaces->setEffect(geometryOnlyEffect.p());

    if (color.a() < 1.0f)
    {
        // Set priority to make sure this transparent geometry are rendered last
        if (m_surfaceFaces.notNull()) m_surfaceFaces->setPriority(100);
        if (m_faultFaces.notNull()) m_faultFaces->setPriority(100);
    }

    m_opacityLevel = color.a();
    m_defaultColor = color.toColor3f();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot)
{
    CVF_ASSERT(cellResultSlot);

    size_t scalarSetIndex = cellResultSlot->gridScalarIndex();
    const cvf::ScalarMapper* mapper = cellResultSlot->legendConfig()->scalarMapper();

    // If the result is static, only read that.
    size_t resTimeStepIdx = timeStepIndex;
    if (cellResultSlot->hasStaticResult()) resTimeStepIdx = 0;

    RifReaderInterface::PorosityModelResultType porosityModel = RigReservoirCellResults::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());

    RigEclipseCase* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();
    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = eclipseCase->dataAccessObject(m_grid.p(), porosityModel, resTimeStepIdx, scalarSetIndex);

    if (dataAccessObject.isNull()) return;

    // Outer surface
    if (m_surfaceFaces.notNull())
    {
        m_surfaceGenerator.textureCoordinates(m_surfaceFacesTextureCoords.p(), dataAccessObject.p(), mapper);

        for(size_t i = 0; i < m_surfaceFacesTextureCoords->size(); ++i)
        {
            if ((*m_surfaceFacesTextureCoords)[i].y() != 1.0f)
            {
                if (m_opacityLevel == 1.0f) (*m_surfaceFacesTextureCoords)[i].y() = 0;
            }
        }

        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_surfaceFaces->drawable());
        if (dg) dg->setTextureCoordArray(m_surfaceFacesTextureCoords.p());

        bool usePolygonOffset = true;
        caf::ScalarMapperEffectGenerator scalarEffgen(mapper, usePolygonOffset);

        scalarEffgen.setOpacityLevel(m_opacityLevel);

        cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateEffect();

        m_surfaceFaces->setEffect(scalarEffect.p());
    }

    // Faults
    if (m_faultFaces.notNull())
    {
        m_faultGenerator.textureCoordinates(m_faultFacesTextureCoords.p(), dataAccessObject.p(), mapper);

        for(size_t i = 0; i < m_faultFacesTextureCoords->size(); ++i)
        {
            if ((*m_faultFacesTextureCoords)[i].y() != 1.0f)
            {
                if (m_opacityLevel == 1.0f) (*m_faultFacesTextureCoords)[i].y() = 0;
            }
        }

        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_faultFaces->drawable());
        if (dg) dg->setTextureCoordArray(m_faultFacesTextureCoords.p());

        bool usePolygonOffset = true;
        caf::ScalarMapperEffectGenerator scalarEffgen(mapper, usePolygonOffset);

        scalarEffgen.setOpacityLevel(m_opacityLevel);

        cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateEffect();

        m_faultFaces->setEffect(scalarEffect.p());
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
            RivCellEdgeGeometryGenerator::addCellEdgeResultsToDrawableGeo(timeStepIndex, cellResultSlot, cellEdgeResultSlot, &m_surfaceGenerator, dg);

            cvf::ScalarMapper* cellScalarMapper = NULL;
            if (cellResultSlot->hasResult()) cellScalarMapper = cellResultSlot->legendConfig()->scalarMapper();

            CellEdgeEffectGenerator cellFaceEffectGen(cellEdgeResultSlot->legendConfig()->scalarMapper(), cellScalarMapper);
            cellFaceEffectGen.setOpacityLevel(m_opacityLevel);
            cellFaceEffectGen.setDefaultCellColor(m_defaultColor);

            cvf::ref<cvf::Effect> eff = cellFaceEffectGen.generateEffect();

            m_surfaceFaces->setEffect(eff.p());
        }
    }
    if (m_faultFaces.notNull())
    {
        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_faultFaces->drawable());
        if (dg) 
        {
            RivCellEdgeGeometryGenerator::addCellEdgeResultsToDrawableGeo(timeStepIndex, cellResultSlot, cellEdgeResultSlot, &m_faultGenerator, dg);

            cvf::ScalarMapper* cellScalarMapper = NULL;
            if (cellResultSlot->hasResult()) cellScalarMapper = cellResultSlot->legendConfig()->scalarMapper();

            CellEdgeEffectGenerator cellFaceEffectGen(cellEdgeResultSlot->legendConfig()->scalarMapper(), cellScalarMapper);
            cellFaceEffectGen.setOpacityLevel(m_opacityLevel);
            cellFaceEffectGen.setDefaultCellColor(m_defaultColor);

            cvf::ref<cvf::Effect> eff = cellFaceEffectGen.generateEffect();

            m_faultFaces->setEffect(eff.p());
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
