/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RivWellSpheresPartMgr.h"

#include "RigCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"


#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
 
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryUtils.h"
#include "cvfModelBasicList.h"
#include "cvfObject.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellSpheresPartMgr::RivWellSpheresPartMgr(RimEclipseView* reservoirView, RimEclipseWell* well)
{
    m_rimReservoirView = reservoirView;
    m_rimWell      = well;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellSpheresPartMgr::~RivWellSpheresPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellSpheresPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (m_rimReservoirView.isNull()) return;
    if (!m_rimReservoirView->eclipseCase()) return;
    if (!m_rimReservoirView->eclipseCase()->reservoirData()) return;
    
    const RigMainGrid* mainGrid = m_rimReservoirView->eclipseCase()->reservoirData()->mainGrid();
    CVF_ASSERT(mainGrid);

    RigSingleWellResultsData* rigWellResult = m_rimWell->wellResults();
    if (!rigWellResult) return;

    if (!rigWellResult->hasWellResult(frameIndex)) return;

    const RigWellResultFrame& wellResultFrame = rigWellResult->wellResultFrame(frameIndex);

    for (const RigWellResultBranch& wellResultBranch : wellResultFrame.m_wellResultBranches)
    {
        for (const RigWellResultPoint& wellResultPoint : wellResultBranch.m_branchResultPoints) 
        {
            size_t gridIndex = wellResultPoint.m_gridIndex;

            if (gridIndex >= mainGrid->gridCount()) continue;
            
            const RigGridBase* rigGrid = rigGrid = mainGrid->gridByIndex(gridIndex);

            size_t gridCellIndex = wellResultPoint.m_gridCellIndex;
            if (gridCellIndex >= rigGrid->cellCount()) continue;

            const RigCell& rigCell = rigGrid->cell(gridCellIndex);

            double characteristicCellSize = m_rimReservoirView->eclipseCase()->reservoirData()->mainGrid()->characteristicIJCellSize();
            double cellRadius = m_rimReservoirView->wellCollection()->cellCenterSpheresScaleFactor() * characteristicCellSize;
           
            cvf::Vec3d center = rigCell.center();

            cvf::Color3f color = wellCellColor(wellResultFrame, wellResultPoint);

            cvf::ref<caf::DisplayCoordTransform> transForm = m_rimReservoirView->displayCoordTransform();
            cvf::Vec3d displayCoord = transForm->transformToDisplayCoord(center);

            cvf::ref<cvf::DrawableGeo> geo = createSphere(cellRadius, displayCoord);
            cvf::ref<cvf::Part> part = createPart(geo.p(), color);

            model->addPart(part.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellSpheresPartMgr::createSphere(double radius, const cvf::Vec3d& pos)
{
    cvf::Vec3f posFloat(pos);

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::GeometryBuilderFaceList builder;
    cvf::GeometryUtils::createSphere(radius, 4, 5, &builder);

    cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();

    // Move sphere coordinates to the destination location
    for (size_t i = 0; i < vertices->size(); i++)
    {
        vertices->set(i, vertices->val(i) + posFloat);
    }

    cvf::ref<cvf::UIntArray> faceList = builder.faceList();

    geo->setVertexArray(vertices.p());
    geo->setFromFaceList(*faceList);
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellSpheresPartMgr::createPart(cvf::DrawableGeo* geo, const cvf::Color3f& color)
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(geo);

    caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
    cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

    part->setEffect(eff.p());

    return part;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RivWellSpheresPartMgr::wellCellColor(const RigWellResultFrame& wellResultFrame, const RigWellResultPoint& wellResultPoint)
{
    // Colours should be synchronized with RivWellPipesPartMgr::updatePipeResultColor

    cvf::Color3f cellColor(cvf::Color3f::GRAY);
      
    if (wellResultPoint.m_isOpen)
    {
        switch (wellResultFrame.m_productionType)
        {
        case RigWellResultFrame::PRODUCER:
            cellColor = cvf::Color3f::GREEN;
            break;
        case RigWellResultFrame::OIL_INJECTOR:
            cellColor = cvf::Color3f::RED;
            break;
        case RigWellResultFrame::GAS_INJECTOR:
            cellColor = cvf::Color3f::RED;
            break;
        case RigWellResultFrame::WATER_INJECTOR:
            cellColor = cvf::Color3f::BLUE;
            break;
        case RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE:
            cellColor = cvf::Color3f::GRAY;
            break;
        }
    }

    return cellColor;
}

