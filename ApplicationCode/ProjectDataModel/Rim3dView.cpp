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

#include "Rim3dView.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaViewRedrawScheduler.h"

#include "RimEclipseCase.h"
#include "RimGridView.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuTimeStepChangedHandler.h"

#include "cafDisplayCoordTransform.h"
#include "cafFrameAnimationControl.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"
#include "cvfViewport.h"

#include <QDateTime>
#include <climits>


namespace caf {

template<>
void caf::AppEnum< Rim3dView::MeshModeType >::setUp()
{
    addItem(Rim3dView::FULL_MESH,      "FULL_MESH",       "All");
    addItem(Rim3dView::FAULTS_MESH,    "FAULTS_MESH",     "Faults only");
    addItem(Rim3dView::NO_MESH,        "NO_MESH",         "None");
    setDefault(Rim3dView::FULL_MESH);
}

template<>
void caf::AppEnum< Rim3dView::SurfaceModeType >::setUp()
{
    addItem(Rim3dView::SURFACE,              "SURFACE",             "All");
    addItem(Rim3dView::FAULTS,               "FAULTS",              "Faults only");
    addItem(Rim3dView::NO_SURFACE,           "NO_SURFACE",          "None");
    setDefault(Rim3dView::SURFACE);
}

} // End namespace caf


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(Rim3dView, "GenericView"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dView::Rim3dView(void)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);


    CAF_PDM_InitField(&name, "UserDescription", QString(""), "Name", "", "", "");

    CAF_PDM_InitField(&m_cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "", "", "", "");
    m_cameraPosition.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&m_cameraPointOfInterest, "CameraPointOfInterest", cvf::Vec3d::ZERO, "", "", "", "");
    m_cameraPointOfInterest.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isPerspectiveView, "PerspectiveProjection", true, "Perspective Projection", "", "", "");

    double defaultScaleFactor = preferences->defaultScaleFactorZ;
    CAF_PDM_InitField(&scaleZ, "GridZScale", defaultScaleFactor, "Z Scale", "", "Scales the scene in the Z direction", "");

    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitField(&m_backgroundColor, "ViewBackgroundColor", defBackgColor, "Background", "", "", "");

    CAF_PDM_InitField(&maximumFrameRate, "MaximumFrameRate", 10, "Maximum Frame Rate", "", "", "");
    maximumFrameRate.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&hasUserRequestedAnimation, "AnimationMode", false, "Animation Mode", "", "", "");
    hasUserRequestedAnimation.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_currentTimeStep, "CurrentTimeStep", 0, "Current Time Step", "", "", "");
    m_currentTimeStep.uiCapability()->setUiHidden(true);

    caf::AppEnum<Rim3dView::MeshModeType> defaultMeshType = NO_MESH;
    if (preferences->defaultGridLines) defaultMeshType = FULL_MESH;
    CAF_PDM_InitField(&meshMode, "MeshMode", defaultMeshType, "Grid Lines",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&surfaceMode, "SurfaceMode", "Grid Surface",  "", "", "");

    CAF_PDM_InitField(&showGridBox, "ShowGridBox", true, "Show Grid Box", "", "", "");

    CAF_PDM_InitField(&m_disableLighting, "DisableLighting", false, "Disable Results Lighting", "", "Disable light model for scalar result colors", "");

    m_crossSectionVizModel = new cvf::ModelBasicList;
    m_crossSectionVizModel->setName("CrossSectionModel");

    m_highlightVizModel = new cvf::ModelBasicList;
    m_highlightVizModel->setName("HighlightModel");

    m_wellPathPipeVizModel = new cvf::ModelBasicList;
    m_wellPathPipeVizModel->setName("WellPathPipeModel");

    this->setAs3DViewMdiWindow();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dView::~Rim3dView(void)
{
    removeMdiWindowFromMdiArea();

    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer* Rim3dView::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* Rim3dView::createViewWidget(QWidget* mainWindowParent)
{
    QGLFormat glFormat;
    glFormat.setDirectRendering(RiaApplication::instance()->useShaders());

    m_viewer = new RiuViewer(glFormat, NULL);
    m_viewer->setOwnerReservoirView(this);

    cvf::String xLabel;
    cvf::String yLabel;
    cvf::String zLabel;

    this->axisLabels(&xLabel, &yLabel, &zLabel);
    m_viewer->setAxisLabels(xLabel, yLabel, zLabel);

    return m_viewer->layoutWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateViewWidgetAfterCreation()
{
    m_viewer->setDefaultPerspectiveNearPlaneDistance(10);

    this->resetLegendsInViewer();

    m_viewer->updateNavigationPolicy();
    m_viewer->enablePerfInfoHud(RiaApplication::instance()->showPerformanceInfo());

    m_viewer->mainCamera()->setViewMatrix(m_cameraPosition);
    m_viewer->setPointOfInterest(m_cameraPointOfInterest());
    m_viewer->enableParallelProjection(!isPerspectiveView());

    m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));

    this->updateGridBoxData();
    this->updateAnnotationItems();
    this->createHighlightAndGridBoxDisplayModel();

    m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateMdiWindowTitle()
{
    if (m_viewer)
    {
        QString windowTitle;
        if (ownerCase())
        {
            windowTitle = QString("%1 - %2").arg(ownerCase()->caseUserDescription()).arg(name);
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
void Rim3dView::deleteViewWidget()
{
    if (m_viewer) 
    {
        m_viewer->deleteLater();
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup("Viewer");
    viewGroup->add(&name);
    viewGroup->add(&m_backgroundColor);
    viewGroup->add(&showGridBox);
    viewGroup->add(&isPerspectiveView);
    viewGroup->add(&m_disableLighting);

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Grid Appearance");
    gridGroup->add(&scaleZ);
    gridGroup->add(&meshMode);
    gridGroup->add(&surfaceMode);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage Rim3dView::snapshotWindowContent()
{
    if (m_viewer)
    {
        // Force update of scheduled display models before snapshotting
        RiaViewRedrawScheduler::instance()->slotUpdateScheduledDisplayModels();

        m_viewer->repaint();

        return m_viewer->snapshotImage();
    }

    return QImage();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::scheduleCreateDisplayModelAndRedraw()
{
    RiaViewRedrawScheduler::instance()->scheduleDisplayModelUpdateAndRedraw(this);
    if (this->isMasterView())
    {
        RimViewLinker* viewLinker = this->assosiatedViewLinker();
        if (viewLinker)
        {
            viewLinker->scheduleCreateDisplayModelAndRedrawForDependentViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setCurrentTimeStepAndUpdate(int frameIndex)
{
    setCurrentTimeStep(frameIndex);

    this->updateCurrentTimeStep();

    RimProject* project;
    firstAncestorOrThisOfTypeAsserted(project);
    project->mainPlotCollection()->updateCurrentTimeStepInPlots();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString Rim3dView::timeStepName(int frameIdx) const
{
    return this->ownerCase()->timeStepName(frameIdx);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setCurrentTimeStep(int frameIndex)
{
    const int oldTimeStep = m_currentTimeStep;

    m_currentTimeStep = frameIndex;
    clampCurrentTimestep();

    if (m_currentTimeStep != oldTimeStep)
    {
        RiuTimeStepChangedHandler::instance()->handleTimeStepChanged(this);
        this->onTimeStepChanged();
    }

    this->hasUserRequestedAnimation = true;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateCurrentTimeStepAndRedraw()
{
    this->updateCurrentTimeStep();

    RimProject* project;
    firstAncestorOrThisOfTypeAsserted(project);
    project->mainPlotCollection()->updateCurrentTimeStepInPlots();

    if (m_viewer) m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::createDisplayModelAndRedraw()
{
    if (m_viewer)
    {
        this->clampCurrentTimestep();

        createDisplayModel();
        createHighlightAndGridBoxDisplayModel();
        updateDisplayModelVisibility();

        if (m_cameraPosition().isIdentity())
        {
            setDefaultView();
            m_cameraPosition = m_viewer->mainCamera()->viewMatrix();
            m_cameraPointOfInterest = m_viewer->pointOfInterest();
        }
    }

    RiuMainWindow::instance()->refreshAnimationActions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setDefaultView()
{
    if (m_viewer)
    {
        m_viewer->setDefaultView();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::endAnimation()
{
    this->hasUserRequestedAnimation = false;
    this->updateStaticCellColors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* Rim3dView::wellPathCollection()
{
    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    CVF_ASSERT(proj && proj->activeOilField() && proj->activeOilField()->wellPathCollection());

    return proj->activeOilField()->wellPathCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setupBeforeSave()
{
    if (m_viewer)
    {
        hasUserRequestedAnimation = m_viewer->isAnimationActive(); // JJS: This is not conceptually correct. The variable is updated as we go, and store the user intentions. But I guess that in practice...
        m_cameraPosition = m_viewer->mainCamera()->viewMatrix();
        m_cameraPointOfInterest = m_viewer->pointOfInterest();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
// Surf: No Fault Surf
//  Mesh -------------
//    No F  F     G
// Fault F  F     G
//  Mesh G  G     G
//
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isGridVisualizationMode() const
{
    return (   this->surfaceMode() == SURFACE 
            || this->meshMode()    == FULL_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setMeshOnlyDrawstyle()
{
    if (isGridVisualizationMode())
    {
        meshMode.setValueWithFieldChanged(FULL_MESH);
    }
    else
    {
        meshMode.setValueWithFieldChanged(FAULTS_MESH);
    }

    surfaceMode.setValueWithFieldChanged(NO_SURFACE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setMeshSurfDrawstyle()
{
    if (isGridVisualizationMode())
    {
        surfaceMode.setValueWithFieldChanged(SURFACE);
        meshMode.setValueWithFieldChanged(FULL_MESH);
    }
    else
    {
        surfaceMode.setValueWithFieldChanged(FAULTS);
        meshMode.setValueWithFieldChanged(FAULTS_MESH);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setFaultMeshSurfDrawstyle()
{
    // Surf: No Fault Surf
    //  Mesh -------------
    //    No FF  FF    SF
    // Fault FF  FF    SF
    //  Mesh SF  SF    SF
    if (this->isGridVisualizationMode())
    {
        surfaceMode.setValueWithFieldChanged(SURFACE);
    }
    else
    {
        surfaceMode.setValueWithFieldChanged(FAULTS);
    }

    meshMode.setValueWithFieldChanged(FAULTS_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setSurfOnlyDrawstyle()
{
    if (isGridVisualizationMode())
    {
        surfaceMode.setValueWithFieldChanged(SURFACE);
    }
    else
    {
        surfaceMode.setValueWithFieldChanged(FAULTS);
    }

    meshMode.setValueWithFieldChanged(NO_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setSurfaceDrawstyle()
{
    if (surfaceMode() != NO_SURFACE) surfaceMode.setValueWithFieldChanged(SURFACE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::disableLighting(bool disable)
{
    m_disableLighting = disable;
    updateCurrentTimeStepAndRedraw();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isLightingDisabled() const
{
    return m_disableLighting();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &meshMode)
    {
        createDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
        RiuMainWindow::instance()->refreshAnimationActions();
    }
    else if (changedField == &isPerspectiveView)
    {
        if (m_viewer) m_viewer->enableParallelProjection(!isPerspectiveView());
    }
    else if (changedField == &scaleZ)
    {
        if (scaleZ < 1) scaleZ = 1;

        this->updateGridBoxData();

        if (m_viewer)
        {
            cvf::Vec3d poi = m_viewer->pointOfInterest();
            cvf::Vec3d eye, dir, up;
            eye = m_viewer->mainCamera()->position();
            dir = m_viewer->mainCamera()->direction();
            up  = m_viewer->mainCamera()->up();

            eye[2] = poi[2]*scaleZ()/this->scaleTransform()->worldTransform()(2, 2) + (eye[2] - poi[2]);
            poi[2] = poi[2]*scaleZ()/this->scaleTransform()->worldTransform()(2, 2);

            m_viewer->mainCamera()->setFromLookAt(eye, eye + dir, up);
            m_viewer->setPointOfInterest(poi);

            updateScaleTransform();
            createDisplayModelAndRedraw();

            m_viewer->update();


        }

        RiuMainWindow::instance()->updateScaleValue();
    }
    else if (changedField == &surfaceMode)
    {
        createDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
        RiuMainWindow::instance()->refreshAnimationActions();
    }
    else if (changedField == &showGridBox)
    {
        createHighlightAndGridBoxDisplayModelWithRedraw();
    }
    else if (changedField == &m_disableLighting)
    {
        createDisplayModel();
        RiuMainWindow::instance()->refreshDrawStyleActions();
        RiuMainWindow::instance()->refreshAnimationActions();
    }
    else if (changedField == &name)
    {
        updateMdiWindowTitle();

        if (viewController())
        {
            viewController()->updateDisplayNameAndIcon();
            viewController()->updateConnectedEditors();
        }
        else
        {
            if (isMasterView())
            {
                assosiatedViewLinker()->updateUiNameAndIcon();
                assosiatedViewLinker()->updateConnectedEditors();
            }
        }
    }
    else if (changedField == &m_currentTimeStep)
    {
        if (m_viewer)
        {
            m_viewer->update();


        }
    }
    else if (changedField == &m_backgroundColor)
    {
        if (m_viewer != nullptr)
        {
            m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));
        }
        updateGridBoxData();
        updateAnnotationItems();
    }
    else if (changedField == &maximumFrameRate)
    {
        // !! Use cvf::UNDEFINED_INT or something if we end up with frame rate 0?
        // !! Should be able to specify legal range for number properties
        if (m_viewer)
        {
            m_viewer->animationControl()->setTimeout(maximumFrameRate != 0 ? 1000/maximumFrameRate : INT_MAX);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::addWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                  const cvf::BoundingBox& wellPathClipBoundingBox)
{
    if (!this->ownerCase()) return;

    cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

    wellPathCollection()->appendStaticGeometryPartsToModel(wellPathModelBasicList,
                                                             this->ownerCase()->characteristicCellSize(),
                                                             wellPathClipBoundingBox,
                                                             transForm.p());

    wellPathModelBasicList->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::addDynamicWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, const cvf::BoundingBox& wellPathClipBoundingBox)
{
    if (!this->ownerCase()) return;

    cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

    QDateTime currentTimeStamp;
    std::vector<QDateTime> timeStamps = ownerCase()->timeStepDates();
    if (currentTimeStep() < static_cast<int>(timeStamps.size()))
    {
        currentTimeStamp = timeStamps[currentTimeStep()];
    }

    wellPathCollection()->appendDynamicGeometryPartsToModel(wellPathModelBasicList,
        currentTimeStamp,
        this->ownerCase()->characteristicCellSize(),
        wellPathClipBoundingBox,
        transForm.p());

    wellPathModelBasicList->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setScaleZAndUpdate(double scaleZ)
{
    this->scaleZ = scaleZ;
    updateScaleTransform();

    this->updateGridBoxData();

    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isMasterView() const
{
    RimViewLinker* viewLinker = this->assosiatedViewLinker();
    if (viewLinker && this == viewLinker->masterView())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateGridBoxData()
{
    if (m_viewer && ownerCase())
    {
        m_viewer->updateGridBoxData(scaleZ(), 
                                    ownerCase()->displayModelOffset(),
                                    backgroundColor(), 
                                    showActiveCellsOnly() ? ownerCase()->activeCellsBoundingBox() 
                                                          : ownerCase()->allCellsBoundingBox()
                                    );
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateAnnotationItems()
{
    if (m_viewer)
    {
        m_viewer->updateAnnotationItems();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::createHighlightAndGridBoxDisplayModelWithRedraw()
{
    createHighlightAndGridBoxDisplayModel();

    if (m_viewer)
    {
        m_viewer->update();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::createHighlightAndGridBoxDisplayModel()
{
    m_viewer->removeStaticModel(m_highlightVizModel.p());

    m_highlightVizModel->removeAllParts();

    cvf::Collection<cvf::Part> parts;
    createPartCollectionFromSelection(&parts);
    if (parts.size() > 0)
    {
        for (size_t i = 0; i < parts.size(); i++)
        {
            m_highlightVizModel->addPart(parts[i].p());
        }

        m_highlightVizModel->updateBoundingBoxesRecursive();
        m_viewer->addStaticModelOnce(m_highlightVizModel.p());
    }

    m_viewer->showGridBox(showGridBox());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dView::showActiveCellsOnly()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::zoomAll()
{
    if (m_viewer)
    {
        m_viewer->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<caf::DisplayCoordTransform> Rim3dView::displayCoordTransform() const
{
    cvf::ref<caf::DisplayCoordTransform> coordTrans = new caf::DisplayCoordTransform;

    cvf::Vec3d scale(1.0, 1.0, scaleZ);
    coordTrans->setScale(scale);

    RimCase* rimCase = ownerCase();
    if (rimCase)
    {
        coordTrans->setTranslation(rimCase->displayModelOffset());
    }

    return coordTrans;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* Rim3dView::viewWidget()
{
    if ( m_viewer ) return m_viewer->layoutWidget();
    else return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::forceShowWindowOn()
{
    m_showWindow.setValueWithFieldChanged(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::handleMdiWindowClosed()
{
    RimViewWindow::handleMdiWindowClosed();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dView::setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry)
{
    RimViewWindow::setMdiWindowGeometry(windowGeometry);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
