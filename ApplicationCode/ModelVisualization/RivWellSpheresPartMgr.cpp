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

#include "RiaApplication.h"

#include "RigMainGrid.h"
#include "RigSimWellData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSimWellInView.h"

#include "RiuViewer.h"

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
#include "cvfOpenGLResourceManager.h"
#include "cvfShaderProgram.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellSpheresPartMgr::RivWellSpheresPartMgr(RimEclipseView* reservoirView, RimSimWellInView* well)
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
    if (!m_rimReservoirView->eclipseCase()->eclipseCaseData()) return;   
    
    const RigMainGrid* mainGrid = m_rimReservoirView->mainGrid();
    CVF_ASSERT(mainGrid);

    RigSimWellData* rigWellResult = m_rimWell->simWellData();
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

    cvf::ref<cvf::Part> part = createPart(centerColorPairs, wellResultFrame.m_isOpen);

    model->addPart(part.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivWellSpheresPartMgr::createPart(std::vector<std::pair<cvf::Vec3f, cvf::Color3f> >& centerColorPairs, bool isWellOpen)
{
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

    cvf::ref<cvf::DrawableVectors> vectorDrawable;
    if (RiaApplication::instance()->useShaders())
    {
        // NOTE: Drawable vectors must be rendered using shaders when the rest of the application is rendered using shaders
        // Drawing vectors using fixed function when rest of the application uses shaders causes visual artifacts
        vectorDrawable = new cvf::DrawableVectors("u_transformationMatrix", "u_color");
    }
    else
    {
        vectorDrawable = new cvf::DrawableVectors();
    }

    vectorDrawable->setVectors(vertices.p(), vecRes.p());
    vectorDrawable->setColors(colors.p());

    cvf::GeometryBuilderTriangles builder;
    double characteristicCellSize = m_rimReservoirView->mainGrid()->characteristicIJCellSize();
    double cellRadius = m_rimReservoirView->wellCollection()->spheresScaleFactor() * characteristicCellSize;

    if (isWellOpen)
    {
        // Increase radius to make sure open connection are slightly larger than closed connections
        cellRadius = 1.1 * cellRadius;
    }

    cvf::GeometryUtils::createSphere(cellRadius, 15, 15, &builder);

    vectorDrawable->setGlyph(builder.trianglesUShort().p(), builder.vertices().p());

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(vectorDrawable.p());

    cvf::ref<cvf::Effect> eff = new cvf::Effect;
    if (RiaApplication::instance()->useShaders())
    {
        if (m_rimReservoirView->viewer())
        {
            cvf::ref<cvf::OpenGLContext> oglContext = m_rimReservoirView->viewer()->cvfOpenGLContext();
            cvf::OpenGLResourceManager* resourceManager = oglContext->resourceManager();
            cvf::ref<cvf::ShaderProgram> vectorProgram = resourceManager->getLinkedVectorDrawerShaderProgram(oglContext.p());

            eff->setShaderProgram(vectorProgram.p());
        }
    }

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

    RimSimWellInViewCollection* wellColl = nullptr;
    if (m_rimWell)
    {
        m_rimWell->firstAncestorOrThisOfType(wellColl);
    }

    if (wellColl)
    {
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
            }
        }
    }

    return cellColor;
}

