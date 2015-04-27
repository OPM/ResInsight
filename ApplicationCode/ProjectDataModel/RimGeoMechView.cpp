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

    CAF_PDM_InitFieldNoDefault(&overlayInfoConfig,  "OverlayInfoConfig", "Info Box", "", "", "");
    overlayInfoConfig = new Rim3dOverlayInfoConfig();

    CAF_PDM_InitField(&name, "UserDescription", QString("View"), "Name", "", "", "");
    
    double defaultScaleFactor = 1.0;
    if (preferences) defaultScaleFactor = preferences->defaultScaleFactorZ;
    CAF_PDM_InitField(&scaleZ,          "GridZScale", defaultScaleFactor,         "Z Scale",          "", "Scales the scene in the Z direction", "");
    
    CAF_PDM_InitField(&showWindow, "ShowWindow", true, "Show 3D viewer", "", "", "");
    showWindow.setUiHidden(true);

    CAF_PDM_InitField(&cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "", "", "", "");

    caf::AppEnum<RimGeoMechView::MeshModeType> defaultMeshType = NO_MESH;
    if (preferences->defaultGridLines) defaultMeshType = FULL_MESH;
    CAF_PDM_InitField(&meshMode, "MeshMode", defaultMeshType, "Grid lines",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&surfaceMode, "SurfaceMode", "Grid surface",  "", "", "");

    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitField(&backgroundColor,     "ViewBackgroundColor",  defBackgColor, "Background", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::~RimGeoMechView(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGeoMechView::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateViewerWidget()
{
    if (showWindow())
    {
        bool isViewerCreated = false;
        if (!m_viewer)
        {
            QGLFormat glFormat;
            glFormat.setDirectRendering(RiaApplication::instance()->useShaders());

            m_viewer = new RiuViewer(glFormat, NULL);
            //m_viewer->setOwnerReservoirView(this);

            RiuMainWindow::instance()->addViewer(m_viewer);
            m_viewer->setMinNearPlaneDistance(10);
           

            m_viewer->removeAllColorLegends();
            m_viewer->addColorLegendToBottomLeftCorner(this->cellResult()->legendConfig->legend());

            if (RiaApplication::instance()->navigationPolicy() == RiaApplication::NAVIGATION_POLICY_CEETRON)
            {
                m_viewer->setNavigationPolicy(new caf::CeetronPlusNavigation);
            }
            else
            {
                m_viewer->setNavigationPolicy(new caf::CadNavigation);
            }

            m_viewer->enablePerfInfoHud(RiaApplication::instance()->showPerformanceInfo());

            //m_viewer->layoutWidget()->showMaximized();

            isViewerCreated = true;
        }

        RiuMainWindow::instance()->setActiveViewer(m_viewer);

        if (isViewerCreated) m_viewer->mainCamera()->setViewMatrix(cameraPosition);
        m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));

        m_viewer->update();
    }
    else
    {
        if (m_viewer)
        {
            if (m_viewer->layoutWidget()->parentWidget())
            {
                m_viewer->layoutWidget()->parentWidget()->hide();
            }
            else
            {
                m_viewer->layoutWidget()->hide(); 
            }
        }
    }

    updateViewerWidgetWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateViewerWidgetWindowTitle()
{
    if (m_viewer)
    {
        QString windowTitle;
        if (false)//m_reservoir.notNull())
        {
        //    windowTitle = QString("%1 - %2").arg(m_reservoir->caseUserDescription()).arg(name);
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
        cvf::ref<cvf::Transform>  scaleTransform = new cvf::Transform();
        scaleTransform->setLocalTransform(cvf::Mat4d::IDENTITY);

        cvf::ref<cvf::ModelBasicList> cvfModel =  new cvf::ModelBasicList;
        m_geoMechVizModel = new RivGeoMechPartMgr();
        m_geoMechVizModel->clearAndSetReservoir(m_geomechCase->geoMechData(), this);
        for (int femPartIdx = 0; femPartIdx < m_geomechCase->geoMechData()->femParts()->partCount(); ++femPartIdx)
        {
            cvf::ref<cvf::UByteArray> elmVisibility =  m_geoMechVizModel->cellVisibility(femPartIdx);
            m_geoMechVizModel->setTransform(scaleTransform.p());
            RivElmVisibilityCalculator::computeAllVisible(elmVisibility.p(),  m_geomechCase->geoMechData()->femParts()->part(femPartIdx));
            m_geoMechVizModel->setCellVisibility(femPartIdx, elmVisibility.p());
        }
        m_geoMechVizModel->appendGridPartsToModel(cvfModel.p());

        cvf::ref<cvf::Scene> scene = new cvf::Scene;
        scene->addModel(cvfModel.p());
        scene->updateBoundingBoxesRecursive();

        m_viewer->setMainScene(scene.p());
        m_viewer->zoomAll();
        m_viewer->update();
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
void RivElmVisibilityCalculator::computeAllVisible(cvf::UByteArray* elmVisibilities, const RigFemPart* femPart)
{
    elmVisibilities->resize(femPart->elementCount());
    elmVisibilities->setAll(true);
}
