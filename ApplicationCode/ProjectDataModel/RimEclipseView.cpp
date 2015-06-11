/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseView.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimEclipseCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimCellRangeFilterCollection.h"
#include "RimFaultCollection.h"
#include "RimFaultResultSlot.h"
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
#include "RivTernarySaturationOverlayItem.h"
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




CAF_PDM_SOURCE_INIT(RimEclipseView, "ReservoirView");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView::RimEclipseView()
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);

    CAF_PDM_InitObject("Reservoir View", ":/ReservoirView.png", "", "");
 
    CAF_PDM_InitFieldNoDefault(&cellResult,  "GridCellResult", "Cell Result", ":/CellResult.png", "", "");
    cellResult = new RimResultSlot();

    CAF_PDM_InitFieldNoDefault(&cellEdgeResult,  "GridCellEdgeResult", "Cell Edge Result", ":/EdgeResult_1.png", "", "");
    cellEdgeResult = new RimCellEdgeResultSlot();

    CAF_PDM_InitFieldNoDefault(&faultResultSettings,  "FaultResultSettings", "Separate Fault Result", "", "", "");
    faultResultSettings = new RimFaultResultSlot();

  
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

    // Visualization fields
    CAF_PDM_InitField(&showMainGrid,        "ShowMainGrid",         true,   "Show Main Grid",   "", "", "");
    CAF_PDM_InitField(&showInactiveCells,   "ShowInactiveCells",    false,  "Show Inactive Cells",   "", "", "");
    CAF_PDM_InitField(&showInvalidCells,    "ShowInvalidCells",     false,  "Show Invalid Cells",   "", "", "");
   
    this->cellResult()->setReservoirView(this);
    this->cellResult()->legendConfig()->setPosition(cvf::Vec2ui(10, 120));

    this->cellEdgeResult()->setReservoirView(this);
    this->cellEdgeResult()->legendConfig()->setReservoirView(this);
    this->cellEdgeResult()->legendConfig()->setPosition(cvf::Vec2ui(10, 320));
    this->cellEdgeResult()->legendConfig()->setColorRangeMode(RimLegendConfig::PINK_WHITE);

    this->faultResultSettings()->setReservoirView(this);

    m_reservoirGridPartManager = new RivReservoirViewPartMgr(this);

    m_pipesPartManager = new RivReservoirPipesPartMgr(this);
    m_reservoir = NULL;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView::~RimEclipseView()
{
    delete this->faultResultSettings();
    delete this->cellResult();
    delete this->cellEdgeResult();

    delete rangeFilterCollection();
    delete propertyFilterCollection();
    delete wellCollection();
    delete faultCollection();

    m_reservoirGridPartManager->clearGeometryCache();

    m_reservoir = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateViewerWidgetWindowTitle()
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
/// Clamp the current timestep to actual possibilities
//--------------------------------------------------------------------------------------------------
void RimEclipseView::clampCurrentTimestep()
{
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
void RimEclipseView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimView::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &showWindow)
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

        this->updateUiIconFromToggleField();
    }

    else if (changedField == &showInvalidCells)
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::INACTIVE);
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        createDisplayModelAndRedraw();
    }
    else if (changedField == &showInactiveCells)
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::INACTIVE);
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        createDisplayModelAndRedraw();
    }
    else if (changedField == &showMainGrid)
    {
        createDisplayModelAndRedraw();
    }
    else if (changedField == &rangeFilterCollection)
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        scheduleCreateDisplayModelAndRedraw();
    }
    else if (changedField == &propertyFilterCollection)
    {
        m_reservoirGridPartManager->scheduleGeometryRegen(RivReservoirViewPartMgr::PROPERTY_FILTERED);

        scheduleCreateDisplayModelAndRedraw();
    }


}

void RimEclipseView::updateScaleTransform()
{
    cvf::Mat4d scale = cvf::Mat4d::IDENTITY;
    scale(2, 2) = scaleZ();

    this->scaleTransform()->setLocalTransform(scale);
    m_pipesPartManager->setScaleTransform(this->scaleTransform());

    if (m_viewer) m_viewer->updateCachedValuesInScene();
}



