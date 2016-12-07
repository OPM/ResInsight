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

    std::vector<cvf::Vec3d> cellCenters;

    for (const RigWellResultBranch& wellResultBranch : wellResultFrame.m_wellResultBranches)
    {
        for (const RigWellResultPoint& wellResultPoint : wellResultBranch.m_branchResultPoints) 
        {
            size_t gridIndex = wellResultPoint.m_gridIndex;
            size_t gridCellIndex = wellResultPoint.m_gridCellIndex;

            const RigGridBase* rigGrid = mainGrid->gridByIndex(gridIndex);
            const RigCell& rigCell = rigGrid->cell(gridCellIndex);

            cvf::Vec3d center = rigCell.center();

            cellCenters.push_back(center);
        }
    }


    for (cvf::Vec3d c : cellCenters)
    {
        cvf::Vec4d transfCoord = m_scaleTransform->worldTransform() * cvf::Vec4d(c - mainGrid->displayModelOffset(), 1);

        cvf::Vec3d displayCoord;
        displayCoord[0] = transfCoord[0];
        displayCoord[1] = transfCoord[1];
        displayCoord[2] = transfCoord[2];

        cvf::ref<cvf::DrawableGeo> geo = createSphere(10, displayCoord);
        cvf::ref<cvf::Part> part = createPart(geo.p());

        model->addPart(part.p());
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
cvf::ref<cvf::Part> RivWellSpheresPartMgr::createPart(cvf::DrawableGeo* geo)
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(geo);

    caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(cvf::Color3f::GREEN), caf::PO_1);
    cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

    part->setEffect(eff.p());

    return part;
}

