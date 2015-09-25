#include "RimView.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigCaseData.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimViewLink.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimViewLinker.h"
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
    addItem(RimView::FAULTS_MESH,    "FAULTS_MESH",      "Faults only");
    addItem(RimView::NO_MESH,        "NO_MESH",        "None");
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

    double defaultScaleFactor = 1.0;
    if (preferences) defaultScaleFactor = preferences->defaultScaleFactorZ;
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

    CAF_PDM_InitField(&m_disableLighting, "DisableLighting", false, "Disable Results Lighting", "", "Disable light model for scalar result colors", "");

    CAF_PDM_InitFieldNoDefault(&windowGeometry, "WindowGeometry", "", "", "", "");
    windowGeometry.uiCapability()->setUiHidden(true);

    m_previousGridModeMeshLinesWasFaults = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView::~RimView(void)
{
    delete this->m_overlayInfoConfig();

    if (m_viewer)
    {
        RiuMainWindow::instance()->removeViewer(m_viewer->layoutWidget());
    }

    delete m_viewer;
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
void RimView::setShowFaultsOnly(bool showFaults)
{
    if (showFaults)
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
    }
    else if (changedField == &scaleZ)
    {
        if (scaleZ < 1) scaleZ = 1;

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
    }
    else if (changedField == &m_disableLighting)
    {
        createDisplayModel();
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
    else if (changedField == &name)
    {
        updateViewerWidgetWindowTitle();
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
    if (m_overrideRangeFilterCollection)
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
    if (m_overrideRangeFilterCollection)
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