//--------------------------------------------------------------------------------------------------
/// Create display model,
/// or at least empty scenes as frames that is delivered to the viewer
/// The real geometry generation is done inside RivReservoirViewGeometry and friends
//--------------------------------------------------------------------------------------------------
void RimEclipseView::createDisplayModel()
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

    if (isTimeStepDependentDataVisible())
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

    if (!this->propertyFilterCollection()->hasActiveFilters() || faultCollection()->showFaultsOutsideFilters())
    {
        updateFaultForcedVisibility();

        std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultGeometryTypesToAppend = visibleFaultGeometryTypes();

        RivReservoirViewPartMgr::ReservoirGeometryCacheType faultLabelType = m_reservoirGridPartManager->geometryTypeForFaultLabels(faultGeometryTypesToAppend);

        for (size_t frameIdx = 0; frameIdx < frameModels.size(); ++frameIdx)
        {
            for (size_t gtIdx = 0; gtIdx < faultGeometryTypesToAppend.size(); ++gtIdx)
            {
                m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameModels[frameIdx].p(), faultGeometryTypesToAppend[gtIdx]);
            }

            m_reservoirGridPartManager->appendFaultLabelsStaticGeometryPartsToModel(frameModels[frameIdx].p(), faultLabelType);
        }

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

    RimEclipseCase* eclCase = eclipseCase();
    RigCaseData* caseData = eclCase ? eclCase->reservoirData() : NULL;
    RigMainGrid* mainGrid = caseData ? caseData->mainGrid() : NULL;
    CVF_ASSERT(mainGrid);

    size_t frameIndex;
    for (frameIndex = 0; frameIndex < frameModels.size(); frameIndex++)
    {
        cvf::ModelBasicList* model = frameModels.at(frameIndex);
        model->updateBoundingBoxesRecursive();

        cvf::ref<cvf::Scene> scene = new cvf::Scene;
        scene->addModel(model);

        // Add well paths, if any
        addWellPathsToScene(scene.p(), mainGrid->displayModelOffset(), mainGrid->characteristicIJCellSize(), currentActiveCellInfo()->geometryBoundingBox(), m_reservoirGridPartManager->scaleTransform());

        if (frameIndex == 0)
            m_viewer->setMainScene(scene.p());
        else
            m_viewer->addFrame(scene.p());
    }

    // If the animation was active before recreating everything, make viewer view current frame

    if (isAnimationActive || cellResult->hasResult())
    {
        m_viewer->animationControl()->setCurrentFrame(m_currentTimeStep);
    }
    else
    {
        overlayInfoConfig()->update3DInfo();
        updateLegends();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateCurrentTimeStep()
{
    updateLegends(); // To make sure the scalar mappers are set up correctly

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

        if (faultCollection()->showFaultsOutsideFilters())
        {
            std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultGeometryTypesToAppend = visibleFaultGeometryTypes();

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

                if (!faultCollection()->showFaultsOutsideFilters())
                {
                    m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE); 
                }
            }
            else
            {
                m_reservoirGridPartManager->appendStaticGeometryPartsToModel(frameParts.p(), RivReservoirViewPartMgr::INACTIVE, gridIndices);

                if (!faultCollection()->showFaultsOutsideFilters())
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
        if (this->hasUserRequestedAnimation() && this->cellEdgeResult()->hasResult())
        {
			m_reservoirGridPartManager->updateCellEdgeResultColor(geometriesToRecolor[i], m_currentTimeStep, this->cellResult(), this->cellEdgeResult());
        } 
        else if ((this->hasUserRequestedAnimation() && this->cellResult()->hasResult()) || this->cellResult()->isTernarySaturationSelected())
        {
            m_reservoirGridPartManager->updateCellResultColor(geometriesToRecolor[i], m_currentTimeStep, this->cellResult());
        }
        else
        {
            this->updateStaticCellColors(geometriesToRecolor[i]);
        }
    }

    this->updateFaultColors();

    // Well pipes
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

            // Add well paths, if any
            RimEclipseCase* eclCase = eclipseCase();
            RigCaseData* caseData = eclCase ? eclCase->reservoirData() : NULL;
            RigMainGrid* mainGrid = caseData ? caseData->mainGrid() : NULL;
            CVF_ASSERT(mainGrid);
         
            addWellPathsToScene(frameScene, mainGrid->displayModelOffset(), mainGrid->characteristicIJCellSize(), currentActiveCellInfo()->geometryBoundingBox(), m_reservoirGridPartManager->scaleTransform());
        }
    }

    overlayInfoConfig()->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::loadDataAndUpdate()
{
    updateScaleTransform();

    if (m_reservoir)
    {
        if (!m_reservoir->openReserviorCase())
        {
            QMessageBox::warning(RiuMainWindow::instance(), 
                                "Error when opening project file", 
                                "Could not open the Eclipse Grid file: \n"+ m_reservoir->gridFileName());
            m_reservoir = NULL;
            return;
        }
    }

    CVF_ASSERT(this->cellResult() != NULL);
    this->cellResult()->loadResult();

    CVF_ASSERT(this->cellEdgeResult() != NULL);
    this->cellEdgeResult()->loadResult();

    this->faultResultSettings()->customFaultResult()->loadResult();
    this->faultResultSettings()->updateFieldVisibility();

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
void RimEclipseView::initAfterRead()
{
    this->faultResultSettings()->setReservoirView(this);
    this->cellResult()->setReservoirView(this);
    this->cellEdgeResult()->setReservoirView(this);
    this->rangeFilterCollection()->setReservoirView(this);
    this->propertyFilterCollection()->setReservoirView(this);

    this->updateUiIconFromToggleField();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateStaticCellColors()
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
void RimEclipseView::updateStaticCellColors(unsigned short geometryType)
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
void RimEclipseView::updateDisplayModelVisibility()
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
/// Convenience for quick access to results
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage* RimEclipseView::currentGridCellResults()
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
RigActiveCellInfo* RimEclipseView::currentActiveCellInfo()
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
void RimEclipseView::scheduleGeometryRegen(unsigned short geometryType)
{
    m_reservoirGridPartManager->scheduleGeometryRegen(static_cast<RivReservoirViewPartMgr::ReservoirGeometryCacheType>(geometryType));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::scheduleReservoirGridGeometryRegen()
{
    m_reservoirGridPartManager->clearGeometryCache();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseView::schedulePipeGeometryRegen()
{
    m_pipesPartManager->scheduleGeometryRegen();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::indicesToVisibleGrids(std::vector<size_t>* gridIndices)
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
void RimEclipseView::updateLegends()
{
    if (m_viewer)
    {
        m_viewer->removeAllColorLegends();
    }

    if (!m_reservoir || !m_viewer || !m_reservoir->reservoirData() )
    {
        return;
    }

    RigCaseData* eclipseCase = m_reservoir->reservoirData();
    CVF_ASSERT(eclipseCase);

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResult()->porosityModel());
    RigCaseCellResultsData* results = eclipseCase->results(porosityModel);
    CVF_ASSERT(results);

    updateMinMaxValuesAndAddLegendToView(QString("Cell Results: \n"), this->cellResult(), results);
    if (this->faultResultSettings()->showCustomFaultResult() && this->faultResultSettings()->hasValidCustomResult())
    {
        updateMinMaxValuesAndAddLegendToView(QString("Fault Results: \n"), this->currentFaultResultSlot(), results);
    }

    if (this->cellEdgeResult()->hasResult())
    {
        double globalMin, globalMax;
        double globalPosClosestToZero, globalNegClosestToZero;
        this->cellEdgeResult()->minMaxCellEdgeValues(globalMin, globalMax);
        this->cellEdgeResult()->posNegClosestToZero(globalPosClosestToZero, globalNegClosestToZero);

        this->cellEdgeResult()->legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, globalPosClosestToZero, globalNegClosestToZero);
        this->cellEdgeResult()->legendConfig->setAutomaticRanges(globalMin, globalMax, globalMin, globalMax);

        m_viewer->addColorLegendToBottomLeftCorner(this->cellEdgeResult()->legendConfig->legend());
        this->cellEdgeResult()->legendConfig->legend()->setTitle(cvfqt::Utils::toString(QString("Edge Results: \n") + this->cellEdgeResult()->resultVariable));
    }
    else
    {
        this->cellEdgeResult()->legendConfig->setClosestToZeroValues(0, 0, 0, 0);
        this->cellEdgeResult()->legendConfig->setAutomaticRanges(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateMinMaxValuesAndAddLegendToView(QString legendLabel, RimResultSlot* resultSlot, RigCaseCellResultsData* cellResultsData)
{
    if (resultSlot->hasResult())
    {
        double globalMin, globalMax;
        double globalPosClosestToZero, globalNegClosestToZero;
        cellResultsData->minMaxCellScalarValues(resultSlot->scalarResultIndex(), globalMin, globalMax);
        cellResultsData->posNegClosestToZero(resultSlot->scalarResultIndex(), globalPosClosestToZero, globalNegClosestToZero);

        double localMin, localMax;
        double localPosClosestToZero, localNegClosestToZero;
        if (resultSlot->hasDynamicResult())
        {
            cellResultsData->minMaxCellScalarValues(resultSlot->scalarResultIndex(), m_currentTimeStep, localMin, localMax);
            cellResultsData->posNegClosestToZero(resultSlot->scalarResultIndex(), m_currentTimeStep, localPosClosestToZero, localNegClosestToZero);
        }
        else
        {
            localMin = globalMin;
            localMax = globalMax;

            localPosClosestToZero = globalPosClosestToZero;
            localNegClosestToZero = globalNegClosestToZero;
        }

        resultSlot->legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
        resultSlot->legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

        m_viewer->addColorLegendToBottomLeftCorner(resultSlot->legendConfig->legend());
        resultSlot->legendConfig->legend()->setTitle(cvfqt::Utils::toString(legendLabel + resultSlot->resultVariable()));
    }
    else
    {
        resultSlot->legendConfig->setClosestToZeroValues(0, 0, 0, 0);
        resultSlot->legendConfig->setAutomaticRanges(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);
    }

    size_t maxTimeStepCount = cellResultsData->maxTimeStepCount();
    if (resultSlot->isTernarySaturationSelected() && maxTimeStepCount > 1)
    {
        RimReservoirCellResultsStorage* gridCellResults = resultSlot->currentGridCellResults();
        {
            double globalMin = 0.0;
            double globalMax = 1.0;
            double localMin = 0.0;
            double localMax = 1.0;

            size_t scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
            if (scalarSetIndex != cvf::UNDEFINED_SIZE_T)
            {
                cellResultsData->minMaxCellScalarValues(scalarSetIndex, globalMin, globalMax);
                cellResultsData->minMaxCellScalarValues(scalarSetIndex, m_currentTimeStep, localMin, localMax);

                resultSlot->ternaryLegendConfig()->setAutomaticRanges(RimTernaryLegendConfig::TERNARY_SOIL_IDX, globalMin, globalMax, localMin, localMax);
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
                cellResultsData->minMaxCellScalarValues(scalarSetIndex, globalMin, globalMax);
                cellResultsData->minMaxCellScalarValues(scalarSetIndex, m_currentTimeStep, localMin, localMax);

                resultSlot->ternaryLegendConfig()->setAutomaticRanges(RimTernaryLegendConfig::TERNARY_SGAS_IDX, globalMin, globalMax, localMin, localMax);
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
                cellResultsData->minMaxCellScalarValues(scalarSetIndex, globalMin, globalMax);
                cellResultsData->minMaxCellScalarValues(scalarSetIndex, m_currentTimeStep, localMin, localMax);

                resultSlot->ternaryLegendConfig()->setAutomaticRanges(RimTernaryLegendConfig::TERNARY_SWAT_IDX, globalMin, globalMax, localMin, localMax);
            }
        }

        if (resultSlot->ternaryLegendConfig->legend())
        {
            resultSlot->ternaryLegendConfig->legend()->setTitle(cvfqt::Utils::toString(legendLabel));
            m_viewer->addColorLegendToBottomLeftCorner(resultSlot->ternaryLegendConfig->legend());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::setEclipseCase(RimEclipseCase* reservoir)
{
    m_reservoir = reservoir;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseView::eclipseCase()
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
void RimEclipseView::syncronizeWellsWithResults()
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
void RimEclipseView::calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid)
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
                    size_t reservoirCellIndex = grid->reservoirCellIndex(gridCellIndex);

                    if (activeCellInfo->isActive(reservoirCellIndex))
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
                                    size_t reservoirCellIndex = grid->reservoirCellIndex(fenceCellIndex);

                                    if (activeCellInfo && activeCellInfo->isActive(reservoirCellIndex))
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
void RimEclipseView::updateDisplayModelForWellResults()
{
    m_reservoirGridPartManager->clearGeometryCache();
    m_pipesPartManager->clearGeometryCache();

    syncronizeWellsWithResults();

    createDisplayModel();
    updateDisplayModelVisibility();

    if (hasUserRequestedAnimation() && m_viewer)
    {
        m_viewer->animationControl()->setCurrentFrame(m_currentTimeStep);
    }

    RiuMainWindow::instance()->refreshAnimationActions(); 
   
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
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
void RimEclipseView::updateFaultForcedVisibility()
{
    // Force visibility of faults based on application state
    // As fault geometry is visible in grid visualization mode, fault geometry must be forced visible
    // even if the fault item is disabled in project tree view

    if (!faultCollection->showFaultCollection)
    {
        m_reservoirGridPartManager->setFaultForceVisibilityForGeometryType(RivReservoirViewPartMgr::ALL_WELL_CELLS, true);
    }

    m_reservoirGridPartManager->setFaultForceVisibilityForGeometryType(RivReservoirViewPartMgr::RANGE_FILTERED, true);
    m_reservoirGridPartManager->setFaultForceVisibilityForGeometryType(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS, true);
    m_reservoirGridPartManager->setFaultForceVisibilityForGeometryType(RivReservoirViewPartMgr::VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> RimEclipseView::visibleFaultGeometryTypes() const
{
    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultParts;

    if (this->propertyFilterCollection()->hasActiveFilters() && !faultCollection()->showFaultsOutsideFilters())
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
void RimEclipseView::updateFaultColors()
{
    // Update all fault geometry
    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> faultGeometriesToRecolor = visibleFaultGeometryTypes();

    RimResultSlot* faultResultSlot = currentFaultResultSlot();

    for (size_t i = 0; i < faultGeometriesToRecolor.size(); ++i)
    {
		if (this->hasUserRequestedAnimation() && this->cellEdgeResult()->hasResult())
		{
			m_reservoirGridPartManager->updateFaultCellEdgeResultColor(faultGeometriesToRecolor[i], m_currentTimeStep, faultResultSlot, this->cellEdgeResult());
		}
		else
		{
			m_reservoirGridPartManager->updateFaultColors(faultGeometriesToRecolor[i], m_currentTimeStep, faultResultSlot);
		}
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::isTimeStepDependentDataVisible() const
{
    if (this->cellResult()->hasDynamicResult()) return true;

    if (this->propertyFilterCollection()->hasActiveDynamicFilters()) return true;
        
    if (this->wellCollection->hasVisibleWellPipes()) return true;

    if (this->cellResult()->isTernarySaturationSelected()) return true;
    
    if (this->faultResultSettings->showCustomFaultResult())
    {
        if (this->faultResultSettings->customFaultResult()->hasDynamicResult()) return true;

        if (this->faultResultSettings->customFaultResult()->isTernarySaturationSelected()) return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultSlot* RimEclipseView::currentFaultResultSlot()
{
    RimResultSlot* faultResultSlot = this->cellResult();

    if (this->faultResultSettings()->showCustomFaultResult())
    {
        faultResultSlot = this->faultResultSettings()->customFaultResult();
    }

    return faultResultSlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::resetLegendsInViewer()
{
    this->cellResult()->legendConfig->recreateLegend();
    this->cellResult()->ternaryLegendConfig->recreateLegend();
    this->cellEdgeResult()->legendConfig->recreateLegend();

    m_viewer->removeAllColorLegends();
    m_viewer->addColorLegendToBottomLeftCorner(this->cellResult()->legendConfig->legend());
    m_viewer->addColorLegendToBottomLeftCorner(this->cellEdgeResult()->legendConfig->legend());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Transform* RimEclipseView::scaleTransform()
{
    return m_reservoirGridPartManager->scaleTransform();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* RimEclipseView::ownerCase()
{
    return eclipseCase();
}

