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

#include "RimView.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridCollection.h"
#include "RimIntersectionCollection.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimPropertyFilterCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafFrameAnimationControl.h"
#include "cafPdmObjectFactory.h"

#include "cvfCamera.h"
#include "cvfModel.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfViewport.h"

#include <limits.h>
#include "cvfTransform.h"


namespace caf {

template<>
void caf::AppEnum< RimView::MeshModeType >::setUp()
{
    addItem(RimView::FULL_MESH,      "FULL_MESH",       "All");
    addItem(RimView::FAULTS_MESH,    "FAULTS_MESH",     "Faults only");
    addItem(RimView::NO_MESH,        "NO_MESH",         "None");
    setDefault(RimView::FULL_MESH);
}

template<>
void caf::AppEnum< RimView::SurfaceModeType >::setUp()
{
    addItem(RimView::SURFACE,              "SURFACE",             "All");
    addItem(RimView::FAULTS,               "FAULTS",              "Faults only");
    addItem(RimView::NO_SURFACE,           "NO_SURFACE",          "None");
    setDefault(RimView::SURFACE);
}

} // End namespace caf


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimView, "GenericView"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView::RimView(void)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);


    CAF_PDM_InitField(&name, "UserDescription", QString(""), "Name", "", "", "");

    CAF_PDM_InitField(&cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "", "", "", "");
    cameraPosition.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&cameraPointOfInterest, "CameraPointOfInterest", cvf::Vec3d::ZERO, "", "", "", "");
    cameraPointOfInterest.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isPerspectiveView, "PerspectiveProjection", true, "Perspective Projection", "", "", "");

    double defaultScaleFactor = preferences->defaultScaleFactorZ;
    CAF_PDM_InitField(&scaleZ, "GridZScale", defaultScaleFactor, "Z Scale", "", "Scales the scene in the Z direction", "");

    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitField(&backgroundColor, "ViewBackgroundColor", defBackgColor, "Background", "", "", "");

    CAF_PDM_InitField(&maximumFrameRate, "MaximumFrameRate", 10, "Maximum frame rate", "", "", "");
    maximumFrameRate.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&hasUserRequestedAnimation, "AnimationMode", false, "Animation Mode", "", "", "");
    hasUserRequestedAnimation.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_currentTimeStep, "CurrentTimeStep", 0, "Current Time Step", "", "", "");
    m_currentTimeStep.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_overlayInfoConfig, "OverlayInfoConfig", "Info Box", "", "", "");
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView(this);
    m_overlayInfoConfig.uiCapability()->setUiHidden(true);

    caf::AppEnum<RimView::MeshModeType> defaultMeshType = NO_MESH;
    if (preferences->defaultGridLines) defaultMeshType = FULL_MESH;
    CAF_PDM_InitField(&meshMode, "MeshMode", defaultMeshType, "Grid lines",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&surfaceMode, "SurfaceMode", "Grid surface",  "", "", "");

    CAF_PDM_InitField(&showGridBox, "ShowGridBox", true, "Show Grid Box", "", "", "");

    CAF_PDM_InitField(&m_disableLighting, "DisableLighting", false, "Disable Results Lighting", "", "Disable light model for scalar result colors", "");


    CAF_PDM_InitFieldNoDefault(&m_rangeFilterCollection, "RangeFilters", "Range Filters", "", "", "");
    m_rangeFilterCollection.uiCapability()->setUiHidden(true);
    m_rangeFilterCollection = new RimCellRangeFilterCollection();

    CAF_PDM_InitFieldNoDefault(&m_overrideRangeFilterCollection, "RangeFiltersControlled", "Range Filters (controlled)", "", "", "");
    m_overrideRangeFilterCollection.uiCapability()->setUiHidden(true);
    m_overrideRangeFilterCollection.xmlCapability()->setIOWritable(false);
    m_overrideRangeFilterCollection.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&crossSectionCollection, "CrossSections", "Intersections", "", "", "");
    crossSectionCollection.uiCapability()->setUiHidden(true);
    crossSectionCollection = new RimIntersectionCollection();

    CAF_PDM_InitFieldNoDefault(&m_gridCollection, "GridCollection", "GridCollection", "", "", "");
    m_gridCollection.uiCapability()->setUiHidden(true);
    m_gridCollection = new RimGridCollection();

    m_previousGridModeMeshLinesWasFaults = false;

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
RimView::~RimView(void)
{
    RimProject* proj = RiaApplication::instance()->project();

    if (proj && this->isMasterView())
    {
        delete proj->viewLinkerCollection->viewLinker();
        proj->viewLinkerCollection->viewLinker = NULL;

        proj->uiCapability()->updateConnectedEditors();
    }

    RimViewController* vController = this->viewController();
    if (proj && vController)
    {
        vController->setManagedView(NULL);
        vController->ownerViewLinker()->removeViewController(vController);
        delete vController;

        proj->uiCapability()->updateConnectedEditors();
    }

    delete this->m_overlayInfoConfig();

    removeMdiWindowFromMdiArea();

    deleteViewWidget();

    delete m_rangeFilterCollection;
    delete m_overrideRangeFilterCollection;
    delete crossSectionCollection;
    delete m_gridCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer* RimView::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimView::createViewWidget(QWidget* mainWindowParent)
{
    QGLFormat glFormat;
    glFormat.setDirectRendering(RiaApplication::instance()->useShaders());

    m_viewer = new RiuViewer(glFormat, NULL);
    m_viewer->setOwnerReservoirView(this);

    return m_viewer->layoutWidget();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateViewWidgetAfterCreation()
{
    m_viewer->setDefaultPerspectiveNearPlaneDistance(10);

    this->resetLegendsInViewer();

    m_viewer->updateNavigationPolicy();
    m_viewer->enablePerfInfoHud(RiaApplication::instance()->showPerformanceInfo());

    m_viewer->mainCamera()->setViewMatrix(cameraPosition);
    m_viewer->setPointOfInterest(cameraPointOfInterest());
    m_viewer->enableParallelProjection(!isPerspectiveView());

    m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));

    this->updateGridBoxData();
    this->createHighlightAndGridBoxDisplayModel();

    m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateMdiWindowTitle()
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
void RimView::deleteViewWidget()
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
void RimView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup("Viewer");
    viewGroup->add(&name);
    viewGroup->add(&backgroundColor);
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
QImage RimView::snapshotWindowContent()
{
    if (m_viewer)
    {
        // Force update of scheduled display models before snapshotting
        RiaApplication::instance()->slotUpdateScheduledDisplayModels();

        m_viewer->repaint();

        return m_viewer->snapshotImage();
    }

    return QImage();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::scheduleCreateDisplayModelAndRedraw()
{
    RiaApplication::instance()->scheduleDisplayModelUpdateAndRedraw(this);
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
void RimView::setCurrentTimeStepAndUpdate(int frameIndex)
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
void RimView::setCurrentTimeStep(int frameIndex)
{
    m_currentTimeStep = frameIndex;
    clampCurrentTimestep();

    this->hasUserRequestedAnimation = true;
    if (this->propertyFilterCollection() && this->propertyFilterCollection()->hasActiveDynamicFilters())
    {  
        m_currentReservoirCellVisibility = NULL; 
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateCurrentTimeStepAndRedraw()
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
void RimView::createDisplayModelAndRedraw()
{
    if (m_viewer)
    {
        this->clampCurrentTimestep();

        createDisplayModel();
        createHighlightAndGridBoxDisplayModel();
        updateDisplayModelVisibility();

        if (cameraPosition().isIdentity())
        {
            setDefaultView();
            cameraPosition = m_viewer->mainCamera()->viewMatrix();
            cameraPointOfInterest = m_viewer->pointOfInterest();
        }
    }

    RiuMainWindow::instance()->refreshAnimationActions();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setDefaultView()
{
    if (m_viewer)
    {
        m_viewer->setDefaultView();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::endAnimation()
{
    this->hasUserRequestedAnimation = false;
    this->updateStaticCellColors();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RimView::wellPathsPartManager()
{
    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    CVF_ASSERT(proj && proj->activeOilField() && proj->activeOilField()->wellPathCollection());

    return proj->activeOilField()->wellPathCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setupBeforeSave()
{
    if (m_viewer)
    {
        hasUserRequestedAnimation = m_viewer->isAnimationActive(); // JJS: This is not conceptually correct. The variable is updated as we go, and store the user intentions. But I guess that in practice...
        cameraPosition = m_viewer->mainCamera()->viewMatrix();
        cameraPointOfInterest = m_viewer->pointOfInterest();
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
bool RimView::isGridVisualizationMode() const
{
    return (   this->surfaceMode() == SURFACE 
            || this->meshMode()    == FULL_MESH);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setMeshOnlyDrawstyle()
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
void RimView::setMeshSurfDrawstyle()
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
void RimView::setFaultMeshSurfDrawstyle()
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
void RimView::setSurfOnlyDrawstyle()
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
void RimView::showGridCells(bool enableGridCells)
{
    if (!enableGridCells)
    {
        m_previousGridModeMeshLinesWasFaults = meshMode() == FAULTS_MESH;
        if (surfaceMode() != NO_SURFACE) surfaceMode.setValueWithFieldChanged(FAULTS);
        if (meshMode() != NO_MESH) meshMode.setValueWithFieldChanged(FAULTS_MESH);
    }
    else
    {
        if (surfaceMode() != NO_SURFACE) surfaceMode.setValueWithFieldChanged(SURFACE);
        if (meshMode() != NO_MESH) meshMode.setValueWithFieldChanged(m_previousGridModeMeshLinesWasFaults ? FAULTS_MESH : FULL_MESH);
    }

    m_gridCollection->isActive = enableGridCells;
    m_gridCollection->updateConnectedEditors();
    m_gridCollection->updateUiIconFromState(enableGridCells);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setSurfaceDrawstyle()
{
    if (surfaceMode() != NO_SURFACE) surfaceMode.setValueWithFieldChanged(SURFACE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::disableLighting(bool disable)
{
    m_disableLighting = disable;
    updateCurrentTimeStepAndRedraw();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimView::isLightingDisabled() const
{
    return m_disableLighting();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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
        
        // Regenerate well paths
        RimOilField* oilFields = RiaApplication::instance()->project() ? RiaApplication::instance()->project()->activeOilField() : NULL;
        RimWellPathCollection* wellPathCollection = (oilFields) ? oilFields->wellPathCollection() : NULL;
        
        wellPathsPartManager()->scheduleGeometryRegen(); 

        crossSectionCollection->updateIntersectionBoxGeometry();

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

            RimViewLinker* viewLinker = this->assosiatedViewLinker();
            if (viewLinker)
            {
                viewLinker->updateScaleZ(this, scaleZ);
                viewLinker->updateCamera(this);
            }
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

            RimViewLinker* viewLinker = this->assosiatedViewLinker();
            if (viewLinker)
            {
                viewLinker->updateTimeStep(this, m_currentTimeStep);
            }
        }
    }
    else if (changedField == &backgroundColor)
    {
        if (m_viewer != nullptr)
        {
            m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));
        }
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
void RimView::addWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                  const cvf::BoundingBox& wellPathClipBoundingBox)
{
    if (!this->ownerCase()) return;

    cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

    wellPathsPartManager()->appendStaticGeometryPartsToModel(wellPathModelBasicList,
        this->ownerCase()->characteristicCellSize(),
        wellPathClipBoundingBox,
        transForm.p());

    wellPathModelBasicList->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::addDynamicWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, const cvf::BoundingBox& wellPathClipBoundingBox)
{
    if (!this->ownerCase()) return;

    cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

    QDateTime currentTimeStamp;
    std::vector<QDateTime> timeStamps = ownerCase()->timeStepDates();
    if (currentTimeStep() < timeStamps.size())
    {
        currentTimeStamp = timeStamps[currentTimeStep()];
    }

    wellPathsPartManager()->appendDynamicGeometryPartsToModel(wellPathModelBasicList,
        currentTimeStamp,
        this->ownerCase()->characteristicCellSize(),
        wellPathClipBoundingBox,
        transForm.p());

    wellPathModelBasicList->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection* RimView::rangeFilterCollection()
{
    if (this->viewController() && this->viewController()->isRangeFiltersControlled() && m_overrideRangeFilterCollection)
    {
        return m_overrideRangeFilterCollection;
    }
    else
    {
        return m_rangeFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimCellRangeFilterCollection* RimView::rangeFilterCollection() const
{
    if (this->viewController() && this->viewController()->isRangeFiltersControlled() && m_overrideRangeFilterCollection)
    {
        return m_overrideRangeFilterCollection;
    }
    else
    {
        return m_rangeFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setOverrideRangeFilterCollection(RimCellRangeFilterCollection* rfc)
{
    if (m_overrideRangeFilterCollection()) delete m_overrideRangeFilterCollection();

    m_overrideRangeFilterCollection = rfc;
    this->scheduleGeometryRegen(RANGE_FILTERED);
    this->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setScaleZAndUpdate(double scaleZ)
{
    this->scaleZ = scaleZ;
    updateScaleTransform();

    this->updateGridBoxData();

    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewController* RimView::viewController() const
{
    RimViewController* viewController = NULL;
    std::vector<caf::PdmObjectHandle*> reffingObjs;

    this->objectsWithReferringPtrFields(reffingObjs);
    for (size_t i = 0; i < reffingObjs.size(); ++i)
    {
        viewController = dynamic_cast<RimViewController*>(reffingObjs[i]);
        if (viewController) break;
    }

    return viewController;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimView::viewLinkerIfMasterView() const
{
    RimViewLinker* viewLinker = NULL;
    std::vector<caf::PdmObjectHandle*> reffingObjs;

    this->objectsWithReferringPtrFields(reffingObjs);

    for (size_t i = 0; i < reffingObjs.size(); ++i)
    {
        viewLinker = dynamic_cast<RimViewLinker*>(reffingObjs[i]);
        if (viewLinker) break;
    }

    return viewLinker;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimView::assosiatedViewLinker() const
{
    RimViewLinker* viewLinker = this->viewLinkerIfMasterView();
    if (!viewLinker)
    {
        RimViewController* viewController = this->viewController();
        if (viewController)
        {
            viewLinker = viewController->ownerViewLinker();
        }
    }

    return viewLinker;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimView::currentTotalCellVisibility()
{
    if (m_currentReservoirCellVisibility.isNull())
    {
        m_currentReservoirCellVisibility = new cvf::UByteArray;
        this->calculateCurrentTotalCellVisibility(m_currentReservoirCellVisibility.p());
    }

    return m_currentReservoirCellVisibility;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimView::isMasterView() const
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
bool RimView::hasOverridenRangeFilterCollection()
{
    return m_overrideRangeFilterCollection() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::replaceRangeFilterCollectionWithOverride()
{
    RimCellRangeFilterCollection* overrideRfc = m_overrideRangeFilterCollection;
    CVF_ASSERT(overrideRfc);

    RimCellRangeFilterCollection* currentRfc = m_rangeFilterCollection;
    if (currentRfc)
    {
        delete currentRfc;
    }

    // Must call removeChildObject() to make sure the object has no parent
    // No parent is required when assigning a object into a field
    m_overrideRangeFilterCollection.removeChildObject(overrideRfc);

    m_rangeFilterCollection = overrideRfc;

    this->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::removeModelByName(cvf::Scene* scene, const cvf::String& modelName)
{
    std::vector<cvf::Model*> modelsToBeRemoved;
    for (cvf::uint i = 0; i < scene->modelCount(); i++)
    {
        if (scene->model(i)->name() == modelName)
        {
            modelsToBeRemoved.push_back(scene->model(i));
        }
    }

    for (size_t i = 0; i < modelsToBeRemoved.size(); i++)
    {
        scene->removeModel(modelsToBeRemoved[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateGridBoxData()
{
    if (m_viewer)
    {
        m_viewer->updateGridBoxData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::createHighlightAndGridBoxDisplayModelWithRedraw()
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
void RimView::createHighlightAndGridBoxDisplayModel()
{
    m_viewer->removeStaticModel(m_highlightVizModel.p());
    m_viewer->removeStaticModel(m_viewer->gridBoxModel());

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

    if (showGridBox)
    {
        m_viewer->addStaticModelOnce(m_viewer->gridBoxModel());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimView::showActiveCellsOnly()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::selectOverlayInfoConfig()
{
    RiuMainWindow::instance()->selectAsCurrentItem(m_overlayInfoConfig);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::zoomAll()
{
    if (m_viewer)
    {
        m_viewer->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<caf::DisplayCoordTransform> RimView::displayCoordTransform()
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
QWidget* RimView::viewWidget()
{
    if ( m_viewer ) return m_viewer->layoutWidget();
    else return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::forceShowWindowOn()
{
    m_showWindow.setValueWithFieldChanged(true);
}

