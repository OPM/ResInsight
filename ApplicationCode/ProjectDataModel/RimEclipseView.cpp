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
#include "RiaColorTables.h"
#include "RiaPreferences.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigVirtualPerforationTransmissibilities.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimFaultInViewCollection.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowDiagSolution.h"
#include "RimGridCollection.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimLegendConfig.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanColors.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimVirtualPerforationResults.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "RivReservoirSimWellsPartMgr.h"
#include "RivReservoirViewPartMgr.h"
#include "RivSingleCellPartGenerator.h"
#include "RivTernarySaturationOverlayItem.h"
#include "RivWellPathsPartMgr.h" 

#include "RimFracture.h"
#include "RimFractureTemplateCollection.h"
#include "RimSimWellFracture.h"
#include "RivWellFracturePartMgr.h"


#include "cafCadNavigation.h"
#include "cafCeetronPlusNavigation.h"
#include "cafDisplayCoordTransform.h"
#include "cafFrameAnimationControl.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafOverlayScalarMapperLegend.h"

#include "cvfDrawable.h"
#include "cvfModelBasicList.h"
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

    CAF_PDM_InitObject("Reservoir View", ":/3DView16x16.png", "", "");
 
    CAF_PDM_InitFieldNoDefault(&m_cellResult,  "GridCellResult", "Cell Result", ":/CellResult.png", "", "");
    m_cellResult = new RimEclipseCellColors();
    m_cellResult.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_cellEdgeResult,  "GridCellEdgeResult", "Cell Edge Result", ":/EdgeResult_1.png", "", "");
    m_cellEdgeResult = new RimCellEdgeColors();
    m_cellEdgeResult.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_faultResultSettings,  "FaultResultSettings", "Separate Fault Result", "", "", "");
    m_faultResultSettings = new RimEclipseFaultColors();
    m_faultResultSettings.uiCapability()->setUiHidden(true);
  
    CAF_PDM_InitFieldNoDefault(&m_fractureColors, "StimPlanColors", "Fracture", "", "", "");
    m_fractureColors = new RimStimPlanColors();
    m_fractureColors.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_virtualPerforationResult, "VirtualPerforationResult", "", "", "", "");
    m_virtualPerforationResult = new RimVirtualPerforationResults();
    m_virtualPerforationResult.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellCollection, "WellCollection", "Simulation Wells", "", "", "");
    m_wellCollection = new RimSimWellInViewCollection;
    m_wellCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_faultCollection, "FaultCollection", "Faults", "", "", "");
    m_faultCollection = new RimFaultInViewCollection;
    m_faultCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_propertyFilterCollection, "PropertyFilters", "Property Filters", "", "", "");
    m_propertyFilterCollection = new RimEclipsePropertyFilterCollection();
    m_propertyFilterCollection.uiCapability()->setUiHidden(true);

    // Visualization fields
    CAF_PDM_InitField(&m_showMainGrid,        "ShowMainGrid",         true,   "Show Main Grid",   "", "", "");
    CAF_PDM_InitField(&m_showInactiveCells,   "ShowInactiveCells",    false,  "Show Inactive Cells",   "", "", "");
    CAF_PDM_InitField(&m_showInvalidCells,    "ShowInvalidCells",     false,  "Show Invalid Cells",   "", "", "");
   
    this->cellResult()->setReservoirView(this);

    this->cellEdgeResult()->setReservoirView(this);
    this->cellEdgeResult()->legendConfig()->setColorRangeMode(RimLegendConfig::PINK_WHITE);

    this->faultResultSettings()->setReservoirView(this);

    m_reservoirGridPartManager = new RivReservoirViewPartMgr(this);
    m_simWellsPartManager = new RivReservoirSimWellsPartMgr(this);
    
    m_eclipseCase = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView::~RimEclipseView()
{
    delete this->faultResultSettings();
    delete this->cellResult();
    delete this->cellEdgeResult();

    delete m_propertyFilterCollection;
    delete wellCollection();
    delete faultCollection();

    m_reservoirGridPartManager->clearGeometryCache();

    m_eclipseCase = nullptr;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors* RimEclipseView::cellResult() const
{
    return m_cellResult;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors* RimEclipseView::cellEdgeResult() const
{
    return m_cellEdgeResult;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseFaultColors* RimEclipseView::faultResultSettings() const
{
    return m_faultResultSettings;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanColors* RimEclipseView::fractureColors() const
{
    return m_fractureColors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInViewCollection* RimEclipseView::wellCollection() const
{
    return m_wellCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultInViewCollection* RimEclipseView::faultCollection() const
{
    return m_faultCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimVirtualPerforationResults* RimEclipseView::virtualPerforationResult() const
{
    return m_virtualPerforationResult();
}

//--------------------------------------------------------------------------------------------------
/// Clamp the current timestep to actual possibilities
//--------------------------------------------------------------------------------------------------
void RimEclipseView::clampCurrentTimestep()
{
    if (this->currentGridCellResults()) 
    {
        if (m_currentTimeStep() >= static_cast<int>(this->currentGridCellResults()->maxTimeStepCount()))
        {
            m_currentTimeStep = static_cast<int>(this->currentGridCellResults()->maxTimeStepCount()) -1;
        }
    }

    if (m_currentTimeStep < 0 ) m_currentTimeStep = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::setVisibleGridParts(const std::vector<RivCellSetEnum>& cellSets)
{
    m_visibleGridParts = cellSets;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::setVisibleGridPartsWatertight()
{
    for (RivCellSetEnum cellSetType : m_visibleGridParts) 
    { 
        m_reservoirGridPartManager->forceWatertightGeometryOnForType(cellSetType); 
    } 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    Rim3dView::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_showInvalidCells)
    {
        this->scheduleGeometryRegen(INACTIVE);
        this->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

        scheduleCreateDisplayModelAndRedraw();
    }
    else if (changedField == &m_showInactiveCells)
    {
        this->updateGridBoxData();
        
        this->scheduleGeometryRegen(INACTIVE);
        this->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

        scheduleCreateDisplayModelAndRedraw();
    }
    else if (changedField == &m_showMainGrid)
    {
        scheduleCreateDisplayModelAndRedraw();
    }
    else if (changedField == &m_rangeFilterCollection)
    {
        this->scheduleGeometryRegen(RANGE_FILTERED);
        this->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

        scheduleCreateDisplayModelAndRedraw();
    }
    else if (changedField == &m_propertyFilterCollection)
    {
        this->scheduleGeometryRegen(PROPERTY_FILTERED);

        scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateScaleTransform()
{
    cvf::Mat4d scale = cvf::Mat4d::IDENTITY;
    scale(2, 2) = scaleZ();

    this->scaleTransform()->setLocalTransform(scale);
    m_simWellsPartManager->setScaleTransform(this->scaleTransform());

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

#if 0 // Debug info
    static int callCount = 0;
    std::cout << "RimReservoirView::createDisplayModel() " << callCount++ << std::endl;
    RiuMainWindow::instance()->setResultInfo(QString("RimReservoirView::createDisplayModel() ") + QString::number(callCount++));
#endif

    if (!(m_eclipseCase && m_eclipseCase->eclipseCaseData())) return;

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
        for (i = 0; i < currentGridCellResults()->maxTimeStepCount(); i++)
        {
            timeStepIndices.push_back(i);
        }
    } 
    else if (this->cellResult()->hasStaticResult() 
        || this->cellEdgeResult()->hasResult() 
        || this->eclipsePropertyFilterCollection()->hasActiveFilters())
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
    m_viewer->removeAllFrames();

    wellCollection()->scheduleIsWellPipesVisibleRecalculation();

    // Create vector of grid indices to render
    std::vector<size_t> gridIndices;
    this->indicesToVisibleGrids(&gridIndices);

    ///
    // Get or create the parts for "static" type geometry. The same geometry is used 
    // for the different frames. updateCurrentTimeStep updates the colors etc.
    // For property filtered geometry : just set all the models as empty scenes 
    // updateCurrentTimeStep requests the actual parts
    

    if (!this->eclipsePropertyFilterCollection()->hasActiveFilters()
        || ( this->viewController()
             && this->viewController()->isVisibleCellsOveridden()) )
    {
        std::vector<RivCellSetEnum> geometryTypesToAdd;

        if (this->viewController() && this->viewController()->isVisibleCellsOveridden())
        {
            geometryTypesToAdd.push_back(OVERRIDDEN_CELL_VISIBILITY);
        }
        else if (this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
        {
            geometryTypesToAdd.push_back(RANGE_FILTERED);
            geometryTypesToAdd.push_back(RANGE_FILTERED_WELL_CELLS);
            geometryTypesToAdd.push_back(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
            geometryTypesToAdd.push_back(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
            if (this->showInactiveCells())
            {
                geometryTypesToAdd.push_back(RANGE_FILTERED_INACTIVE);
            }
        }
        else if (!this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
        {
            geometryTypesToAdd.push_back(VISIBLE_WELL_CELLS);
            geometryTypesToAdd.push_back(VISIBLE_WELL_FENCE_CELLS);
        }
        else if (this->rangeFilterCollection()->hasActiveFilters() && !this->wellCollection()->hasVisibleWellCells())
        {
            geometryTypesToAdd.push_back(RANGE_FILTERED);
            geometryTypesToAdd.push_back(RANGE_FILTERED_WELL_CELLS);
            if (this->showInactiveCells())
            {
                geometryTypesToAdd.push_back(RANGE_FILTERED_INACTIVE);
            }
        }
        else
        {
            geometryTypesToAdd.push_back(ALL_WELL_CELLS); // Should be all well cells
            geometryTypesToAdd.push_back(ACTIVE);

            if (this->showInactiveCells())
            {
                geometryTypesToAdd.push_back(INACTIVE);
            }
        }

        // NOTE: This assignment must be done here, as m_visibleGridParts is used in code triggered by 
        // m_reservoirGridPartManager->appendStaticGeometryPartsToModel()
        setVisibleGridParts(geometryTypesToAdd);

        size_t frameIdx;
        for (frameIdx = 0; frameIdx < frameModels.size(); ++frameIdx)
        {
            for (size_t gtIdx = 0; gtIdx < geometryTypesToAdd.size(); ++gtIdx)
            {
                if ( isGridVisualizationMode() )
                {
                    m_reservoirGridPartManager->appendStaticGeometryPartsToModel(frameModels[frameIdx].p(), geometryTypesToAdd[gtIdx], gridIndices);
                }
                else
                {
                    m_reservoirGridPartManager->ensureStaticGeometryPartsCreated( geometryTypesToAdd[gtIdx]);
                }
            }
        }
        // Set static colors 
        this->updateStaticCellColors();
    }
    else 
    {
        std::vector<RivCellSetEnum> empty;
        setVisibleGridParts(empty);
    }

    m_reservoirGridPartManager->clearWatertightGeometryFlags();

    if (   faultCollection()->showFaultCollection()
        || !this->eclipsePropertyFilterCollection()->hasActiveFilters() )
    {
        setVisibleGridPartsWatertight();

        std::set<RivCellSetEnum> faultGeometryTypesToAppend = allVisibleFaultGeometryTypes();
        RivCellSetEnum faultLabelType = m_reservoirGridPartManager->geometryTypeForFaultLabels(faultGeometryTypesToAppend, faultCollection()->isShowingFaultsAndFaultsOutsideFilters());

        for (size_t frameIdx = 0; frameIdx < frameModels.size(); ++frameIdx)
        {
            for (RivCellSetEnum geometryType : faultGeometryTypesToAppend)
            {
                if (geometryType == PROPERTY_FILTERED || geometryType == PROPERTY_FILTERED_WELL_CELLS) continue;
           
                m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameModels[frameIdx].p(), geometryType);
            }

            m_reservoirGridPartManager->appendFaultLabelsStaticGeometryPartsToModel(frameModels[frameIdx].p(), faultLabelType);
        }
    }


    // Cross sections

    m_crossSectionVizModel->removeAllParts();
    m_crossSectionCollection->appendPartsToModel(*this, m_crossSectionVizModel.p(), m_reservoirGridPartManager->scaleTransform());
    m_viewer->addStaticModelOnce(m_crossSectionVizModel.p());

    // Well path model

    m_wellPathPipeVizModel->removeAllParts();

    // NB! StimPlan legend colors must be updated before well path geometry is added to the model
    // as the fracture geometry depends on the StimPlan legend colors
    fractureColors()->updateLegendData();

    addWellPathsToModel(m_wellPathPipeVizModel.p(), currentActiveCellInfo()->geometryBoundingBox());

    m_wellPathsPartManager->appendStaticFracturePartsToModel(m_wellPathPipeVizModel.p());
    m_wellPathPipeVizModel->updateBoundingBoxesRecursive();
    m_viewer->addStaticModelOnce(m_wellPathPipeVizModel.p());

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

    if (frameModels.size() > 1 && this->hasUserRequestedAnimation())
    {
        m_viewer->setCurrentFrame(m_currentTimeStep);
    }
    else
    {
        m_overlayInfoConfig()->update3DInfo();
        updateLegends();
    }

    std::vector<caf::PdmFieldHandle*> objects;
    this->referringPtrFields(objects);
    for (auto object : objects)
    {
        RimFlowCharacteristicsPlot* plot = dynamic_cast<RimFlowCharacteristicsPlot*>(object->ownerObject());
        if (plot != nullptr)
        {
            plot->viewGeometryUpdated();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateCurrentTimeStep()
{
    m_propertyFilterCollection()->updateFromCurrentTimeStep();

    updateLegends(); // To make sure the scalar mappers are set up correctly

    std::vector<RivCellSetEnum> geometriesToRecolor;

    if (this->viewController() && this->viewController()->isVisibleCellsOveridden())
    {
        geometriesToRecolor.push_back(OVERRIDDEN_CELL_VISIBILITY);
    }
    else if (this->eclipsePropertyFilterCollection()->hasActiveFilters())
    {
        cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
        frameParts->setName("GridModel");

        std::vector<size_t> gridIndices;
        this->indicesToVisibleGrids(&gridIndices);

        geometriesToRecolor.push_back( PROPERTY_FILTERED);
        geometriesToRecolor.push_back( PROPERTY_FILTERED_WELL_CELLS);

        if ( isGridVisualizationMode() )
        {
            m_reservoirGridPartManager->appendDynamicGeometryPartsToModel(frameParts.p(), PROPERTY_FILTERED, m_currentTimeStep, gridIndices);
            m_reservoirGridPartManager->appendDynamicGeometryPartsToModel(frameParts.p(), PROPERTY_FILTERED_WELL_CELLS, m_currentTimeStep, gridIndices);
        }
        else
        {
            m_reservoirGridPartManager->ensureDynamicGeometryPartsCreated(PROPERTY_FILTERED, m_currentTimeStep);
            m_reservoirGridPartManager->ensureDynamicGeometryPartsCreated(PROPERTY_FILTERED_WELL_CELLS, m_currentTimeStep);
        }

        setVisibleGridParts(geometriesToRecolor);
        setVisibleGridPartsWatertight();

        std::set<RivCellSetEnum> faultGeometryTypesToAppend = allVisibleFaultGeometryTypes();
        for (RivCellSetEnum geometryType : faultGeometryTypesToAppend)
        {
            if (geometryType == PROPERTY_FILTERED || geometryType == PROPERTY_FILTERED_WELL_CELLS)
            {
                m_reservoirGridPartManager->appendFaultsDynamicGeometryPartsToModel(frameParts.p(), geometryType, m_currentTimeStep);
            }
            else
            {
                m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameParts.p(), geometryType);
            }
        }

        RivCellSetEnum faultLabelType = m_reservoirGridPartManager->geometryTypeForFaultLabels(faultGeometryTypesToAppend, faultCollection()->isShowingFaultsAndFaultsOutsideFilters());
        if (faultLabelType == PROPERTY_FILTERED)
        {
            m_reservoirGridPartManager->appendFaultLabelsDynamicGeometryPartsToModel(frameParts.p(), faultLabelType, m_currentTimeStep);
        }
        else
        {
            m_reservoirGridPartManager->appendFaultLabelsStaticGeometryPartsToModel(frameParts.p(), faultLabelType);
        }

        // Set the transparency on all the Wellcell parts before setting the result color
        float opacity = static_cast< float> (1 - cvf::Math::clamp(this->wellCollection()->wellCellTransparencyLevel(), 0.0, 1.0));
        m_reservoirGridPartManager->updateCellColor(PROPERTY_FILTERED_WELL_CELLS, m_currentTimeStep, cvf::Color4f(cvf::Color3f(cvf::Color3::WHITE), opacity));


        if (this->showInactiveCells())
        {
            std::vector<size_t> gridIndices;
            this->indicesToVisibleGrids(&gridIndices);
 
            if (this->rangeFilterCollection()->hasActiveFilters() ) // Wells not considered, because we do not have a INACTIVE_WELL_CELLS group yet.
            {
                m_reservoirGridPartManager->appendStaticGeometryPartsToModel(frameParts.p(), RANGE_FILTERED_INACTIVE, gridIndices); 

                if (!faultCollection()->isShowingFaultsAndFaultsOutsideFilters())
                {
                    m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameParts.p(), RANGE_FILTERED_INACTIVE); 
                }
            }
            else
            {
                m_reservoirGridPartManager->appendStaticGeometryPartsToModel(frameParts.p(), INACTIVE, gridIndices);

                if (!faultCollection()->isShowingFaultsAndFaultsOutsideFilters())
                {
                    m_reservoirGridPartManager->appendFaultsStaticGeometryPartsToModel(frameParts.p(), INACTIVE);
                }
            }
        }

        if (m_viewer)
        {
            cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
            if (frameScene)
            {
                this->removeModelByName(frameScene, frameParts->name());
                frameScene->addModel(frameParts.p());
                frameParts->updateBoundingBoxesRecursive();
            }
        }
    }
    else if (this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
    {
        geometriesToRecolor.push_back(RANGE_FILTERED);
        geometriesToRecolor.push_back(RANGE_FILTERED_WELL_CELLS);
        geometriesToRecolor.push_back(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        geometriesToRecolor.push_back(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
    }
    else if (!this->rangeFilterCollection()->hasActiveFilters() && this->wellCollection()->hasVisibleWellCells())
    {
        geometriesToRecolor.push_back(VISIBLE_WELL_CELLS);
        geometriesToRecolor.push_back(VISIBLE_WELL_FENCE_CELLS);
    }
    else if (this->rangeFilterCollection()->hasActiveFilters() && !this->wellCollection()->hasVisibleWellCells())
    {
        geometriesToRecolor.push_back(RANGE_FILTERED);
        geometriesToRecolor.push_back(RANGE_FILTERED_WELL_CELLS);
    }
    else 
    {
        geometriesToRecolor.push_back(ACTIVE);
        geometriesToRecolor.push_back(ALL_WELL_CELLS);
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


    if ((this->hasUserRequestedAnimation() && this->cellResult()->hasResult()) || this->cellResult()->isTernarySaturationSelected())
    {
        m_crossSectionCollection->updateCellResultColor(m_currentTimeStep, 
                                                        this->cellResult()->legendConfig()->scalarMapper(),
                                                        this->cellResult()->ternaryLegendConfig()->scalarMapper());
    }
    else
    {
        m_crossSectionCollection->applySingleColorEffect();
    }

    if (m_viewer)
    {
        cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
        if (frameScene)
        {
            // Simulation Wells
            {
                cvf::String name = "SimWellPipeMod";
                this->removeModelByName(frameScene, name);

                cvf::ref<cvf::ModelBasicList> simWellModelBasicList = new cvf::ModelBasicList;
                simWellModelBasicList->setName(name);

                m_simWellsPartManager->appendDynamicGeometryPartsToModel(simWellModelBasicList.p(), m_currentTimeStep);

                simWellModelBasicList->updateBoundingBoxesRecursive();

                frameScene->addModel(simWellModelBasicList.p());

                m_simWellsPartManager->updatePipeResultColor(m_currentTimeStep);
            }

            // Well Paths
            {
                cvf::String name = "WellPathMod";
                this->removeModelByName(frameScene, name);
                
                cvf::ref<cvf::ModelBasicList> wellPathModelBasicList = new cvf::ModelBasicList;
                wellPathModelBasicList->setName(name);

                addDynamicWellPathsToModel(wellPathModelBasicList.p(), currentActiveCellInfo()->geometryBoundingBox());

                frameScene->addModel(wellPathModelBasicList.p());
            }

            // Sim Well Fractures
            {
                cvf::String name = "SimWellFracturesModel";
                this->removeModelByName(frameScene, name);

                cvf::ref<cvf::ModelBasicList> simWellFracturesModelBasicList = new cvf::ModelBasicList;
                simWellFracturesModelBasicList->setName(name);

                cvf::ref<caf::DisplayCoordTransform> transForm = this->displayCoordTransform();

                std::vector<RimFracture*> fractures;
                this->descendantsIncludingThisOfType(fractures);
                for (RimFracture* f : fractures)
                {
                    RimSimWellInView* simWell = nullptr;
                    f->firstAncestorOrThisOfType(simWell);
                    if (simWell)
                    {
                        bool isAnyGeometryPresent = simWell->isWellPipeVisible(m_currentTimeStep) || simWell->isWellSpheresVisible(m_currentTimeStep);
                        if (!isAnyGeometryPresent)
                        {
                            continue;
                        }
                    }

                    f->fracturePartManager()->appendGeometryPartsToModel(simWellFracturesModelBasicList.p(), *this);
                }

                simWellFracturesModelBasicList->updateBoundingBoxesRecursive();
                frameScene->addModel(simWellFracturesModelBasicList.p());
            }
        }
    }

    m_overlayInfoConfig()->update3DInfo();

    // Invisible Wells are marked as read only when "show wells intersecting visible cells" is enabled
    // Visibility of wells differ betweeen time steps, so trigger a rebuild of tree state items
    wellCollection()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::onLoadDataAndUpdate()
{
    updateScaleTransform();

    if (m_eclipseCase)
    {
        if (!m_eclipseCase->openReserviorCase())
        {
            QMessageBox::warning(RiuMainWindow::instance(), 
                                "Error when opening project file", 
                                "Could not open the Eclipse Grid file: \n"+ m_eclipseCase->gridFileName());
            this->setEclipseCase( nullptr);
            return;
        }
    }

    CVF_ASSERT(this->cellResult() != nullptr);
    this->cellResult()->loadResult();

    CVF_ASSERT(this->cellEdgeResult() != nullptr);
    this->cellEdgeResult()->loadResult();

    this->faultResultSettings()->customFaultResult()->loadResult();
    this->fractureColors()->loadDataAndUpdate();

    updateMdiWindowVisibility();

    this->m_propertyFilterCollection()->loadAndInitializePropertyFilters();

    this->faultCollection()->setReservoirView(this);
    this->faultCollection()->syncronizeFaults();

    m_reservoirGridPartManager->clearGeometryCache();
    m_simWellsPartManager->clearGeometryCache();

    syncronizeWellsWithResults();
    
    {
        // Update simulation well fractures after well cell results are imported
        
        std::vector<RimSimWellFracture*> simFractures;
        this->descendantsIncludingThisOfType(simFractures);
        for (auto fracture : simFractures)
        {
            fracture->loadDataAndUpdate();
        }
    }

    if (this->isVirtualConnectionFactorGeometryVisible())
    {
        m_virtualPerforationResult->loadData();
    }

    this->scheduleCreateDisplayModelAndRedraw();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::initAfterRead()
{
    RimViewWindow::initAfterRead();

    this->faultResultSettings()->setReservoirView(this);
    this->cellResult()->setReservoirView(this);
    this->cellEdgeResult()->setReservoirView(this);

    this->updateUiIconFromToggleField();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateStaticCellColors()
{
    updateStaticCellColors( OVERRIDDEN_CELL_VISIBILITY);
    updateStaticCellColors( ACTIVE);
    updateStaticCellColors( ALL_WELL_CELLS);
    updateStaticCellColors( VISIBLE_WELL_CELLS);
    updateStaticCellColors( VISIBLE_WELL_FENCE_CELLS);
    updateStaticCellColors( VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
    updateStaticCellColors( VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
    updateStaticCellColors( INACTIVE);
    updateStaticCellColors( RANGE_FILTERED);
    updateStaticCellColors( RANGE_FILTERED_WELL_CELLS);
    updateStaticCellColors( RANGE_FILTERED_INACTIVE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateStaticCellColors(RivCellSetEnum geometryType)
{
    float opacity = static_cast< float> (1 - cvf::Math::clamp(this->wellCollection()->wellCellTransparencyLevel(), 0.0, 1.0));
    cvf::Color4f color(cvf::Color3::ORANGE);

    switch (geometryType)
    {
        case ACTIVE:                      color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case ALL_WELL_CELLS:              color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case VISIBLE_WELL_CELLS:          color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case VISIBLE_WELL_FENCE_CELLS:    color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER:         
                                                                    color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER:   
                                                                    color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case INACTIVE:                    color = cvf::Color4f(RiaColorTables::undefinedCellColor());  break;
        case RANGE_FILTERED:              color = cvf::Color4f(cvf::Color3::ORANGE);      break;
        case RANGE_FILTERED_WELL_CELLS:   color = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN), opacity ); break;
        case RANGE_FILTERED_INACTIVE:     color = cvf::Color4f(RiaColorTables::undefinedCellColor());  break;   
    }

    if (geometryType == PROPERTY_FILTERED || geometryType == PROPERTY_FILTERED_WELL_CELLS)
    {
        // Always use current time step when updating color of property geometry
        m_reservoirGridPartManager->updateCellColor(geometryType, m_currentTimeStep, color);
    }
    else
    {
        // Use static timestep (timestep 0) for geometry with no change between time steps
        m_reservoirGridPartManager->updateCellColor(geometryType, 0, color);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateDisplayModelVisibility()
{
    Rim3dView::updateDisplayModelVisibility();

    faultCollection()->updateConnectedEditors();

    // This is required to update the read-only state of simulation wells
    // when a range filter is manipulated and visible simulation wells might change
    //
    // The visibility is controlled by RimEclipseWell::defineUiTreeOrdering
    // updateConnectedEditors will call recursively on child objects
    wellCollection()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Convenience for quick access to results
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData* RimEclipseView::currentGridCellResults()
{
    if (m_eclipseCase)
    {
        return m_eclipseCase->results(cellResult()->porosityModel());
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimEclipseView::currentActiveCellInfo()
{
    if (m_eclipseCase &&
        m_eclipseCase->eclipseCaseData()
        )
    {
        return m_eclipseCase->eclipseCaseData()->activeCellInfo(cellResult()->porosityModel());
    }

    return nullptr;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::scheduleGeometryRegen(RivCellSetEnum geometryType)
{
    m_reservoirGridPartManager->scheduleGeometryRegen(geometryType);

    if (this->isMasterView())
    {
        RimViewLinker* viewLinker = this->assosiatedViewLinker();
        if (viewLinker)
        {
            viewLinker->scheduleGeometryRegenForDepViews(geometryType);
        }
    }

    m_currentReservoirCellVisibility = nullptr;
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
void RimEclipseView::scheduleSimWellGeometryRegen()
{
    m_simWellsPartManager->scheduleGeometryRegen();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::indicesToVisibleGrids(std::vector<size_t>* gridIndices)
{
    CVF_ASSERT(gridIndices != nullptr);

    // Create vector of grid indices to render
    std::vector<RigGridBase*> grids;
    if (this->m_eclipseCase && this->m_eclipseCase->eclipseCaseData() )
    {
        this->m_eclipseCase->eclipseCaseData()->allGrids(&grids);
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

    if (!m_eclipseCase || !m_viewer || !m_eclipseCase->eclipseCaseData() )
    {
        return;
    }

    RigEclipseCaseData* eclipseCase = m_eclipseCase->eclipseCaseData();
    CVF_ASSERT(eclipseCase);

    RigCaseCellResultsData* results = eclipseCase->results(cellResult()->porosityModel());
    CVF_ASSERT(results);

    updateMinMaxValuesAndAddLegendToView(QString("Cell Results: \n"), this->cellResult(), results);

    if (this->faultResultSettings()->showCustomFaultResult() && this->faultResultSettings()->hasValidCustomResult())
    {
        updateMinMaxValuesAndAddLegendToView(QString("Fault Results: \n"), this->currentFaultResultColors(), results);
    }

    if (this->cellEdgeResult()->legendConfig()->enableLegend())
    {
        if (this->cellEdgeResult()->hasResult())
        {
            if (this->cellEdgeResult()->isUsingSingleVariable())
            {
                this->cellEdgeResult()->singleVarEdgeResultColors()->updateLegendData(m_currentTimeStep);
            }
            else
            {
                double globalMin, globalMax;
                double globalPosClosestToZero, globalNegClosestToZero;
                this->cellEdgeResult()->minMaxCellEdgeValues(globalMin, globalMax);
                this->cellEdgeResult()->posNegClosestToZero(globalPosClosestToZero, globalNegClosestToZero);

                this->cellEdgeResult()->legendConfig()->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, globalPosClosestToZero, globalNegClosestToZero);
                this->cellEdgeResult()->legendConfig()->setAutomaticRanges(globalMin, globalMax, globalMin, globalMax);

                if (this->cellEdgeResult()->hasCategoryResult())
                {
                    if (cellEdgeResult()->singleVarEdgeResultColors()->resultType() != RiaDefines::FORMATION_NAMES)
                    {
                        cellEdgeResult()->legendConfig()->setIntegerCategories(results->uniqueCellScalarValues(cellEdgeResult()->singleVarEdgeResultColors()->scalarResultIndex()));
                    }
                    else
                    {
                        const std::vector<QString>& fnVector = eclipseCase->activeFormationNames()->formationNames();
                        cellEdgeResult()->legendConfig()->setNamedCategoriesInverse(fnVector);
                    }
                }
            }

            m_viewer->addColorLegendToBottomLeftCorner(this->cellEdgeResult()->legendConfig()->legend());
            this->cellEdgeResult()->legendConfig()->setTitle(QString("Edge Results: \n") + this->cellEdgeResult()->resultVariableUiShortName());
        }
        else
        {
            this->cellEdgeResult()->legendConfig()->setClosestToZeroValues(0, 0, 0, 0);
            this->cellEdgeResult()->legendConfig()->setAutomaticRanges(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);
        }
    }

    RimLegendConfig* stimPlanLegend = fractureColors()->activeLegend();
    if (stimPlanLegend && stimPlanLegend->enableLegend())
    {
        fractureColors()->updateLegendData();
        
        if (fractureColors()->isChecked() && stimPlanLegend->legend())
        {
            m_viewer->addColorLegendToBottomLeftCorner(stimPlanLegend->legend());
        }
    }
    
    if (m_virtualPerforationResult->isActive() && m_virtualPerforationResult->legendConfig()->enableLegend())
    {
        updateVirtualConnectionLegendRanges();

        RimLegendConfig* virtLegend = m_virtualPerforationResult->legendConfig();
        m_viewer->addColorLegendToBottomLeftCorner(virtLegend->legend());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateMinMaxValuesAndAddLegendToView(QString legendLabel, 
                                                          RimEclipseCellColors* resultColors, 
                                                          RigCaseCellResultsData* cellResultsData)
{
    resultColors->updateLegendData(m_currentTimeStep);

    if (resultColors->hasResult() && resultColors->legendConfig()->enableLegend())
    {
        m_viewer->addColorLegendToBottomLeftCorner(resultColors->legendConfig()->legend());
        resultColors->legendConfig()->setTitle(legendLabel + resultColors->resultVariableUiShortName());
    }

    size_t maxTimeStepCount = cellResultsData->maxTimeStepCount();
    if (resultColors->isTernarySaturationSelected() && maxTimeStepCount > 1)
    {
        if (resultColors->ternaryLegendConfig->enableLegend() && resultColors->ternaryLegendConfig->legend())
        {
            resultColors->ternaryLegendConfig->setTitle(legendLabel);
            m_viewer->addColorLegendToBottomLeftCorner(resultColors->ternaryLegendConfig->legend());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::setEclipseCase(RimEclipseCase* reservoir)
{
    m_eclipseCase = reservoir;
    cellResult()->setEclipseCase(reservoir);
    faultResultSettings()->customFaultResult()->setEclipseCase(reservoir);
    
    cellEdgeResult()->setEclipseCase(reservoir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseView::eclipseCase() const
{
    return m_eclipseCase;
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
    if (!(m_eclipseCase && m_eclipseCase->eclipseCaseData()) ) return;

    cvf::Collection<RigSimWellData> simWellData = m_eclipseCase->eclipseCaseData()->wellResults();

 
    std::vector<caf::PdmPointer<RimSimWellInView> > newWells;

    // Clear the possible well results data present
    for (size_t wIdx = 0; wIdx < this->wellCollection()->wells().size(); ++wIdx)
    {
        RimSimWellInView* well = this->wellCollection()->wells()[wIdx];
        well->setSimWellData(nullptr, -1);
    }

    bool isAnyWellCreated = false;

    // Find corresponding well from well result, or create a new

    for (size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx)
    {
        RimSimWellInView* well = this->wellCollection()->findWell(simWellData[wIdx]->m_wellName);

        if (!well)
        {
            well = new RimSimWellInView;
            well->name = simWellData[wIdx]->m_wellName;

            isAnyWellCreated = true;
        }
        newWells.push_back(well);

        well->setSimWellData(simWellData[wIdx].p(), wIdx);
    }

    // Delete all wells that does not have a result

    for (size_t wIdx = 0; wIdx < this->wellCollection()->wells().size(); ++wIdx)
    {
        RimSimWellInView* well = this->wellCollection()->wells()[wIdx];
        RigSimWellData* simWellData = well->simWellData();
        if (simWellData == nullptr)
        {
            delete well;
        }
    }
    this->wellCollection()->wells().clear();

    // Set the new wells into the field.
    this->wellCollection()->wells().insert(0, newWells);

    // Make sure all the wells have their reservoirView ptr setup correctly
    this->wellCollection()->setReservoirView(this);

    // Sort wells before assigning colors, as the colors are distributed based on sorting
    this->wellCollection()->sortWellsByName();

    if (isAnyWellCreated)
    {
        this->wellCollection()->assignDefaultWellColors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RivReservoirViewPartMgr* RimEclipseView::reservoirGridPartManager() const
{
    return m_reservoirGridPartManager.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirViewPartMgr * RimEclipseView::reservoirGridPartManager()
{
    return m_reservoirGridPartManager.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid)
{
    CVF_ASSERT(visibleCells != nullptr);

    // Initialize the return array
    if (visibleCells->size() != grid->cellCount())
    {
        visibleCells->resize(grid->cellCount());
    }
    visibleCells->setAll(false);

    RigActiveCellInfo* activeCellInfo = this->currentActiveCellInfo();

    CVF_ASSERT(activeCellInfo);

    // Loop over the wells and find their contribution
    for (size_t wIdx = 0; wIdx < this->wellCollection()->wells().size(); ++wIdx)
    {
        RimSimWellInView* well =  this->wellCollection()->wells()[wIdx];
        if (well->isWellCellsVisible())
        {
            RigSimWellData* simWellData = well->simWellData();
            if (!simWellData) continue;

            const std::vector< RigWellResultFrame >& wellResFrames = simWellData->m_wellCellsTimeSteps;
            for (size_t wfIdx = 0; wfIdx < wellResFrames.size(); ++wfIdx)
            {
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
                            if (well->showWellCellFence())
                            {
                                size_t i, j, k;
                                grid->ijkFromCellIndex(gridCellIndex, &i, &j, &k);

                                size_t* pI = &i;
                                size_t *pJ = &j;
                                size_t *pK = &k;
                                size_t cellCountFenceDirection = 0;
                                size_t fIdx = 0;

                                if (this->wellCollection()->wellCellFenceType == RimSimWellInViewCollection::K_DIRECTION)
                                {
                                    cellCountFenceDirection = grid->cellCountK();
                                    pK = &fIdx;
                                }
                                else if (this->wellCollection()->wellCellFenceType == RimSimWellInViewCollection::J_DIRECTION)
                                {
                                    cellCountFenceDirection = grid->cellCountJ();
                                    pJ = &fIdx;
                                }
                                else if (this->wellCollection()->wellCellFenceType == RimSimWellInViewCollection::I_DIRECTION)
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
    m_simWellsPartManager->clearGeometryCache();

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
void RimEclipseView::calculateCompletionTypeAndRedrawIfRequired()
{
    bool isDependingOnCompletionType = false;

    if (cellResult()->isCompletionTypeSelected())
    {
        isDependingOnCompletionType = true;
    }

    if (cellEdgeResult()->hasResult())
    {
        std::vector<RimCellEdgeMetaData> metaData;
        cellEdgeResult()->cellEdgeMetaData(&metaData);
        for (const auto& cellEdgeMeta : metaData)
        {
            if (cellEdgeMeta.m_resultVariable == RiaDefines::completionTypeResultName())
            {
                isDependingOnCompletionType = true;
            }
        }
    }

    if (currentFaultResultColors() && currentFaultResultColors()->isCompletionTypeSelected())
    {
        isDependingOnCompletionType = true;
    }

    for (const auto& propFilter : m_propertyFilterCollection()->propertyFilters)
    {
        if (propFilter->isActive() && propFilter->resultDefinition->resultVariable() == RiaDefines::completionTypeResultName())
        {
            isDependingOnCompletionType = true;
        }
    }

    if (isDependingOnCompletionType)
    {
        this->loadDataAndUpdate();
    }

    for (const auto& propFilter : m_propertyFilterCollection()->propertyFilters)
    {
        if (propFilter->isActive() && propFilter->resultDefinition->resultVariable() == RiaDefines::completionTypeResultName())
        {
            propFilter->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::isVirtualConnectionFactorGeometryVisible() const
{
    if (!m_showWindow()) return false;

    if (!m_virtualPerforationResult->isActive()) return false;

    // TODO: Consider check if no well paths are visible

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RivCellSetEnum>& RimEclipseView::visibleGridParts() const
{
    return m_visibleGridParts;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    Rim3dView::defineUiOrdering(uiConfigName, uiOrdering);

    caf::PdmUiGroup* cellGroup = uiOrdering.addNewGroup("Cell Visibility");
    cellGroup->add(&m_showMainGrid);
    cellGroup->add(&m_showInactiveCells);
    cellGroup->add(&m_showInvalidCells);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(m_overlayInfoConfig());
    uiTreeOrdering.add(m_gridCollection());

    uiTreeOrdering.add(cellResult());
    uiTreeOrdering.add(cellEdgeResult());
    uiTreeOrdering.add(faultResultSettings());
    uiTreeOrdering.add(wellCollection());

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);
    RimOilField* oilfield = project->activeOilField();
    
    if (oilfield && oilfield->fractureDefinitionCollection().notNull())
    {
        if (!oilfield->fractureDefinitionCollection()->fractureTemplates().empty())
        {
            uiTreeOrdering.add(fractureColors());
        }
    }

    uiTreeOrdering.add(m_virtualPerforationResult);

    uiTreeOrdering.add(faultCollection());
    uiTreeOrdering.add(crossSectionCollection());
    
    uiTreeOrdering.add(m_rangeFilterCollection());
    uiTreeOrdering.add(m_propertyFilterCollection());

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RivCellSetEnum> RimEclipseView::allVisibleFaultGeometryTypes() const
{
    std::set<RivCellSetEnum> faultGeoTypes;
    faultGeoTypes.insert(m_visibleGridParts.begin(), m_visibleGridParts.end());

    if (faultCollection()->isShowingFaultsAndFaultsOutsideFilters())
    {
        faultGeoTypes.insert(ACTIVE);
        faultGeoTypes.insert(ALL_WELL_CELLS);

        if (showInactiveCells())
        {
            faultGeoTypes.insert(INACTIVE);
        }
    }

    return faultGeoTypes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateFaultColors()
{
    std::set<RivCellSetEnum> faultGeometriesToRecolor = allVisibleFaultGeometryTypes();

    RimEclipseCellColors* faultResultColors = currentFaultResultColors();

    for (RivCellSetEnum cellSetType : faultGeometriesToRecolor)
    {
        if (this->hasUserRequestedAnimation() && this->cellEdgeResult()->hasResult())
        {
            m_reservoirGridPartManager->updateFaultCellEdgeResultColor(cellSetType, m_currentTimeStep, faultResultColors, this->cellEdgeResult());
        }
        else
        {
            m_reservoirGridPartManager->updateFaultColors(cellSetType, m_currentTimeStep, faultResultColors);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::isTimeStepDependentDataVisible() const
{
    if (this->cellResult()->hasDynamicResult()) return true;

    if (this->eclipsePropertyFilterCollection()->hasActiveDynamicFilters()) return true;
        
    if (this->wellCollection()->hasVisibleWellPipes()) return true;

    if (this->cellResult()->isTernarySaturationSelected()) return true;
    
    if (this->faultResultSettings()->showCustomFaultResult())
    {
        if (this->faultResultSettings()->customFaultResult()->hasDynamicResult()) return true;

        if (this->faultResultSettings()->customFaultResult()->isTernarySaturationSelected()) return true;
    }

    if (this->wellPathCollection()->anyWellsContainingPerforationIntervals()) return true;

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors* RimEclipseView::currentFaultResultColors()
{
    RimEclipseCellColors* faultResultColors = this->cellResult();

    if (this->faultResultSettings()->showCustomFaultResult())
    {
        faultResultColors = this->faultResultSettings()->customFaultResult();
    }

    return faultResultColors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::resetLegendsInViewer()
{
    RimLegendConfig* cellResultNormalLegendConfig = this->cellResult()->legendConfig();
    if (cellResultNormalLegendConfig) cellResultNormalLegendConfig->recreateLegend();

    this->cellResult()->ternaryLegendConfig->recreateLegend();
    this->cellEdgeResult()->legendConfig()->recreateLegend();

    m_viewer->removeAllColorLegends();
    
    if (cellResultNormalLegendConfig)
    {
        m_viewer->addColorLegendToBottomLeftCorner(cellResultNormalLegendConfig->legend());
    }

    m_viewer->addColorLegendToBottomLeftCorner(this->cellEdgeResult()->legendConfig()->legend());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateVirtualConnectionLegendRanges()
{
    if (!eclipseCase()) return;

    const RigVirtualPerforationTransmissibilities* virtualTransmissibilities = eclipseCase()->computeAndGetVirtualPerforationTransmissibilities();
    if (virtualTransmissibilities)
    {
        double minValue = HUGE_VAL;
        double maxValue = -HUGE_VAL;
        double posClosestToZero = HUGE_VAL;
        double negClosestToZero = -HUGE_VAL;

        virtualTransmissibilities->computeMinMax(&minValue, &maxValue, &posClosestToZero, &negClosestToZero);

        if (minValue != HUGE_VAL)
        {
            RimLegendConfig* legendConfig = virtualPerforationResult()->legendConfig();

            legendConfig->setAutomaticRanges(minValue, maxValue, minValue, maxValue);
            legendConfig->setClosestToZeroValues(posClosestToZero, negClosestToZero, posClosestToZero, negClosestToZero);
        }
    }
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
RimCase* RimEclipseView::ownerCase() const
{
    return eclipseCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimEclipseView::mainGrid() const
{
    if (eclipseCase() && eclipseCase()->eclipseCaseData())
    {
        return eclipseCase()->eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection* RimEclipseView::eclipsePropertyFilterCollection()
{
    if (m_overridePropertyFilterCollection)
    {
        return m_overridePropertyFilterCollection;
    }
    else
    {
        return m_propertyFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimEclipsePropertyFilterCollection* RimEclipseView::eclipsePropertyFilterCollection() const
{
    if (m_overridePropertyFilterCollection)
    {
        return m_overridePropertyFilterCollection;
    }
    else
    {
        return m_propertyFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::setOverridePropertyFilterCollection(RimEclipsePropertyFilterCollection* pfc)
{
    m_overridePropertyFilterCollection = pfc;

    this->scheduleGeometryRegen(PROPERTY_FILTERED);
    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStep)
{
    size_t gridCount = this->eclipseCase()->eclipseCaseData()->gridCount();
    size_t cellCount = this->mainGrid()->globalCellArray().size();

    totalVisibility->resize(cellCount);
    totalVisibility->setAll(false);

    for (size_t gridIdx = 0; gridIdx < gridCount; ++gridIdx)
    {
        RigGridBase * grid = this->eclipseCase()->eclipseCaseData()->grid(gridIdx);
        int gridCellCount = static_cast<int>(grid->cellCount());

        for (size_t gpIdx = 0; gpIdx < m_visibleGridParts.size(); ++gpIdx)
        {
            const cvf::UByteArray* visibility =  m_reservoirGridPartManager->cellVisibility(m_visibleGridParts[gpIdx], gridIdx, timeStep);

            for (int lcIdx = 0; lcIdx < gridCellCount; ++ lcIdx)
            {
                (*totalVisibility)[grid->reservoirCellIndex(lcIdx)] |= (*visibility)[lcIdx];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::showActiveCellsOnly()
{
    return !m_showInactiveCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts)
{
    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    std::vector<RiuSelectionItem*> items;
    riuSelManager->selectedItems(items);

    for (size_t i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT)
        {
            RiuEclipseSelectionItem* eclipseSelItem = static_cast<RiuEclipseSelectionItem*>(items[i]);
            if (eclipseSelItem && eclipseSelItem->m_view == this)
            {
                CVF_ASSERT(eclipseSelItem->m_view->eclipseCase());
                CVF_ASSERT(eclipseSelItem->m_view->eclipseCase()->eclipseCaseData());

                RivSingleCellPartGenerator partGen(eclipseSelItem->m_view->eclipseCase()->eclipseCaseData(), eclipseSelItem->m_gridIndex, eclipseSelItem->m_gridLocalCellIndex);

                cvf::ref<cvf::Part> part = partGen.createPart(eclipseSelItem->m_color);
                part->setTransform(this->scaleTransform());

                parts->push_back(part.p());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::updateIconStateForFilterCollections()
{
    m_rangeFilterCollection()->updateIconState();
    m_rangeFilterCollection()->uiCapability()->updateConnectedEditors();

    // NB - notice that it is the filter collection managed by this view that the icon update applies to
    m_propertyFilterCollection()->updateIconState();
    m_propertyFilterCollection()->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseView::axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel)
{
    CVF_ASSERT(xLabel && yLabel && zLabel);

    *xLabel = "E(x)";
    *yLabel = "N(y)";
    *zLabel = "Z";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::isUsingFormationNames() const
{
    if ((cellResult()->resultType() == RiaDefines::FORMATION_NAMES)) return true;
    
    return eclipsePropertyFilterCollection()->isUsingFormationNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::showInvalidCells() const
{
    return m_showInvalidCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::showInactiveCells() const
{
    return m_showInactiveCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseView::showMainGrid() const
{
    return m_showMainGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimPropertyFilterCollection* RimEclipseView::propertyFilterCollection() const
{
    return eclipsePropertyFilterCollection();
}
