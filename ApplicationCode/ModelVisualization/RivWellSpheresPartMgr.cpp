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
#include "cvfDrawableVectors.h"
#include "cvfGeometryBuilderTriangles.h"

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

    std::vector<std::pair<cvf::Vec3f, cvf::Color3f> > centerColorPairs;

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

           
            cvf::Vec3d center = rigCell.center();
            cvf::ref<caf::DisplayCoordTransform> transForm = m_rimReservoirView->displayCoordTransform();
            cvf::Vec3d displayCoord = transForm->transformToDisplayCoord(center);

            cvf::Color3f color = wellCellColor(wellResultFrame, wellResultPoint);

            centerColorPairs.push_back(std::make_pair(cvf::Vec3f(displayCoord), color));
        }
    }



    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray;
    cvf::ref<cvf::Vec3fArray> vecRes = new cvf::Vec3fArray;
    cvf::ref<cvf::Color3fArray> colors = new cvf::Color3fArray;

    size_t numVecs = centerColorPairs.size();
    vertices->reserve(numVecs);
    vecRes->reserve(numVecs);
    colors->reserve(numVecs);

    for (auto centerColorPair : centerColorPairs)
    {
        vertices->add(centerColorPair.first);
        vecRes->add(cvf::Vec3f::X_AXIS);
        colors->add(centerColorPair.second);
    }

    cvf::ref<cvf::DrawableVectors> vectorDrawable = new cvf::DrawableVectors();
    vectorDrawable->setVectors(vertices.p(), vecRes.p());
    vectorDrawable->setColors(colors.p());

    cvf::GeometryBuilderTriangles builder;
    double characteristicCellSize = m_rimReservoirView->eclipseCase()->reservoirData()->mainGrid()->characteristicIJCellSize();
    double cellRadius = m_rimReservoirView->wellCollection()->cellCenterSpheresScaleFactor() * characteristicCellSize;
    cvf::GeometryUtils::createSphere(cellRadius, 15, 15, &builder);

    vectorDrawable->setGlyph(builder.trianglesUShort().p(), builder.vertices().p());

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(vectorDrawable.p());
    cvf::ref<cvf::Effect> eff2 = new cvf::Effect;
    part->setEffect(eff2.p());

    model->addPart(part.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellSpheresPartMgr::createSphere(double radius, const cvf::Vec3d& pos)
{
    cvf::Vec3f posFloat(pos);

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::GeometryBuilderFaceList builder;
    cvf::GeometryUtils::createSphere(radius, 10, 10, &builder);

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

