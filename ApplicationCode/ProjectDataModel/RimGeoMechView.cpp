/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGeoMechView.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RimGeoMechResultSlot.h"

#include "RiuMainWindow.h"
#include "cafCeetronPlusNavigation.h"
#include "cafCadNavigation.h"
#include "RimLegendConfig.h"
#include "cvfOverlayScalarMapperLegend.h"

#include "RimGeoMechCase.h"
#include "cvfPart.h"
#include "cvfViewport.h"
#include "cvfModelBasicList.h"
#include "cvfScene.h"
#include "RimReservoirView.h"
#include "RiuViewer.h"
#include "RivGeoMechPartMgr.h"
#include "RigGeoMechCaseData.h"
#include "cvfqtUtils.h"


namespace caf {

template<>
void caf::AppEnum< RimGeoMechView::MeshModeType >::setUp()
{
    addItem(RimGeoMechView::FULL_MESH,      "FULL_MESH",       "All");
    addItem(RimGeoMechView::FAULTS_MESH,    "FAULTS_MESH",      "Faults only");
    addItem(RimGeoMechView::NO_MESH,        "NO_MESH",        "None");
    setDefault(RimGeoMechView::FULL_MESH);
}

template<>
void caf::AppEnum< RimGeoMechView::SurfaceModeType >::setUp()
{
    addItem(RimGeoMechView::SURFACE,              "SURFACE",             "All");
    addItem(RimGeoMechView::FAULTS,               "FAULTS",              "Faults only");
    addItem(RimGeoMechView::NO_SURFACE,           "NO_SURFACE",          "None");
    setDefault(RimGeoMechView::SURFACE);
}

} // End namespace caf





CAF_PDM_SOURCE_INIT(RimGeoMechView, "GeoMechView");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::RimGeoMechView(void)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);

    CAF_PDM_InitObject("Geomechanical View", ":/ReservoirView.png", "", "");

    CAF_PDM_InitFieldNoDefault(&cellResult, "GridCellResult", "Color Result", ":/CellResult.png", "", "");
    cellResult = new RimGeoMechResultSlot();
 
    caf::AppEnum<RimGeoMechView::MeshModeType> defaultMeshType = NO_MESH;
    if (preferences->defaultGridLines) defaultMeshType = FULL_MESH;
    CAF_PDM_InitField(&meshMode, "MeshMode", defaultMeshType, "Grid lines",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&surfaceMode, "SurfaceMode", "Grid surface",  "", "", "");

    this->cellResult()->setReservoirView(this);
    this->cellResult()->legendConfig()->setPosition(cvf::Vec2ui(10, 120));
    this->cellResult()->legendConfig()->setReservoirView(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::~RimGeoMechView(void)
{
    m_geomechCase = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateViewerWidgetWindowTitle()
{
    if (m_viewer)
    {
        QString windowTitle;
        if (m_geomechCase.notNull())
        {
            windowTitle = QString("%1 - %2").arg(m_geomechCase->caseUserDescription()).arg(name);
        }
        else
        {
            windowTitle = name;
        }

        m_viewer->layoutWidget()->setWindowTitle(windowTitle);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::loadDataAndUpdate()
{
    if (m_geomechCase)
    {
        m_geomechCase->openGeoMechCase();

    }
    updateViewerWidget();

    createDisplayModelAndRedraw();

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::createDisplayModelAndRedraw()
{
    if (m_viewer && m_geomechCase)
    {
        int partCount = 0;
        if (m_geomechCase->geoMechData() && m_geomechCase->geoMechData()->femParts())
        {
            partCount = m_geomechCase->geoMechData()->femParts()->partCount();
        }

        if (partCount >= 0)
        {
            cvf::ref<cvf::Transform>  scaleTransform = new cvf::Transform();
            scaleTransform->setLocalTransform(cvf::Mat4d::IDENTITY);

            cvf::ref<cvf::ModelBasicList> cvfModel =  new cvf::ModelBasicList;
            m_geoMechVizModel = new RivGeoMechPartMgr();
            m_geoMechVizModel->clearAndSetReservoir(m_geomechCase->geoMechData(), this);

            for (int femPartIdx = 0; femPartIdx < partCount; ++femPartIdx)
            {
                cvf::ref<cvf::UByteArray> elmVisibility =  m_geoMechVizModel->cellVisibility(femPartIdx);
                m_geoMechVizModel->setTransform(scaleTransform.p());
                RivElmVisibilityCalculator::computeAllVisible(elmVisibility.p(), m_geomechCase->geoMechData()->femParts()->part(femPartIdx));
                m_geoMechVizModel->setCellVisibility(femPartIdx, elmVisibility.p());
            }

            m_geoMechVizModel->updateCellColor(cvf::Color4f(cvf::Color3f::ORANGE));
            if (cellResult()->resultFieldName() != "")
            {
                m_geoMechVizModel->updateCellResultColor(m_currentTimeStep(), this->cellResult());
            }
            m_geoMechVizModel->appendGridPartsToModel(cvfModel.p());

            cvf::ref<cvf::Scene> scene = new cvf::Scene;
            scene->addModel(cvfModel.p());
            scene->updateBoundingBoxesRecursive();

            m_viewer->setMainScene(scene.p());
            m_viewer->zoomAll();
            m_viewer->update();

            updateLegends();
            overlayInfoConfig()->update3DInfo();
        }
    }
    RiuMainWindow::instance()->refreshAnimationActions(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::setGeoMechCase(RimGeoMechCase* gmCase)
{
    m_geomechCase = gmCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::resetLegendsInViewer()
{
    this->cellResult()->legendConfig->recreateLegend();

    m_viewer->removeAllColorLegends();
    m_viewer->addColorLegendToBottomLeftCorner(this->cellResult()->legendConfig->legend());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateLegends()
{
    if (m_viewer)
    {
        m_viewer->removeAllColorLegends();
    }

    if (!m_geomechCase || !m_viewer || !m_geomechCase->geoMechData() )
    {
        return;
    }

    RigGeoMechCaseData* gmCase = m_geomechCase->geoMechData();
    CVF_ASSERT(gmCase);


    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
#if 0

    RigCaseCellResultsData* results = gmCase->results(porosityModel);
    CVF_ASSERT(results);

    if (cellResult->hasDynamicResult())
    {
        cellResultsData->minMaxCellScalarValues(cellResult->scalarResultIndex(), m_currentTimeStep, localMin, localMax);
        cellResultsData->posNegClosestToZero(cellResult->scalarResultIndex(), m_currentTimeStep, localPosClosestToZero, localNegClosestToZero);
    }
    else
    {
        localMin = globalMin;
        localMax = globalMax;

        localPosClosestToZero = globalPosClosestToZero;
        localNegClosestToZero = globalNegClosestToZero;
    }

    cellResult->legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
    cellResult->legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);
#endif
    caf::AppEnum<RimGeoMechResultSlot::ResultPositionEnum> resPosType = cellResult->resultPositionType();
    QString fieldName = cellResult->resultFieldName();
    QString compName = cellResult->resultComponentName();

    m_viewer->addColorLegendToBottomLeftCorner(cellResult->legendConfig->legend());

    cellResult->legendConfig->legend()->setTitle(cvfqt::Utils::toString( resPosType.text() + "\n" + fieldName + " " + compName));



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechView::geoMechCase()
{
    return m_geomechCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivElmVisibilityCalculator::computeAllVisible(cvf::UByteArray* elmVisibilities, const RigFemPart* femPart)
{
    elmVisibilities->resize(femPart->elementCount());
    elmVisibilities->setAll(true);
}
