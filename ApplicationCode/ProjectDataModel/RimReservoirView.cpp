/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimReservoirView.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimCellRangeFilterCollection.h"
#include "RimFaultCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimResultSlot.h"
#include "RimTernaryLegendConfig.h"
#include "RimWell.h"
#include "RimWellCollection.h"
#include "RimWellPathCollection.h"
#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RivReservoirPipesPartMgr.h"
#include "RivWellPathCollectionPartMgr.h"

#include "cafCadNavigation.h"
#include "cafCeetronPlusNavigation.h"
#include "cafFrameAnimationControl.h"

#include "cvfDrawable.h"
#include "cvfModelBasicList.h"
#include "cvfOverlayScalarMapperLegend.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfViewport.h" 
#include "cvfqtUtils.h"

#include <QMessageBox>

#include <limits.h>


namespace caf {

template<>
void caf::AppEnum< RimReservoirView::MeshModeType >::setUp()
{
    addItem(RimReservoirView::FULL_MESH,      "FULL_MESH",       "All");
    addItem(RimReservoirView::FAULTS_MESH,    "FAULTS_MESH",      "Faults only");
    addItem(RimReservoirView::NO_MESH,        "NO_MESH",        "None");
    setDefault(RimReservoirView::FULL_MESH);
}

template<>
void caf::AppEnum< RimReservoirView::SurfaceModeType >::setUp()
{
    addItem(RimReservoirView::SURFACE,              "SURFACE",             "All");
    addItem(RimReservoirView::FAULTS,               "FAULTS",              "Faults only");
    addItem(RimReservoirView::NO_SURFACE,           "NO_SURFACE",          "None");
    setDefault(RimReservoirView::SURFACE);
}

} // End namespace caf





