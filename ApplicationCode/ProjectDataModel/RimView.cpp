#include "RimView.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigCaseData.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCrossSectionCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimPropertyFilterCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "RivWellPathCollectionPartMgr.h"

#include "cafFrameAnimationControl.h"
#include "cafPdmObjectFactory.h"
#include "cvfCamera.h"
#include "cvfModel.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfViewport.h"

#include <limits.h>


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

    CAF_PDM_InitField(&showWindow, "ShowWindow", true, "Show 3D viewer", "", "", "");
    showWindow.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "", "", "", "");
    cameraPosition.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&isPerspectiveView, "PerspectiveProjection", true, "Perspective Projection", "", "", "");
    isPerspectiveView.uiCapability()->setUiHidden(true); // For now as this is experimental

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

    CAF_PDM_InitFieldNoDefault(&windowGeometry, "WindowGeometry", "", "", "", "");
    windowGeometry.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_rangeFilterCollection, "RangeFilters", "Range Filters", "", "", "");
    m_rangeFilterCollection.uiCapability()->setUiHidden(true);
    m_rangeFilterCollection = new RimCellRangeFilterCollection();

    CAF_PDM_InitFieldNoDefault(&m_overrideRangeFilterCollection, "RangeFiltersControlled", "Range Filters (controlled)", "", "", "");
    m_overrideRangeFilterCollection.uiCapability()->setUiHidden(true);
    m_overrideRangeFilterCollection.xmlCapability()->setIOWritable(false);
    m_overrideRangeFilterCollection.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&crossSectionCollection, "CrossSections", "Intersections", "", "", "");
    crossSectionCollection.uiCapability()->setUiHidden(true);
    crossSectionCollection = new RimCrossSectionCollection();

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

    if (m_viewer)
    {
        RiuMainWindow::instance()->removeViewer(m_viewer->layoutWidget());
    }

    delete m_viewer;

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
void RimView::updateViewerWidget()
{
    if (showWindow())
    {
        bool isViewerCreated = false;
        if (!m_viewer)
        {
            QGLFormat glFormat;
            glFormat.setDirectRendering(RiaApplication::instance()->useShaders());

            m_viewer = new RiuViewer(glFormat, NULL);
            m_viewer->setOwnerReservoirView(this);

            RiuMainWindow::instance()->addViewer(m_viewer->layoutWidget(), windowGeometry());
            m_viewer->setMinNearPlaneDistance(10);
           
            this->resetLegendsInViewer();

            m_viewer->updateNavigationPolicy();
            m_viewer->enablePerfInfoHud(RiaApplication::instance()->showPerformanceInfo());

            isViewerCreated = true;
        }

        RiuMainWindow::instance()->setActiveViewer(m_viewer->layoutWidget());

        if (isViewerCreated) m_viewer->mainCamera()->setViewMatrix(cameraPosition);
        m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));

        this->updateGridBoxData();

        m_viewer->update();
    }
    else
    {
        if (m_viewer && m_viewer->layoutWidget())
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
void RimView::setCurrentTimeStep(int frameIndex)
{
    m_currentTimeStep = frameIndex;
    clampCurrentTimestep();

    this->hasUserRequestedAnimation = true;
    if (this->propertyFilterCollection() && this->propertyFilterCollection()->hasActiveDynamicFilters())
    {  
        m_currentReservoirCellVisibility = NULL; 
    }
    this->updateCurrentTimeStep();
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateCurrentTimeStepAndRedraw()
{
    this->updateCurrentTimeStep();
    
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
void RimView::setupBeforeSave()
{
    if (m_viewer)
    {
        hasUserRequestedAnimation = m_viewer->isAnimationActive(); // JJS: This is not conceptually correct. The variable is updated as we go, and store the user intentions. But I guess that in practice...
        cameraPosition = m_viewer->mainCamera()->viewMatrix();

        windowGeometry = RiuMainWindow::instance()->windowGeometryForViewer(m_viewer->layoutWidget());
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
        meshMode.uiCapability()->setValueFromUi(FULL_MESH);
    }
    else
    {
        meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
    }

    surfaceMode.uiCapability()->setValueFromUi(NO_SURFACE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setMeshSurfDrawstyle()
{
    if (isGridVisualizationMode())
    {
        surfaceMode.uiCapability()->setValueFromUi(SURFACE);
        meshMode.uiCapability()->setValueFromUi(FULL_MESH);
    }
    else
    {
        surfaceMode.uiCapability()->setValueFromUi(FAULTS);
        meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
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
        surfaceMode.uiCapability()->setValueFromUi(SURFACE);
    }
    else
    {
        surfaceMode.uiCapability()->setValueFromUi(FAULTS);
    }

    meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setSurfOnlyDrawstyle()
{
    if (isGridVisualizationMode())
    {
        surfaceMode.uiCapability()->setValueFromUi(SURFACE);
    }
    else
    {
        surfaceMode.uiCapability()->setValueFromUi(FAULTS);
    }

    meshMode.uiCapability()->setValueFromUi(NO_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::showGridCells(bool enableGridCells)
{
    if (!enableGridCells)
    {
        m_previousGridModeMeshLinesWasFaults = meshMode() == FAULTS_MESH;
        if (surfaceMode() != NO_SURFACE) surfaceMode.uiCapability()->setValueFromUi(FAULTS);
        if (meshMode() != NO_MESH) meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
    }
    else
    {
        if (surfaceMode() != NO_SURFACE) surfaceMode.uiCapability()->setValueFromUi(SURFACE);
        if (meshMode() != NO_MESH) meshMode.uiCapability()->setValueFromUi(m_previousGridModeMeshLinesWasFaults ? FAULTS_MESH : FULL_MESH);
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
    if (surfaceMode() != NO_SURFACE) surfaceMode.uiCapability()->setValueFromUi(SURFACE);
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
        if (wellPathCollection) wellPathCollection->wellPathCollectionPartMgr()->scheduleGeometryRegen();

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
        updateViewerWidgetWindowTitle();

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
        if (viewer() != NULL)
        {
            updateViewerWidget();
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
                                  const cvf::Vec3d& displayModelOffset,  
                                  double characteristicCellSize, 
                                  const cvf::BoundingBox& wellPathClipBoundingBox, 
                                  cvf::Transform* scaleTransform)
{
    RimOilField* oilFields =                                    RiaApplication::instance()->project()   ? RiaApplication::instance()->project()->activeOilField() : NULL;
    RimWellPathCollection* wellPathCollection =                 oilFields                               ? oilFields->wellPathCollection() : NULL;
    RivWellPathCollectionPartMgr* wellPathCollectionPartMgr =   wellPathCollection                      ? wellPathCollection->wellPathCollectionPartMgr() : NULL;

    if (wellPathCollectionPartMgr)
    {
        wellPathCollectionPartMgr->appendStaticGeometryPartsToModel(wellPathModelBasicList, 
                                                                    displayModelOffset,
                                                                    scaleTransform, 
                                                                    characteristicCellSize, 
                                                                    wellPathClipBoundingBox);
    }

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

