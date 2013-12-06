/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "RivFaultPart.h"

#include "cvfPart.h"
#include "cafEffectGenerator.h"
#include "cvfStructGrid.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "RivCellEdgeEffectGenerator.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RimCase.h"
#include "RimWellCollection.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimReservoirCellResultsCacher.h"





//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFaultPart::RivFaultPart(const RigGridBase* grid, const RimFault* rimFault)
    :   m_nativeFaultGenerator(grid, rimFault->faultGeometry()),
        m_grid(grid),
        m_rimFault(rimFault),
        m_opacityLevel(1.0f),
        m_defaultColor(cvf::Color3::WHITE)
{
    m_nativeFaultFacesTextureCoords = new cvf::Vec2fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPart::setCellVisibility(cvf::UByteArray* cellVisibilities)
{
    m_nativeFaultGenerator.setCellVisibility(cellVisibilities);

    generatePartGeometry();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPart::updateCellColor(cvf::Color4f color)
{
    m_defaultColor = color;

    updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPart::updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot)
{
    CVF_ASSERT(cellResultSlot);

    size_t scalarSetIndex = cellResultSlot->gridScalarIndex();
    const cvf::ScalarMapper* mapper = cellResultSlot->legendConfig()->scalarMapper();

    // If the result is static, only read that.
    size_t resTimeStepIdx = timeStepIndex;
    if (cellResultSlot->hasStaticResult()) resTimeStepIdx = 0;

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());

    RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();
    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = eclipseCase->dataAccessObject(m_grid.p(), porosityModel, resTimeStepIdx, scalarSetIndex);

    if (dataAccessObject.isNull()) return;


    // Faults
    if (m_nativeFaultFaces.notNull())
    {
        m_nativeFaultGenerator.textureCoordinates(m_nativeFaultFacesTextureCoords.p(), dataAccessObject.p(), mapper);

        if (m_opacityLevel < 1.0f )
        {
            const std::vector<cvf::ubyte>& isWellPipeVisible      = cellResultSlot->reservoirView()->wellCollection()->isWellPipesVisible(timeStepIndex);
            cvf::ref<cvf::UIntArray>       gridCellToWellindexMap = eclipseCase->gridCellToWellIndex(m_grid->gridIndex());
            const std::vector<size_t>&  quadsToGridCells = m_nativeFaultGenerator.quadToGridCellIndices();

            for(size_t i = 0; i < m_nativeFaultFacesTextureCoords->size(); ++i)
            {
                if ((*m_nativeFaultFacesTextureCoords)[i].y() == 1.0f) continue; // Do not touch undefined values

                size_t quadIdx = i/4;
                size_t cellIndex = quadsToGridCells[quadIdx];
                cvf::uint wellIndex = gridCellToWellindexMap->get(cellIndex);
                if (wellIndex != cvf::UNDEFINED_UINT)
                {
                    if ( !isWellPipeVisible[wellIndex]) 
                    {
                        (*m_nativeFaultFacesTextureCoords)[i].y() = 0; // Set the Y texture coordinate to the opaque line in the texture
                    }
                }
            }
        }

        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_nativeFaultFaces->drawable());
        if (dg) dg->setTextureCoordArray(m_nativeFaultFacesTextureCoords.p());

        bool usePolygonOffset = true;
        caf::ScalarMapperEffectGenerator scalarEffgen(mapper, usePolygonOffset);

        scalarEffgen.setOpacityLevel(m_opacityLevel);

        cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateEffect();

        m_nativeFaultFaces->setEffect(scalarEffect.p());
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPart::updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot)
{

    /*
    if (m_faultFaces.notNull())
    {
        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_faultFaces->drawable());
        if (dg) 
        {
            RivCellEdgeGeometryGenerator::addCellEdgeResultsToDrawableGeo(timeStepIndex, cellResultSlot, cellEdgeResultSlot,
                &m_faultGenerator, dg, m_grid->gridIndex(), m_opacityLevel);

            cvf::ScalarMapper* cellScalarMapper = NULL;
            if (cellResultSlot->hasResult()) cellScalarMapper = cellResultSlot->legendConfig()->scalarMapper();

            CellEdgeEffectGenerator cellFaceEffectGen(cellEdgeResultSlot->legendConfig()->scalarMapper(), cellScalarMapper);
            cellFaceEffectGen.setOpacityLevel(m_opacityLevel);
            cellFaceEffectGen.setDefaultCellColor(m_defaultColor);

            cvf::ref<cvf::Effect> eff = cellFaceEffectGen.generateEffect();

            m_faultFaces->setEffect(eff.p());
        }
    }
    */
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPart::appendPartsToModel(cvf::ModelBasicList* model)
{
    CVF_ASSERT(model != NULL);

    if (m_rimFault && m_rimFault->showFault())
    {
        if(m_nativeFaultFaces.notNull()      ) model->addPart(m_nativeFaultFaces.p()      );
        if(m_nativeFaultGridLines.notNull()  ) model->addPart(m_nativeFaultGridLines.p()  );
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPart::generatePartGeometry()
{
    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_nativeFaultGenerator.generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid " + cvf::String(static_cast<int>(m_grid->gridIndex())));
            part->setId(m_grid->gridIndex());       // !! For now, use grid index as part ID (needed for pick info)
            part->setDrawable(geo.p());
            //part->setTransform(m_scaleTransform.p());

            // Set mapping from triangle face index to cell index
            part->setSourceInfo(m_nativeFaultGenerator.triangleToSourceGridCellMap().p());

            part->updateBoundingBox();
            part->setEnableMask(faultBit);

            m_nativeFaultFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_nativeFaultGenerator.createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid mesh" + cvf::String(static_cast<int>(m_grid->gridIndex())));
            part->setDrawable(geoMesh.p());

            //part->setTransform(m_scaleTransform.p());
            part->updateBoundingBox();
            part->setEnableMask(meshFaultBit);

            m_nativeFaultGridLines = part;
        }
    }

    updatePartEffect();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPart::updatePartEffect()
{
    if (m_nativeFaultFaces.notNull())
    {
        cvf::Color3f partColor = m_defaultColor.toColor3f();

        if (m_rimFault->showFaultColor())
        {
            partColor = m_rimFault->faultColor();
        }

        if (m_defaultColor.a() < 1.0f)
        {
            // Set priority to make sure this transparent geometry are rendered last
            m_nativeFaultFaces->setPriority(100);
        }

        m_opacityLevel = m_defaultColor.a();

        // Set default effect
        caf::SurfaceEffectGenerator geometryEffgen(partColor, true);
        cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateEffect();

        m_nativeFaultFaces->setEffect(geometryOnlyEffect.p());
    }

    if (m_nativeFaultGridLines.notNull())
    {
        // Update mesh colors as well, in case of change
        RiaPreferences* prefs = RiaApplication::instance()->preferences();

        cvf::ref<cvf::Effect> eff;
        caf::MeshEffectGenerator faultEffGen(prefs->defaultFaultGridLineColors());
        eff = faultEffGen.generateEffect();
        m_nativeFaultGridLines->setEffect(eff.p());
    }
}