CAF_PDM_SOURCE_INIT(RimReservoirView, "ReservoirView");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView::RimReservoirView()
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);

    CAF_PDM_InitObject("Reservoir View", ":/ReservoirView.png", "", "");
 
    CAF_PDM_InitFieldNoDefault(&cellResult,  "GridCellResult", "Cell Result", ":/CellResult.png", "", "");
    cellResult = new RimResultSlot();

    CAF_PDM_InitFieldNoDefault(&cellEdgeResult,  "GridCellEdgeResult", "Cell Edge Result", ":/EdgeResult_1.png", "", "");
    cellEdgeResult = new RimCellEdgeResultSlot();

    CAF_PDM_InitFieldNoDefault(&overlayInfoConfig,  "OverlayInfoConfig", "Info Box", "", "", "");
    overlayInfoConfig = new Rim3dOverlayInfoConfig();
    overlayInfoConfig->setReservoirView(this);

    CAF_PDM_InitField(&name,            "UserDescription", QString(""), "Name",             "", "", "");
    
    double defaultScaleFactor = 1.0;
    if (preferences) defaultScaleFactor = preferences->defaultScaleFactorZ;
    CAF_PDM_InitField(&scaleZ,          "GridZScale", defaultScaleFactor,         "Z Scale",          "", "Scales the scene in the Z direction", "");

    CAF_PDM_InitField(&showWindow,      "ShowWindow",      true,        "Show 3D viewer",   "", "", "");
    showWindow.setUiHidden(true);

    CAF_PDM_InitField(&m_currentTimeStep, "CurrentTimeStep", 0,          "Current Time Step","", "", "");
    m_currentTimeStep.setUiHidden(true);

    CAF_PDM_InitField(&animationMode, "AnimationMode", false, "Animation Mode","", "", "");
    animationMode.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&wellCollection, "WellCollection", "Simulation Wells", "", "", "");
    wellCollection = new RimWellCollection;

    CAF_PDM_InitFieldNoDefault(&faultCollection, "FaultCollection", "Faults", "", "", "");
    faultCollection = new RimFaultCollection;

    CAF_PDM_InitFieldNoDefault(&rangeFilterCollection, "RangeFilters", "Range Filters",         "", "", "");
    rangeFilterCollection = new RimCellRangeFilterCollection();
    rangeFilterCollection->setReservoirView(this);

    CAF_PDM_InitFieldNoDefault(&propertyFilterCollection, "PropertyFilters", "Property Filters",         "", "", "");
    propertyFilterCollection = new RimCellPropertyFilterCollection();
    propertyFilterCollection->setReservoirView(this);

    caf::AppEnum<RimReservoirView::MeshModeType> defaultMeshType = NO_MESH;
    if (preferences->defaultGridLines) defaultMeshType = FULL_MESH;
    CAF_PDM_InitField(&meshMode, "MeshMode", defaultMeshType, "Grid lines",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&surfaceMode, "SurfaceMode", "Grid surface",  "", "", "");

    CAF_PDM_InitField(&maximumFrameRate, "MaximumFrameRate", 10, "Maximum frame rate","", "", "");
    maximumFrameRate.setUiHidden(true);

    // Visualization fields
    CAF_PDM_InitField(&showMainGrid,        "ShowMainGrid",         true,   "Show Main Grid",   "", "", "");
    CAF_PDM_InitField(&showInactiveCells,   "ShowInactiveCells",    false,  "Show Inactive Cells",   "", "", "");
    CAF_PDM_InitField(&showInvalidCells,    "ShowInvalidCells",     false,  "Show Invalid Cells",   "", "", "");
    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitField(&backgroundColor,     "ViewBackgroundColor",  defBackgColor, "Background", "", "", "");


    CAF_PDM_InitField(&cameraPosition,      "CameraPosition", cvf::Mat4d::IDENTITY, "", "", "", "");

  
    this->cellResult()->setReservoirView(this);
    this->cellResult()->legendConfig()->setPosition(cvf::Vec2ui(10, 120));

    this->cellEdgeResult()->setReservoirView(this);
    this->cellEdgeResult()->legendConfig()->setReservoirView(this);
    this->cellEdgeResult()->legendConfig()->setPosition(cvf::Vec2ui(10, 320));
    this->cellEdgeResult()->legendConfig()->setColorRangeMode(RimLegendConfig::PINK_WHITE);

    m_reservoirGridPartManager = new RivReservoirViewPartMgr(this);

    m_pipesPartManager = new RivReservoirPipesPartMgr(this);
    m_reservoir = NULL;

    m_previousGridModeMeshLinesWasFaults = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView::~RimReservoirView()
{
    delete this->cellResult();
    delete this->cellEdgeResult();
    delete this->overlayInfoConfig();

    delete rangeFilterCollection();
    delete propertyFilterCollection();
    delete wellCollection();
    delete faultCollection();

    if (m_viewer)
    {
        RiuMainWindow::instance()->removeViewer(m_viewer);
    }

    m_reservoirGridPartManager->clearGeometryCache();
    delete m_viewer;
    
    m_reservoir = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateViewerWidget()
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

            RiuMainWindow::instance()->addViewer(m_viewer);
            m_viewer->setMinNearPlaneDistance(10);
            this->cellResult()->legendConfig->recreateLegend();
            this->cellResult()->ternaryLegendConfig->recreateLegend();
            this->cellEdgeResult()->legendConfig->recreateLegend();
            m_viewer->setColorLegend1(this->cellResult()->legendConfig->legend());
            m_viewer->setColorLegend2(this->cellEdgeResult()->legendConfig->legend());

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
void RimReservoirView::updateViewerWidgetWindowTitle()
{
    if (m_viewer)
    {
        QString windowTitle;
        if (m_reservoir.notNull())
        {
            windowTitle = QString("%1 - %2").arg(m_reservoir->caseUserDescription()).arg(name);
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
void RimReservoirView::clampCurrentTimestep()
{
    // Clamp the current timestep to actual possibilities
    if (this->currentGridCellResults() && this->currentGridCellResults()->cellResults()) 
    {
        if (m_currentTimeStep() >= static_cast<int>(this->currentGridCellResults()->cellResults()->maxTimeStepCount()))
        {
            m_currentTimeStep = static_cast<int>(this->currentGridCellResults()->cellResults()->maxTimeStepCount()) -1;
        }
    }

    if (m_currentTimeStep < 0 ) m_currentTimeStep = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::scheduleCreateDisplayModelAndRedraw()
{
    RiaApplication::instance()->scheduleDisplayModelUpdateAndRedraw(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::createDisplayModelAndRedraw()
{
    if (m_viewer)
    {
        m_viewer->animationControl()->slotStop();

        this->clampCurrentTimestep();

        createDisplayModel();
        updateDisplayModelVisibility();

        if (m_viewer->frameCount() > 0)
        {
            m_viewer->animationControl()->setCurrentFrame(m_currentTimeStep);
        }
    }

    RiuMainWindow::instance()->refreshAnimationActions(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setDefaultView()
{
    if (m_viewer)
    {
        m_viewer->setDefaultView();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &scaleZ )
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

            eye[2] = poi[2]*scaleZ()/m_reservoirGridPartManager->scaleTransform()->worldTransform()(2,2) + (eye[2] - poi[2]);
            poi[2] = poi[2]*scaleZ()/m_reservoirGridPartManager->scaleTransform()->worldTransform()(2,2);

            m_viewer->mainCamera()->setFromLookAt(eye, eye + dir, up);
            m_viewer->setPointOfInterest(poi);

            updateScaleTransform();
            createDisplayModelAndRedraw();
            m_viewer->update();
        }

        RiuMainWindow::instance()->updateScaleValue();
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
    else if (changedField == &showWindow ) 
    {
        if (showWindow)
        {
            bool generateDisplayModel = (viewer() == NULL);
            updateViewerWidget();
            if (generateDisplayModel)
            {
                updateDisplayModelForWellResults();
            }
        }
        else
        {
            if (m_viewer)
            {
                RiuMainWindow::instance()->removeViewer(m_viewer);
                delete m_viewer;
                m_viewer = NULL;
            }
        }

        this->updateUiIconFromState(showWindow);
    }
    else if (changedField == &backgroundColor ) 
    {
        if (viewer() != NULL)
        {
            updateViewerWidget();
        }
    }
    else if (changedField == &m_currentTimeStep)
    {
        if (m_viewer)
        {
            m_viewer->update();
        }
    }
    else if (changedField == &showInvalidCells )
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::INACTIVE);
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        createDisplayModelAndRedraw();
    } 
    else if ( changedField == &showInactiveCells )
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::INACTIVE);
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        createDisplayModelAndRedraw();
    } 
    else if ( changedField == &showMainGrid )
    {
        createDisplayModelAndRedraw();
    } 
    else if ( changedField == &rangeFilterCollection )
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        scheduleCreateDisplayModelAndRedraw();
    } 
    else if ( changedField == &propertyFilterCollection)
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::PROPERTY_FILTERED);

        scheduleCreateDisplayModelAndRedraw();
    } 
    else if (changedField == &meshMode)
    {
        createDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
    } 
    else if (changedField == &surfaceMode)
    {
        createDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
    else if (changedField == &name)
    {
        updateViewerWidgetWindowTitle();
    }
}

void RimReservoirView::updateScaleTransform()
{
    CVF_ASSERT(m_reservoirGridPartManager.notNull());
    CVF_ASSERT(m_pipesPartManager.notNull());

    cvf::Mat4d scale = cvf::Mat4d::IDENTITY;
    scale(2, 2) = scaleZ();

    m_reservoirGridPartManager->setScaleTransform(scale);
    m_pipesPartManager->setScaleTransform(m_reservoirGridPartManager->scaleTransform());

    if (m_viewer) m_viewer->updateCachedValuesInScene();
}



//--------------------------------------------------------------------------------------------------
/// Create display model,
/// or at least empty scenes as frames that is delivered to the viewer
/// The real geometry generation is done inside RivReservoirViewGeometry and friends
//--------------------------------------------------------------------------------------------------
void RimReservoirView::createDisplayModel()
{
    if (m_viewer.isNull()) return;

     //static int callCount = 0;
     //std::cout << "RimReservoirView::createDisplayModel() " << callCount++ << std::endl;
     //RiuMainWindow::instance()->setResultInfo(QString ("RimReservoirView::createDisplayModel() ") + QString::number(callCount++));

    if (!(m_reservoir && m_reservoir->reservoirData())) return;

    // Define a vector containing time step indices to produce geometry for.
    // First entry in this vector is used to define the geometry only result mode with no results.
    std::vector<size_t> timeStepIndices;

    // The one and only geometry entry
    timeStepIndices.push_back(0);

    // Find the number of time frames the animation needs to show the requested data.

    if (this->cellResult()->hasDynamicResult() 
        || this->propertyFilterCollection()->hasActiveDynamicFilters() 
        || this->wellCollection->hasVisibleWellPipes()
        || this->cellResult()->isTernarySaturationSelected())
    {
        CVF_ASSERT(currentGridCellResults());

        size_t i;
        for (i = 0; i < currentGridCellResults()->cellResults()->maxTimeStepCount(); i++)
        {
            timeStepIndices.push_back(i);
        }
    } 
    else if (this->cellResult()->hasStaticResult() 
        || this->cellEdgeResult()->hasResult() 
        || this->propertyFilterCollection()->hasActiveFilters())
    {
        // The one and only result entry
        timeStepIndices.push_back(0);
    }



    cvf::Collection<cvf::ModelBasicList> frameModels;
    size_t timeIdx;
    for (timeIdx = 0; timeIdx < timeStepIndices.size(); timeIdx++)
    {
        frameModels.push_back(new cvf::ModelBasicList);
    }

    // Remove all existing animation frames from the viewer. 
    // The parts are still cached in the RivReservoir geometry and friends

    bool isAnimationActive = m_viewer->isAnimationActive();
    m_viewer->removeAllFrames();

    wellCollection->scheduleIsWellPipesVisibleRecalculation();

    // Create vector of grid indices to render
    std::vector<size_t> gridIndices;
    this->indicesToVisibleGrids(&gridIndices);

    ///
    // Get or create the parts for "static" type geometry. The same geometry is used 
    // for the different frames. updateCurrentTimeStep updates the colors etc.
    // For property filtered geometry : just set all the models as empty scenes 
    // updateCurrentTimeStep requests the actual parts

    if (!this->propertyFilterCollection()->hasActiveFilters())
    {
        std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> geometryTypesToAdd;

        if (this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
        {
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::RANGE_FILTERED);
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
            if (this->showInactiveCells())
            {
                geometryTypesToAdd.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
            }
        }
        else if (!this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
        {
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS);
        }
        else if (this->rangeFilterCollection()->hasActiveFilters() && !this->wellCollection()->hasVisibleWellCells())
        {
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::RANGE_FILTERED);
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);
            if (this->showInactiveCells())
            {
                geometryTypesToAdd.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
            }
        }
        else
        {
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::ALL_WELL_CELLS); // Should be all well cells
            geometryTypesToAdd.push_back(RivReservoirViewPartMgr::ACTIVE);

            if (this->showInactiveCells())
            {
                geometryTypesToAdd.push_back(RivReservoirViewPartMgr::INACTIVE);
            }
        }

        size_t frameIdx;
        for (frameIdx = 0; frameIdx < frameModels.size(); ++frameIdx)
        {
            for (size_t gtIdx = 0; gtIdx < geometryTypesToAdd.size(); ++gtIdx)
            {
                m_reservoirGridPartManager->appendStaticGeometryPartsToModel(frameModels[frameIdx].p(), geometryTypesToAdd[gtIdx], gridIndices); 
            }
        }

        // Set static colors 
        this->updateStaticCellColors();

        m_visibleGridParts = geometryTypesToAdd;
    }

    if (!this->propertyFilterCollection()->hasActiveFilters() || faultCollection()->showFaultsOutsideFilters)
    {
        std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultGeometryTypesToAppend = visibleFaultParts();

        RivReservoirViewPartMgr::ReservoirGeometryCacheType faultLabelType = m_reservoirGridPartManager->geometryTypeForFaultLabels(faultGeometryTypesToAppend);

        for (size_t frameIdx = 0; frameIdx < frameModels.size(); ++frameIdx)
        {
            for (size_t gtIdx = 0; gtIdx < faultGeometryTypesToAppend.size(); ++gtIdx)
            {
                m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameModels[frameIdx].p(), faultGeometryTypesToAppend[gtIdx]);
            }

            m_reservoirGridPartManager->appendFaultLabelsStaticGeometryPartsToModel(frameModels[frameIdx].p(), faultLabelType);
        }

        updateFaultForcedVisibility();

    }

    // Compute triangle count, Debug only

    if (false)
    {
        size_t totalTriangleCount = 0;
        {
            size_t mIdx;
            for (mIdx = 0; mIdx < frameModels.size(); mIdx++)
            {
                cvf::Collection<cvf::Part> partCollection;
                frameModels.at(mIdx)->allParts(&partCollection);

                size_t modelTriangleCount = 0;
                size_t pIdx;
                for (pIdx = 0; pIdx < partCollection.size(); pIdx++)
                {
                    modelTriangleCount += partCollection.at(pIdx)->drawable()->triangleCount();
                }

                totalTriangleCount += modelTriangleCount;
            }
        }
    }

    // Create Scenes from the frameModels
    // Animation frames for results display, starts from frame 1

    size_t frameIndex;
    for (frameIndex = 0; frameIndex < frameModels.size(); frameIndex++)
    {
        cvf::ModelBasicList* model = frameModels.at(frameIndex);
        model->updateBoundingBoxesRecursive();

        cvf::ref<cvf::Scene> scene = new cvf::Scene;
        scene->addModel(model);

        if (frameIndex == 0)
            m_viewer->setMainScene(scene.p());
        else
            m_viewer->addFrame(scene.p());
    }

    // If the animation was active before recreating everything, make viewer view current frame

    if (isAnimationActive)
    {
        m_viewer->slotSetCurrentFrame(m_currentTimeStep);
    }

    overlayInfoConfig()->update3DInfo();
    updateLegends(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateCurrentTimeStep()
{
    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> geometriesToRecolor;

    if (this->propertyFilterCollection()->hasActiveFilters())
    {
        cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;

        std::vector<size_t> gridIndices;
        this->indicesToVisibleGrids(&gridIndices);

        geometriesToRecolor.push_back( RivReservoirViewPartMgr::PROPERTY_FILTERED);
        m_reservoirGridPartManager->appendDynamicGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::PROPERTY_FILTERED, m_currentTimeStep, gridIndices);

        geometriesToRecolor.push_back( RivReservoirViewPartMgr::PROPERTY_FILTERED_WELL_CELLS);
        m_reservoirGridPartManager->appendDynamicGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::PROPERTY_FILTERED_WELL_CELLS, m_currentTimeStep, gridIndices);

        if (faultCollection()->showFaultsOutsideFilters)
        {
            std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultGeometryTypesToAppend = visibleFaultParts();

            for (size_t i = 0; i < faultGeometryTypesToAppend.size(); i++)
            {
                m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameParts.p(), faultGeometryTypesToAppend[i]);
            }

            RivReservoirViewPartMgr::ReservoirGeometryCacheType faultLabelType = m_reservoirGridPartManager->geometryTypeForFaultLabels(faultGeometryTypesToAppend);
            m_reservoirGridPartManager->appendFaultLabelsStaticGeometryPartsToModel(frameParts.p(), faultLabelType);
        }
        else
        {
            m_reservoirGridPartManager->appendFaultsDynamicGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::PROPERTY_FILTERED, m_currentTimeStep);
            m_reservoirGridPartManager->appendFaultLabelsDynamicGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::PROPERTY_FILTERED, m_currentTimeStep);

            m_reservoirGridPartManager->appendFaultsDynamicGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::PROPERTY_FILTERED_WELL_CELLS, m_currentTimeStep);
        }

        // Set the transparency on all the Wellcell parts before setting the result color
        float opacity = static_cast< float> (1 - cvf::Math::clamp(this->wellCollection()->wellCellTransparencyLevel(), 0.0, 1.0));
        m_reservoirGridPartManager->updateCellColor(RivReservoirViewPartMgr::PROPERTY_FILTERED_WELL_CELLS, m_currentTimeStep, cvf::Color4f(cvf::Color3f(cvf::Color3::WHITE), opacity));


        if (this->showInactiveCells())
        {
            std::vector<size_t> gridIndices;
            this->indicesToVisibleGrids(&gridIndices);
 
            if (this->rangeFilterCollection()->hasActiveFilters() ) // Wells not considered, because we do not have a INACTIVE_WELL_CELLS group yet.
            {
                m_reservoirGridPartManager->appendStaticGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE, gridIndices); 

                if (!faultCollection()->showFaultsOutsideFilters)
                {
                    m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE); 
                }
            }
            else
            {
                m_reservoirGridPartManager->appendStaticGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::INACTIVE, gridIndices);

                if (!faultCollection()->showFaultsOutsideFilters)
                {
                    m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::INACTIVE);
                }
            }
        }

        if (m_viewer)
        {
            cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
            if (frameScene)
            {
                frameParts->updateBoundingBoxesRecursive();
                frameScene->removeAllModels();
                frameScene->addModel(frameParts.p());
            }
        }

        m_visibleGridParts = geometriesToRecolor;
    }
    else if (this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
    {
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::RANGE_FILTERED);
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
    }
    else if (!this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
    {
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS);
    }
    else if (this->rangeFilterCollection()->hasActiveFilters() && !this->wellCollection()->hasVisibleWellCells())
    {
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::RANGE_FILTERED);
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);
    }
    else 
    {
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::ACTIVE);
        geometriesToRecolor.push_back(RivReservoirViewPartMgr::ALL_WELL_CELLS);
    }

    for (size_t i = 0; i < geometriesToRecolor.size(); ++i)
    {
        if (this->animationMode() && this->cellEdgeResult()->hasResult())
        {
            m_reservoirGridPartManager->updateCellEdgeResultColor(geometriesToRecolor[i], m_currentTimeStep, this->cellResult(), this->cellEdgeResult());
        } 
        else if ((this->animationMode() && this->cellResult()->hasResult()) || this->cellResult()->isTernarySaturationSelected())
        {
            m_reservoirGridPartManager->updateCellResultColor(geometriesToRecolor[i], m_currentTimeStep, this->cellResult());
        }
        else
        {
            this->updateStaticCellColors(geometriesToRecolor[i]);
        }
    }

    this->updateFaultColors();

    // Well pipes and well paths
    if (m_viewer)
    {
        cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
        if (frameScene)
        {
            // Well pipes
            // ----------
            cvf::String wellPipeModelName = "WellPipeModel";
            std::vector<cvf::Model*> wellPipeModels;
            for (cvf::uint i = 0; i < frameScene->modelCount(); i++)
            {
                if (frameScene->model(i)->name() == wellPipeModelName)
                {
                    wellPipeModels.push_back(frameScene->model(i));
                }
            }

            for (size_t i = 0; i < wellPipeModels.size(); i++)
            {
                //printf("updateCurrentTimeStep: Remove WellPipeModel %i from frameScene, for frame %i\n", i, m_currentTimeStep.v());
                frameScene->removeModel(wellPipeModels[i]);
            }

            cvf::ref<cvf::ModelBasicList> wellPipeModelBasicList = new cvf::ModelBasicList;
            wellPipeModelBasicList->setName(wellPipeModelName);

            m_pipesPartManager->appendDynamicGeometryPartsToModel(wellPipeModelBasicList.p(), m_currentTimeStep);
            m_pipesPartManager->updatePipeResultColor(m_currentTimeStep);

            wellPipeModelBasicList->updateBoundingBoxesRecursive();
            //printf("updateCurrentTimeStep: Add WellPipeModel to frameScene\n");
            frameScene->addModel(wellPipeModelBasicList.p());
            
            // Well paths
            // ----------
            cvf::String wellPathModelName = "WellPathModel";
            std::vector<cvf::Model*> wellPathModels;
            for (cvf::uint i = 0; i < frameScene->modelCount(); i++)
            {
                if (frameScene->model(i)->name() == wellPathModelName)
                {
                    wellPathModels.push_back(frameScene->model(i));
                }
            }

            for (size_t i = 0; i < wellPathModels.size(); i++)
            {
                //printf("updateCurrentTimeStep: Remove WellPathModel %i from frameScene, for frame %i\n", i, m_currentTimeStep.v());
                frameScene->removeModel(wellPathModels[i]);
            }

            // Append static Well Paths to model
            cvf::ref<cvf::ModelBasicList> wellPathModelBasicList = new cvf::ModelBasicList;
            wellPathModelBasicList->setName(wellPathModelName);
            RimOilField* oilFields = (RiaApplication::instance()->project()) ? RiaApplication::instance()->project()->activeOilField() : NULL;
            RimWellPathCollection* wellPathCollection = (oilFields) ? oilFields->wellPathCollection() : NULL;
            RivWellPathCollectionPartMgr* wellPathCollectionPartMgr = (wellPathCollection) ? wellPathCollection->wellPathCollectionPartMgr() : NULL;
            if (wellPathCollectionPartMgr)
            {
                //printf("updateCurrentTimeStep: Append well paths for frame %i: ", m_currentTimeStep.v());
                cvf::Vec3d displayModelOffset = eclipseCase()->reservoirData()->mainGrid()->displayModelOffset();
                double characteristicCellSize = eclipseCase()->reservoirData()->mainGrid()->characteristicIJCellSize();
                cvf::BoundingBox boundingBox = currentActiveCellInfo()->geometryBoundingBox();
                wellPathCollectionPartMgr->appendStaticGeometryPartsToModel(wellPathModelBasicList.p(), displayModelOffset, m_reservoirGridPartManager->scaleTransform(), characteristicCellSize, boundingBox); 
                //printf("\n");
            }
            wellPathModelBasicList->updateBoundingBoxesRecursive();
            frameScene->addModel(wellPathModelBasicList.p());
        }
    }

    overlayInfoConfig()->update3DInfo();
    updateLegends();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::loadDataAndUpdate()
{
    updateScaleTransform();

    if (m_reservoir)
    {
        if (!m_reservoir->openEclipseGridFile())
        {
            QMessageBox::warning(RiuMainWindow::instance(), 
                                "Error when opening project file", 
                                "Could not open the Eclipse Grid file: \n"+ m_reservoir->gridFileName());
            m_reservoir = NULL;
            return;
        }
        else
        {
            RiaApplication* app = RiaApplication::instance();
            if (app->preferences()->autocomputeSOIL)
            {
                {
                    RimReservoirCellResultsStorage* results = m_reservoir->results(RifReaderInterface::MATRIX_RESULTS);
                    results->loadOrComputeSOIL();
                    results->createCombinedTransmissibilityResults();
                }
                {
                    RimReservoirCellResultsStorage* results = m_reservoir->results(RifReaderInterface::FRACTURE_RESULTS);
                    results->loadOrComputeSOIL();
                    results->createCombinedTransmissibilityResults();
                }
            }
        }
    }

    CVF_ASSERT(this->cellResult() != NULL);
    this->cellResult()->loadResult();

    if (m_reservoir->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->globalActiveCellCount() == 0)
    {
        this->cellResult->setPorosityModelUiFieldHidden(true);
    }

    CVF_ASSERT(this->cellEdgeResult() != NULL);
    this->cellEdgeResult()->loadResult();

    updateViewerWidget();

    this->propertyFilterCollection()->loadAndInitializePropertyFilters();

    this->faultCollection()->setReservoirView(this);
    this->faultCollection()->syncronizeFaults();

    m_reservoirGridPartManager->clearGeometryCache();

    syncronizeWellsWithResults();

    createDisplayModelAndRedraw();

    if (cameraPosition().isIdentity())
    {
        setDefaultView();
    }

}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::initAfterRead()
{
    this->cellResult()->setReservoirView(this);
    this->cellEdgeResult()->setReservoirView(this);
    this->rangeFilterCollection()->setReservoirView(this);
    this->propertyFilterCollection()->setReservoirView(this);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setCurrentTimeStep(int frameIndex)
{
    m_currentTimeStep = frameIndex;
    this->animationMode = true;
    this->updateCurrentTimeStep();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::endAnimation()
{
    this->animationMode = false;
    this->updateStaticCellColors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateStaticCellColors()
{
    updateStaticCellColors( RivReservoirViewPartMgr::ACTIVE);
    updateStaticCellColors( RivReservoirViewPartMgr::ALL_WELL_CELLS);
    updateStaticCellColors( RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
    updateStaticCellColors( RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS);
    updateStaticCellColors( RivReservoirViewPartMgr::VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
    updateStaticCellColors( RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
    updateStaticCellColors( RivReservoirViewPartMgr::INACTIVE);
    updateStaticCellColors( RivReservoirViewPartMgr::RANGE_FILTERED);
    updateStaticCellColors( RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);
    updateStaticCellColors( RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateStaticCellColors(unsigned short geometryType)
{
    float opacity = static_cast< float> (1 - cvf::Math::clamp(this->wellCollection()->wellCellTransparencyLevel(), 0.0, 1.0));
    cvf::Color4f color(cvf::Color3::ORANGE);

    switch (geometryType)
    {
        case RivReservoirViewPartMgr::ACTIVE:                      color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case RivReservoirViewPartMgr::ALL_WELL_CELLS:              color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case RivReservoirViewPartMgr::VISIBLE_WELL_CELLS:          color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS:    color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case RivReservoirViewPartMgr::VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER:         
                                                                    color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER:   
                                                                    color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case RivReservoirViewPartMgr::INACTIVE:                    color = cvf::Color4f(cvf::Color3::LIGHT_GRAY);  break;
        case RivReservoirViewPartMgr::RANGE_FILTERED:              color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS:   color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE:     color = cvf::Color4f(cvf::Color3::LIGHT_GRAY);  break;   
    }

    m_reservoirGridPartManager->updateCellColor(static_cast<RivReservoirViewPartMgr::ReservoirGeometryCacheType>(geometryType), color);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer* RimReservoirView::viewer()
{
    return m_viewer;
}


//--------------------------------------------------------------------------------------------------
/// Get pick info text for given part ID, face index, and intersection point
//--------------------------------------------------------------------------------------------------
bool RimReservoirView::pickInfo(size_t gridIndex, size_t cellIndex, cvf::StructGridInterface::FaceType face, const cvf::Vec3d& point, QString* pickInfoText) const
{
    CVF_ASSERT(pickInfoText);

    if (m_reservoir)
    {
        const RigCaseData* eclipseCase = m_reservoir->reservoirData();
        if (eclipseCase)
        {
            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            if (eclipseCase->grid(gridIndex)->ijkFromCellIndex(cellIndex, &i, &j, &k))
            {
                // Adjust to 1-based Eclipse indexing
                i++;
                j++;
                k++;

                cvf::Vec3d domainCoord = point + eclipseCase->grid(gridIndex)->displayModelOffset();

                cvf::StructGridInterface::FaceEnum faceEnum(face);
                
                QString faceText = faceEnum.text();

                *pickInfoText = QString("Hit grid %1, cell [%2, %3, %4] face %5, ").arg(gridIndex).arg(i).arg(j).arg(k).arg(faceText);

                QString formattedText;
                formattedText.sprintf("intersection point: [E: %.2f, N: %.2f, Depth: %.2f]", domainCoord.x(), domainCoord.y(), -domainCoord.z());

                *pickInfoText += formattedText;
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Append fault name and result value for the given cell to the string
//--------------------------------------------------------------------------------------------------
void RimReservoirView::appendCellResultInfo(size_t gridIndex, size_t cellIndex, cvf::StructGridInterface::FaceType face,  QString* resultInfoText) 
{
    CVF_ASSERT(resultInfoText);

    if (m_reservoir && m_reservoir->reservoirData())
    {
        RigCaseData* eclipseCase = m_reservoir->reservoirData();
        RigGridBase* grid = eclipseCase->grid(gridIndex);

        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResult()->porosityModel());
        cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject;

        if (this->cellResult()->isTernarySaturationSelected())
        {
            RimReservoirCellResultsStorage* gridCellResults = this->cellResult()->currentGridCellResults();
            if (gridCellResults)
            {
                size_t soilScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
                size_t sgasScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");
                size_t swatScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");

                cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectX = eclipseCase->dataAccessObject(grid, porosityModel, m_currentTimeStep, soilScalarSetIndex);
                cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectY = eclipseCase->dataAccessObject(grid, porosityModel, m_currentTimeStep, sgasScalarSetIndex);
                cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectZ = eclipseCase->dataAccessObject(grid, porosityModel, m_currentTimeStep, swatScalarSetIndex);

                double scalarValue = 0.0;
                
                if (dataAccessObjectX.notNull()) scalarValue = dataAccessObjectX->cellScalar(cellIndex);
                else scalarValue = 0.0;
                resultInfoText->append(QString("SOIL : %1\n").arg(scalarValue));

                if (dataAccessObjectY.notNull()) scalarValue = dataAccessObjectY->cellScalar(cellIndex);
                else scalarValue = 0.0;
                resultInfoText->append(QString("SGAS : %1\n").arg(scalarValue));

                if (dataAccessObjectZ.notNull()) scalarValue = dataAccessObjectZ->cellScalar(cellIndex);
                else scalarValue = 0.0;
                resultInfoText->append(QString("SWAT : %1\n").arg(scalarValue));
            }
        }
        else if (this->cellResult()->hasResult())
        {
            RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResult()->porosityModel());
            cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject;
            
            if (cellResult->hasStaticResult())
            {
                if (this->cellResult()->resultVariable().compare(RimDefines::combinedTransmissibilityResultName(), Qt::CaseInsensitive) == 0)
                {
                    size_t tranX, tranY, tranZ;
                    if (eclipseCase->results(porosityModel)->findTransmissibilityResults(tranX, tranY, tranZ))
                    {
                        cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectX = eclipseCase->dataAccessObject(grid, porosityModel, 0, tranX);
                        cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectY = eclipseCase->dataAccessObject(grid, porosityModel, 0, tranY);
                        cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectZ = eclipseCase->dataAccessObject(grid, porosityModel, 0, tranZ);

                        double scalarValue = dataAccessObjectX->cellScalar(cellIndex);
                        resultInfoText->append(QString("Tran X : %1\n").arg(scalarValue));

                        scalarValue = dataAccessObjectY->cellScalar(cellIndex);
                        resultInfoText->append(QString("Tran Y : %1\n").arg(scalarValue));

                        scalarValue = dataAccessObjectZ->cellScalar(cellIndex);
                        resultInfoText->append(QString("Tran Z : %1\n").arg(scalarValue));
                    }
                }
                else
                {
                    dataAccessObject = eclipseCase->dataAccessObject(grid, porosityModel, 0, this->cellResult()->gridScalarIndex());
                }
            }
            else
            {
                dataAccessObject = eclipseCase->dataAccessObject(grid, porosityModel, m_currentTimeStep, this->cellResult()->gridScalarIndex());
            }

            if (dataAccessObject.notNull())
            {
                double scalarValue = dataAccessObject->cellScalar(cellIndex);
                resultInfoText->append(QString("Cell result : %1\n").arg(scalarValue));
            }
        }

        if (this->cellEdgeResult()->hasResult())
        {
            size_t resultIndices[6];
            QStringList resultNames;
            this->cellEdgeResult()->gridScalarIndices(resultIndices);
            this->cellEdgeResult()->gridScalarResultNames(&resultNames);

            for (int idx = 0; idx < 6; idx++)
            {
                if (resultIndices[idx] == cvf::UNDEFINED_SIZE_T) continue;

                // Cell edge results are static, results are loaded for first time step only
                RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResult()->porosityModel());
                cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObject = eclipseCase->dataAccessObject(grid, porosityModel, 0, resultIndices[idx]);
                if (dataAccessObject.notNull())
                {
                    double scalarValue = dataAccessObject->cellScalar(cellIndex);
                    resultInfoText->append(QString("%1 : %2\n").arg(resultNames[idx]).arg(scalarValue));
                }
            }
        }

        cvf::Collection<RigSingleWellResultsData> wellResults = m_reservoir->reservoirData()->wellResults();
        for (size_t i = 0; i < wellResults.size(); i++)
        {
            RigSingleWellResultsData* singleWellResultData = wellResults.at(i);

            if (m_currentTimeStep < static_cast<int>(singleWellResultData->firstResultTimeStep()))
            {
                continue;
            }

            const RigWellResultFrame& wellResultFrame = singleWellResultData->wellResultFrame(m_currentTimeStep);
            const RigWellResultPoint* wellResultCell = wellResultFrame.findResultCell(gridIndex, cellIndex);
            if (wellResultCell)
            {
                resultInfoText->append(QString("Well-cell connection info: Well Name: %1 Branch Id: %2 Segment Id: %3\n").arg(singleWellResultData->m_wellName).arg(wellResultCell->m_ertBranchId).arg(wellResultCell->m_ertSegmentId));
            }
        }
        
        appendFaultName(grid, cellIndex, face, resultInfoText);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateDisplayModelVisibility()
{
    if (m_viewer.isNull()) return;

    const cvf::uint uintSurfaceBit      = surfaceBit;
    const cvf::uint uintMeshSurfaceBit  = meshSurfaceBit;
    const cvf::uint uintFaultBit        = faultBit;
    const cvf::uint uintMeshFaultBit    = meshFaultBit;
 
    // Initialize the mask to show everything except the the bits controlled here
    unsigned int mask = 0xffffffff & ~uintSurfaceBit & ~uintFaultBit & ~uintMeshSurfaceBit & ~uintMeshFaultBit ;

    // Then turn the appropriate bits on according to the user settings

    if (surfaceMode == SURFACE)
    {
         mask |= uintSurfaceBit;
         mask |= uintFaultBit;
    }
    else if (surfaceMode == FAULTS)
    {
        mask |= uintFaultBit;
    }

    if (meshMode == FULL_MESH)
    {
        mask |= uintMeshSurfaceBit;
        mask |= uintMeshFaultBit;
    }
    else if (meshMode == FAULTS_MESH)
    {
        mask |= uintMeshFaultBit;
    }

    m_viewer->setEnableMask(mask);
    m_viewer->update();

    faultCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setupBeforeSave()
{
    if (m_viewer)
    {
        animationMode = m_viewer->isAnimationActive();
        cameraPosition = m_viewer->mainCamera()->viewMatrix();
    }
}

//--------------------------------------------------------------------------------------------------
/// Convenience for quick access to results
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage* RimReservoirView::currentGridCellResults()
{
    if (m_reservoir)
    {
        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResult->porosityModel());

        return m_reservoir->results(porosityModel);
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimReservoirView::currentActiveCellInfo()
{
    if (m_reservoir &&
        m_reservoir->reservoirData()
        )
    {
        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResult->porosityModel());

        return m_reservoir->reservoirData()->activeCellInfo(porosityModel);
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::scheduleGeometryRegen(unsigned short geometryType)
{
    m_reservoirGridPartManager->scheduleGeometryRegen(static_cast<RivReservoirViewPartMgr::ReservoirGeometryCacheType>(geometryType));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::scheduleReservoirGridGeometryRegen()
{
    m_reservoirGridPartManager->clearGeometryCache();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirView::schedulePipeGeometryRegen()
{
    m_pipesPartManager->scheduleGeometryRegen();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateCurrentTimeStepAndRedraw()
{
    this->updateCurrentTimeStep();
    
    if (m_viewer) m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::indicesToVisibleGrids(std::vector<size_t>* gridIndices)
{
    CVF_ASSERT(gridIndices != NULL);

    // Create vector of grid indices to render
    std::vector<RigGridBase*> grids;
    if (this->m_reservoir && this->m_reservoir->reservoirData() )
    {
        this->m_reservoir->reservoirData()->allGrids(&grids);
    }

    size_t i;
    for (i = 0; i < grids.size(); i++)
    {
        if (!grids[i]->isMainGrid() || this->showMainGrid() )
        {
            gridIndices->push_back(i);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateLegends()
{
    if (!m_reservoir || !m_viewer || !m_reservoir->reservoirData() )
    {
        return;
    }

    RigCaseData* eclipseCase = m_reservoir->reservoirData();
    CVF_ASSERT(eclipseCase);

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResult()->porosityModel());
    RigCaseCellResultsData* results = eclipseCase->results(porosityModel);
    CVF_ASSERT(results);

    if (this->cellResult()->hasResult())
    {
        double globalMin, globalMax;
        double globalPosClosestToZero, globalNegClosestToZero;
        results->minMaxCellScalarValues(this->cellResult()->gridScalarIndex(), globalMin, globalMax);
        results->posNegClosestToZero(this->cellResult()->gridScalarIndex(), globalPosClosestToZero, globalNegClosestToZero);

        double localMin, localMax;
        double localPosClosestToZero, localNegClosestToZero;
        if (this->cellResult()->hasDynamicResult())
        {
            results->minMaxCellScalarValues(this->cellResult()->gridScalarIndex(), m_currentTimeStep, localMin, localMax);
            results->posNegClosestToZero(this->cellResult()->gridScalarIndex(), m_currentTimeStep, localPosClosestToZero, localNegClosestToZero);
        }
        else
        {
             localMin = globalMin;
             localMax = globalMax;

             localPosClosestToZero = globalPosClosestToZero;
             localNegClosestToZero = globalNegClosestToZero;
        }

        this->cellResult()->legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
        this->cellResult()->legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

        m_viewer->setColorLegend1(this->cellResult()->legendConfig->legend());
        this->cellResult()->legendConfig->legend()->setTitle(cvfqt::Utils::toString(QString("Cell Results: \n") + this->cellResult()->resultVariable()));
    }
    else
    {
        this->cellResult()->legendConfig->setClosestToZeroValues(0, 0, 0, 0);
        this->cellResult()->legendConfig->setAutomaticRanges(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);
        m_viewer->setColorLegend1(NULL);
    }

    if (this->cellEdgeResult()->hasResult())
    {
        double globalMin, globalMax;
        double globalPosClosestToZero, globalNegClosestToZero;
        this->cellEdgeResult()->minMaxCellEdgeValues(globalMin, globalMax);
        this->cellEdgeResult()->posNegClosestToZero(globalPosClosestToZero, globalNegClosestToZero);

        this->cellEdgeResult()->legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, globalPosClosestToZero, globalNegClosestToZero);
        this->cellEdgeResult()->legendConfig->setAutomaticRanges(globalMin, globalMax, globalMin, globalMax);

        m_viewer->setColorLegend2(this->cellEdgeResult()->legendConfig->legend());
        this->cellEdgeResult()->legendConfig->legend()->setTitle(cvfqt::Utils::toString(QString("Edge Results: \n") + this->cellEdgeResult()->resultVariable));

    }
    else
    {
        m_viewer->setColorLegend2(NULL);
        this->cellEdgeResult()->legendConfig->setClosestToZeroValues(0, 0, 0, 0);
        this->cellEdgeResult()->legendConfig->setAutomaticRanges(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);
    }

    
    viewer()->removeOverlayItem(this->cellResult()->ternaryLegendConfig->legend());

    size_t maxTimeStepCount = results->maxTimeStepCount();
    if (this->cellResult()->isTernarySaturationSelected() && maxTimeStepCount > 1)
    {
        RimReservoirCellResultsStorage* gridCellResults = this->cellResult()->currentGridCellResults();
        {
            double globalMin = 0.0;
            double globalMax = 1.0;
            double localMin = 0.0;
            double localMax = 1.0;

            size_t scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
            if (scalarSetIndex != cvf::UNDEFINED_SIZE_T)
            {
                results->minMaxCellScalarValues(scalarSetIndex, globalMin, globalMax);
                results->minMaxCellScalarValues(scalarSetIndex, m_currentTimeStep, localMin, localMax);

                this->cellResult()->ternaryLegendConfig()->setAutomaticRanges(RimTernaryLegendConfig::TERNARY_SOIL_IDX, globalMin, globalMax, localMin, localMax);
            }
        }

        {
            double globalMin = 0.0;
            double globalMax = 1.0;
            double localMin = 0.0;
            double localMax = 1.0;

            size_t scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");
            if (scalarSetIndex != cvf::UNDEFINED_SIZE_T)
            {
                results->minMaxCellScalarValues(scalarSetIndex, globalMin, globalMax);
                results->minMaxCellScalarValues(scalarSetIndex, m_currentTimeStep, localMin, localMax);

                this->cellResult()->ternaryLegendConfig()->setAutomaticRanges(RimTernaryLegendConfig::TERNARY_SGAS_IDX, globalMin, globalMax, localMin, localMax);
            }
        }

        {
            double globalMin = 0.0;
            double globalMax = 1.0;
            double localMin = 0.0;
            double localMax = 1.0;

            size_t scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");
            if (scalarSetIndex != cvf::UNDEFINED_SIZE_T)
            {
                results->minMaxCellScalarValues(scalarSetIndex, globalMin, globalMax);
                results->minMaxCellScalarValues(scalarSetIndex, m_currentTimeStep, localMin, localMax);

                this->cellResult()->ternaryLegendConfig()->setAutomaticRanges(RimTernaryLegendConfig::TERNARY_SWAT_IDX, globalMin, globalMax, localMin, localMax);
            }
        }

        if (this->cellResult()->ternaryLegendConfig->legend())
        {
            viewer()->addOverlayItem(this->cellResult()->ternaryLegendConfig->legend());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setEclipseCase(RimCase* reservoir)
{
    m_reservoir = reservoir;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* RimReservoirView::eclipseCase()
{
    return m_reservoir;
}

//--------------------------------------------------------------------------------------------------
//
/*
    wells vs wellres
    For all wellres 
        find well
            if no well, create new 
        connect well and wellres
    for all wells not connected
        Delete ?

*/
//--------------------------------------------------------------------------------------------------
void RimReservoirView::syncronizeWellsWithResults()
{
    if (!(m_reservoir && m_reservoir->reservoirData()) ) return;

    cvf::Collection<RigSingleWellResultsData> wellResults = m_reservoir->reservoirData()->wellResults();

 
    std::vector<caf::PdmPointer<RimWell> > newWells;

    // Clear the possible well results data present
    for (size_t wIdx = 0; wIdx < this->wellCollection()->wells().size(); ++wIdx)
    {
        RimWell* well = this->wellCollection()->wells()[wIdx];
        well->setWellResults(NULL);
    }

    // Find corresponding well from well result, or create a new

    for (size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx)
    {
        RimWell* well = this->wellCollection()->findWell(wellResults[wIdx]->m_wellName);

        if (!well)
        {
            well = new RimWell;
            well->name = wellResults[wIdx]->m_wellName;
            
        }
        newWells.push_back(well);
        well->setWellIndex(wIdx);
        well->setWellResults(wellResults[wIdx].p());
    }

    // Delete all wells that does not have a result

    for (size_t wIdx = 0; wIdx < this->wellCollection()->wells().size(); ++wIdx)
    {
        RimWell* well = this->wellCollection()->wells()[wIdx];
        RigSingleWellResultsData* wellRes = well->wellResults();
        if (wellRes == NULL)
        {
            delete well;
        }
    }
    this->wellCollection()->wells().clear();

    // Set the new wells into the field.
    this->wellCollection()->wells().insert(0, newWells);


    // Make sure all the wells have their reservoirView ptr setup correctly
    this->wellCollection()->setReservoirView(this);
    for (size_t wIdx = 0; wIdx < this->wellCollection()->wells().size(); ++wIdx)
    {
        this->wellCollection()->wells()[wIdx]->setReservoirView(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid)
{
    CVF_ASSERT(visibleCells != NULL);

    // Initialize the return array
    if (visibleCells->size() != grid->cellCount())
    {
        visibleCells->resize(grid->cellCount());
    }
    visibleCells->setAll(false);

    // If all wells are forced off, return
    if (this->wellCollection()->wellCellsToRangeFilterMode() == RimWellCollection::RANGE_ADD_NONE) return;

    RigActiveCellInfo* activeCellInfo = this->currentActiveCellInfo();

    CVF_ASSERT(activeCellInfo);

    // Loop over the wells and find their contribution
    for (size_t wIdx = 0; wIdx < this->wellCollection()->wells().size(); ++wIdx)
    {
        RimWell* well =  this->wellCollection()->wells()[wIdx];
        if (this->wellCollection()->wellCellsToRangeFilterMode() == RimWellCollection::RANGE_ADD_ALL || (well->showWell() && well->showWellCells()) )
        {
            RigSingleWellResultsData* wres = well->wellResults();
            if (!wres) continue;

            const std::vector< RigWellResultFrame >& wellResFrames = wres->m_wellCellsTimeSteps;
            for (size_t wfIdx = 0; wfIdx < wellResFrames.size(); ++wfIdx)
            {
                // Add the wellhead cell if it is active
                if (wellResFrames[wfIdx].m_wellHead.m_gridIndex == grid->gridIndex())
                {
                    size_t gridCellIndex = wellResFrames[wfIdx].m_wellHead.m_gridCellIndex;
                    size_t globalGridCellIndex = grid->globalGridCellIndex(gridCellIndex);

                    if (activeCellInfo->isActive(globalGridCellIndex))
                    {
                        (*visibleCells)[gridCellIndex] = true;
                    }
                }

                // Add all the cells from the branches

                const std::vector<RigWellResultBranch>& wellResSegments = wellResFrames[wfIdx].m_wellResultBranches;
                for (size_t wsIdx = 0; wsIdx < wellResSegments.size(); ++wsIdx)
                {
                    const std::vector<RigWellResultPoint>& wsResCells = wellResSegments[wsIdx].m_branchResultPoints;
                    for (size_t cIdx = 0; cIdx < wsResCells.size(); ++ cIdx)
                    {
                        if (wsResCells[cIdx].m_gridIndex == grid->gridIndex())
                        {
                            if (!wsResCells[cIdx].isCell())
                            {
                                continue;
                            }

                            size_t gridCellIndex = wsResCells[cIdx].m_gridCellIndex;
                            (*visibleCells)[gridCellIndex] = true;

                            // Calculate well fence cells
                            if (well->showWellCellFence() || this->wellCollection()->showWellCellFences())
                            {
                                size_t i, j, k;
                                grid->ijkFromCellIndex(gridCellIndex, &i, &j, &k);

                                size_t* pI = &i;
                                size_t *pJ = &j;
                                size_t *pK = &k;
                                size_t cellCountFenceDirection = 0;
                                size_t fIdx = 0;

                                if (this->wellCollection()->wellCellFenceType == RimWellCollection::K_DIRECTION)
                                {
                                    cellCountFenceDirection = grid->cellCountK();
                                    pK = &fIdx;
                                }
                                else if (this->wellCollection()->wellCellFenceType == RimWellCollection::J_DIRECTION)
                                {
                                    cellCountFenceDirection = grid->cellCountJ();
                                    pJ = &fIdx;
                                }
                                else if (this->wellCollection()->wellCellFenceType == RimWellCollection::I_DIRECTION)
                                {
                                    cellCountFenceDirection = grid->cellCountI();
                                    pI = &fIdx;
                                }

                                for ( fIdx = 0; fIdx < cellCountFenceDirection; ++fIdx)
                                {
                                    size_t fenceCellIndex = grid->cellIndexFromIJK(*pI,*pJ,*pK);
                                    size_t globalGridCellIndex = grid->globalGridCellIndex(fenceCellIndex);

                                    if (activeCellInfo && activeCellInfo->isActive(globalGridCellIndex))
                                    {
                                        (*visibleCells)[fenceCellIndex] = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateDisplayModelForWellResults()
{
    m_reservoirGridPartManager->clearGeometryCache();
    m_pipesPartManager->clearGeometryCache();

    syncronizeWellsWithResults();

    createDisplayModel();
    updateDisplayModelVisibility();

    if (animationMode && m_viewer)
    {
        m_viewer->slotSetCurrentFrame(m_currentTimeStep);
    }

    RiuMainWindow::instance()->refreshAnimationActions(); 
   
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setMeshOnlyDrawstyle()
{
    if (isGridVisualizationMode())
    {
        meshMode.setValueFromUi(FULL_MESH);
    }
    else
    {
        meshMode.setValueFromUi(FAULTS_MESH);
    }

    surfaceMode.setValueFromUi(NO_SURFACE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setMeshSurfDrawstyle()
{
    if (isGridVisualizationMode())
    {
        surfaceMode.setValueFromUi(SURFACE);
        meshMode.setValueFromUi(FULL_MESH);
    }
    else
    {
        surfaceMode.setValueFromUi(FAULTS);
        meshMode.setValueFromUi(FAULTS_MESH);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setFaultMeshSurfDrawstyle()
{
    // Surf: No Fault Surf
    //  Mesh -------------
    //    No FF  FF    SF
    // Fault FF  FF    SF
    //  Mesh SF  SF    SF
    if (this->isGridVisualizationMode())
    {
         surfaceMode.setValueFromUi(SURFACE);
    }
    else
    {
         surfaceMode.setValueFromUi(FAULTS);
    }

    meshMode.setValueFromUi(FAULTS_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setSurfOnlyDrawstyle()
{
    if (isGridVisualizationMode())
    {
        surfaceMode.setValueFromUi(SURFACE);
    }
    else
    {
        surfaceMode.setValueFromUi(FAULTS);
    }
    meshMode.setValueFromUi(NO_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::setShowFaultsOnly(bool showFaults)
{
    if (showFaults)
    {
        m_previousGridModeMeshLinesWasFaults = meshMode() == FAULTS_MESH;
        if (surfaceMode() != NO_SURFACE) surfaceMode.setValueFromUi(FAULTS);
        if (meshMode() != NO_MESH) meshMode.setValueFromUi(FAULTS_MESH);
    }
    else
    {
        if (surfaceMode() != NO_SURFACE) surfaceMode.setValueFromUi(SURFACE);
        if (meshMode() != NO_MESH) meshMode.setValueFromUi(m_previousGridModeMeshLinesWasFaults ? FAULTS_MESH: FULL_MESH);
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
bool RimReservoirView::isGridVisualizationMode() const
{
    return (   this->surfaceMode() == SURFACE 
            || this->meshMode()    == FULL_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimReservoirView::objectToggleField()
{
    return &showWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup("Viewer");
    viewGroup->add(&name);
    viewGroup->add(&backgroundColor);

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Grid Appearance");
    gridGroup->add(&scaleZ);
    gridGroup->add(&meshMode);
    gridGroup->add(&surfaceMode);

    caf::PdmUiGroup* cellGroup = uiOrdering.addNewGroup("Cell Visibility");
    cellGroup->add(&showMainGrid);
    cellGroup->add(&showInactiveCells);
    cellGroup->add(&showInvalidCells);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::appendNNCResultInfo(size_t nncIndex, QString* resultInfo)
{
    CVF_ASSERT(resultInfo);

    if (m_reservoir && m_reservoir->reservoirData())
    {
        RigCaseData* eclipseCase = m_reservoir->reservoirData();

        RigMainGrid* grid = eclipseCase->mainGrid();
        CVF_ASSERT(grid);
        
        RigNNCData* nncData = grid->nncData();
        CVF_ASSERT(nncData);

        if (nncData)
        {
            const RigConnection& conn = nncData->connections()[nncIndex];
            cvf::StructGridInterface::FaceEnum face(conn.m_c1Face);

            QString faultName;
        
            resultInfo->append(QString("NNC Transmissibility : %1\n").arg(conn.m_transmissibility));
            {
                CVF_ASSERT(conn.m_c1GlobIdx < grid->cells().size());
                const RigCell& cell = grid->cells()[conn.m_c1GlobIdx];

                RigGridBase* hostGrid = cell.hostGrid();
                size_t localCellIndex = cell.cellIndex();

                size_t i, j, k;
                if (hostGrid->ijkFromCellIndex(localCellIndex, &i, &j, &k))
                {
                    // Adjust to 1-based Eclipse indexing
                    i++;
                    j++;
                    k++;

                    QString gridName = QString::fromStdString(hostGrid->gridName());
                    resultInfo->append(QString("NNC 1 : cell [%1, %2, %3] face %4 (%5)\n").arg(i).arg(j).arg(k).arg(face.text()).arg(gridName));

                    appendFaultName(hostGrid, conn.m_c1GlobIdx, conn.m_c1Face, &faultName);
                }
            }

            {
                CVF_ASSERT(conn.m_c2GlobIdx < grid->cells().size());
                const RigCell& cell = grid->cells()[conn.m_c2GlobIdx];

                RigGridBase* hostGrid = cell.hostGrid();
                size_t localCellIndex = cell.cellIndex();

                size_t i, j, k;
                if (hostGrid->ijkFromCellIndex(localCellIndex, &i, &j, &k))
                {
                    // Adjust to 1-based Eclipse indexing
                    i++;
                    j++;
                    k++;

                    QString gridName = QString::fromStdString(hostGrid->gridName());
                    cvf::StructGridInterface::FaceEnum oppositeFaceEnum(cvf::StructGridInterface::oppositeFace(face));
                    QString faceText = oppositeFaceEnum.text();

                    resultInfo->append(QString("NNC 2 : cell [%1, %2, %3] face %4 (%5)\n").arg(i).arg(j).arg(k).arg(faceText).arg(gridName));

                    if (faultName.isEmpty())
                    {
                        appendFaultName(hostGrid, conn.m_c2GlobIdx, cvf::StructGridInterface::oppositeFace(conn.m_c1Face), &faultName);
                    }
                }
            }

            resultInfo->append(QString("Face: %2\n").arg(face.text()));

            if (!faultName.isEmpty())
            {
                resultInfo->append(faultName);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///     
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateFaultForcedVisibility()
{
    // Force visibility of faults based on application state
    // As fault geometry is visible in grid visualization mode, fault geometry must be forced visible
    // even if the fault item is disabled in project tree view

    caf::FixedArray<bool, RivReservoirViewPartMgr::PROPERTY_FILTERED> forceOn;

    for (size_t i = 0; i < RivReservoirViewPartMgr::PROPERTY_FILTERED; i++)
    {
        forceOn[i] = false;
    }

    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultParts = visibleFaultParts();
    for (size_t i = 0; i < faultParts.size(); i++)
    {
        forceOn[faultParts[i]];
    }

    for (size_t i = 0; i < RivReservoirViewPartMgr::PROPERTY_FILTERED; i++)
    {
        RivReservoirViewPartMgr::ReservoirGeometryCacheType cacheType = (RivReservoirViewPartMgr::ReservoirGeometryCacheType)i;

        m_reservoirGridPartManager->setFaultForceVisibilityForGeometryType(cacheType, forceOn[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> RimReservoirView::visibleFaultParts() const
{
    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultParts;

    if (this->propertyFilterCollection()->hasActiveFilters() && !faultCollection()->showFaultsOutsideFilters)
    {
        faultParts.push_back(RivReservoirViewPartMgr::PROPERTY_FILTERED);
        faultParts.push_back(RivReservoirViewPartMgr::PROPERTY_FILTERED_WELL_CELLS);

        if (this->showInactiveCells())
        {
            faultParts.push_back(RivReservoirViewPartMgr::INACTIVE);
            faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
        }
    }
    else if (this->faultCollection()->showFaultsOutsideFilters())
    {
        faultParts.push_back(RivReservoirViewPartMgr::ACTIVE);
        faultParts.push_back(RivReservoirViewPartMgr::ALL_WELL_CELLS);
        faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED);
        faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);
        faultParts.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        faultParts.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);

        if (this->showInactiveCells())
        {
            faultParts.push_back(RivReservoirViewPartMgr::INACTIVE);
            faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
        }
    }
    else if (this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
    {
        faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED);
        faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);
        faultParts.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        faultParts.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);

        if (this->showInactiveCells())
        {
            faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
        }
    }
    else if (!this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
    {
        faultParts.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
        faultParts.push_back(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS);
    }
    else if (this->rangeFilterCollection()->hasActiveFilters() && !this->wellCollection()->hasVisibleWellCells())
    {
        faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED);
        faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_WELL_CELLS);

        if (this->showInactiveCells())
        {
            faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
        }
    }
    else
    {
        faultParts.push_back(RivReservoirViewPartMgr::ACTIVE);
        faultParts.push_back(RivReservoirViewPartMgr::ALL_WELL_CELLS);

        if (this->showInactiveCells())
        {
            faultParts.push_back(RivReservoirViewPartMgr::INACTIVE);
            faultParts.push_back(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);
        }
    }

    return faultParts;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::updateFaultColors()
{
    // Update all fault geometry
    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultGeometriesToRecolor = visibleFaultParts();

    for (size_t i = 0; i < faultGeometriesToRecolor.size(); ++i)
    {
        m_reservoirGridPartManager->updateFaultColors(faultGeometriesToRecolor[i], m_currentTimeStep, this->cellResult());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirView::appendFaultName(RigGridBase* grid, size_t cellIndex, cvf::StructGridInterface::FaceType face, QString* resultInfoText)
{
    if (grid->isMainGrid())
    {
        RigMainGrid* mainGrid = grid->mainGrid();

        for (size_t i = 0; i < mainGrid->faults().size(); i++)
        {
            const RigFault* rigFault = mainGrid->faults().at(i);
            const std::vector<RigFault::FaultFace>& faultFaces = rigFault->faultFaces();

            for (size_t fIdx = 0; fIdx < faultFaces.size(); fIdx++)
            {
                if (faultFaces[fIdx].m_nativeGlobalCellIndex == cellIndex)
                {
                    if (face == faultFaces[fIdx].m_nativeFace )
                    {
                        resultInfoText->append(QString("Fault Name: %1\n").arg(rigFault->name()));
                    }

                    return;
                }

                if (faultFaces[fIdx].m_oppositeGlobalCellIndex == cellIndex)
                {
                    if (face == cvf::StructGridInterface::oppositeFace(faultFaces[fIdx].m_nativeFace))
                    {
                        resultInfoText->append(QString("Fault Name: %1\n").arg(rigFault->name()));
                    }

                    return;
                }
            }
        }
    }
}
