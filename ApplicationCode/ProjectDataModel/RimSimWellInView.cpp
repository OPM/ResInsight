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

#include "RimSimWellInView.h"

#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimWellData.h"

#include "RimCellRangeFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimIntersectionCollection.h"

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
#include "RimSimWellFractureCollection.h"
#include "RimSimWellFracture.h"
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

#include "RiuMainWindow.h"

#include "RivReservoirViewPartMgr.h"

#include "cafPdmUiTreeOrdering.h"

#include "cvfMath.h"
#include "RigCell.h"
#include "RimEclipseCase.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigActiveCellInfo.h"

CAF_PDM_SOURCE_INIT(RimSimWellInView, "Well");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInView::RimSimWellInView()
{
    CAF_PDM_InitObject("Well", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&name,           "WellName", "Name", "", "", "");

    CAF_PDM_InitField(&showWell,                "ShowWell",             true,   "Show well ", "", "", "");

    CAF_PDM_InitField(&showWellLabel,           "ShowWellLabel",        true,   "Label", "", "", "");
    CAF_PDM_InitField(&showWellHead,            "ShowWellHead",         true,   "Well Head", "", "", "");
    CAF_PDM_InitField(&showWellPipe,            "ShowWellPipe",         true,   "Pipe", "", "", "");
    CAF_PDM_InitField(&showWellSpheres,         "ShowWellSpheres",      false,  "Spheres", "", "", "");

    CAF_PDM_InitField(&wellHeadScaleFactor,     "WellHeadScaleFactor",  1.0,    "Well Head Scale", "", "", "");
    CAF_PDM_InitField(&pipeScaleFactor,         "WellPipeRadiusScale",  1.0,    "Pipe Radius Scale", "", "", "");
    CAF_PDM_InitField(&wellPipeColor,           "WellPipeColor",        cvf::Color3f(0.588f, 0.588f, 0.804f), "Pipe Color", "", "", "");

    CAF_PDM_InitField(&showWellCells,           "ShowWellCells",        false,  "Well Cells", "", "", "");
    CAF_PDM_InitField(&showWellCellFence,       "ShowWellCellFence",    false,  "Well Cell Fence", "", "", "");

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    CAF_PDM_InitFieldNoDefault(&simwellFractureCollection, "FractureCollection", "Fractures", "", "", "");
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    name.uiCapability()->setUiHidden(true);
    name.uiCapability()->setUiReadOnly(true);

    m_resultWellIndex = cvf::UNDEFINED_SIZE_T;

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    simwellFractureCollection= new RimSimWellFractureCollection();
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInView::~RimSimWellInView()
{
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    if (simwellFractureCollection()) delete simwellFractureCollection();
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSimWellInView::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);
    if (reservoirView)
    {
        if (&showWellLabel == changedField ||
            &showWellHead == changedField ||
            &showWellPipe == changedField ||
            &showWellSpheres == changedField ||
            &wellPipeColor == changedField)
        {
            reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (&showWell == changedField ||
                 &showWellCells == changedField ||
                 &showWellCellFence == changedField)
             
        {
            reservoirView->scheduleGeometryRegen(VISIBLE_WELL_CELLS);
            reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (   &pipeScaleFactor == changedField
                 || &wellHeadScaleFactor == changedField)
        {
            if (reservoirView)
            {
                reservoirView->scheduleSimWellGeometryRegen();
                reservoirView->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }

    RimEclipseWellCollection* wellColl = nullptr;
    this->firstAncestorOrThisOfType(wellColl);
    if (wellColl)
    {
        wellColl->updateStateForVisibilityCheckboxes();

        RiuMainWindow::instance()->refreshDrawStyleActions();
    }

    if (changedField == &wellPipeColor)
    {
        RimEclipseWellCollection::updateWellAllocationPlots();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSimWellInView::objectToggleField()
{
    return &showWell;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::calculateWellPipeStaticCenterLine(std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                       std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds)
{
    RigSimulationWellCenterLineCalculator::calculateWellPipeStaticCenterline(this, pipeBranchesCLCoords, pipeBranchesCellIds);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::calculateWellPipeDynamicCenterLine(size_t timeStepIdx, 
                                                 std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                 std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds) const
{
    RigSimulationWellCenterLineCalculator::calculateWellPipeDynamicCenterline(this, timeStepIdx, pipeBranchesCLCoords, pipeBranchesCellIds);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::wellHeadTopBottomPosition(size_t frameIndex, cvf::Vec3d* top, cvf::Vec3d* bottom)
{

    RimEclipseView* m_rimReservoirView;
    firstAncestorOrThisOfTypeAsserted(m_rimReservoirView);
    
    RigEclipseCaseData* rigReservoir = m_rimReservoirView->eclipseCase()->eclipseCaseData();

    if (!this->simWellData()->hasWellResult(frameIndex)) return;

    const RigWellResultFrame& wellResultFrame = this->simWellData()->wellResultFrame(frameIndex);
    const RigCell& whCell = rigReservoir->cellFromWellResultCell(wellResultFrame.m_wellHead);

    // Match this position with pipe start position in RivWellPipesPartMgr::calculateWellPipeCenterline()

    (*bottom) = whCell.faceCenter(cvf::StructGridInterface::NEG_K);

    // Compute well head based on the z position of the top of the K column the well head is part of
    (*top) = (*bottom);
    if ( m_rimReservoirView->wellCollection()->wellHeadPosition() == RimEclipseWellCollection::WELLHEAD_POS_TOP_COLUMN )
    {
        // Position well head at top active cell of IJ-column

        size_t i, j, k;
        rigReservoir->mainGrid()->ijkFromCellIndex(whCell.mainGridCellIndex(), &i, &j, &k);

        size_t kIndexWellHeadCell = k;
        k = 0;

        size_t topActiveCellIndex = rigReservoir->mainGrid()->cellIndexFromIJK(i, j, k);
        while ( k < kIndexWellHeadCell && !m_rimReservoirView->currentActiveCellInfo()->isActive(topActiveCellIndex) )
        {
            k++;
            topActiveCellIndex = rigReservoir->mainGrid()->cellIndexFromIJK(i, j, k);
        }

        const RigCell& topActiveCell = rigReservoir->mainGrid()->cell(topActiveCellIndex);
        cvf::Vec3d topCellPos = topActiveCell.faceCenter(cvf::StructGridInterface::NEG_K);

        // Modify position if top active cell is closer to sea than well head
        if ( kIndexWellHeadCell > k )
        {
            top->z() = topCellPos.z();
        }
    }
    else
    {
        // Position well head at top of active cells bounding box

        cvf::Vec3d activeCellsBoundingBoxMax = m_rimReservoirView->currentActiveCellInfo()->geometryBoundingBox().max();

        top->z() = activeCellsBoundingBoxMax.z();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSimWellInView::pipeRadius()
{
    RimEclipseView* reservoirView;
    firstAncestorOrThisOfTypeAsserted(reservoirView);

    RigEclipseCaseData* rigReservoir = reservoirView->eclipseCase()->eclipseCaseData();
    
    double characteristicCellSize = rigReservoir->mainGrid()->characteristicIJCellSize();

    double pipeRadius = reservoirView->wellCollection()->pipeScaleFactor() * this->pipeScaleFactor() * characteristicCellSize;

    return pipeRadius;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::intersectsDynamicWellCellsFilteredCells(size_t frameIndex) const
{
    if (this->simWellData() == nullptr) return false;

    if (!simWellData()->hasWellResult(frameIndex)) return false;

    const RigWellResultFrame& wrsf = this->simWellData()->wellResultFrame(frameIndex);

    return intersectsWellCellsFilteredCells(wrsf, frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::intersectsWellCellsFilteredCells(const RigWellResultFrame &wrsf, size_t frameIndex) const
{
    RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);
    if (!reservoirView) return false;

    const std::vector<RivCellSetEnum>& visGridParts = reservoirView->visibleGridParts();
    RivReservoirViewPartMgr* rvMan = reservoirView->reservoirGridPartManager();


    for (const RivCellSetEnum& visGridPart : visGridParts)
    {
        if (visGridPart == ALL_WELL_CELLS
            || visGridPart == VISIBLE_WELL_CELLS
            || visGridPart == VISIBLE_WELL_FENCE_CELLS
            || visGridPart == VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER
            || visGridPart == VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER
            )
        {
            // Exclude all cells related to well cells
            continue;
        }

        // First check the wellhead:

        size_t gridIndex = wrsf.m_wellHead.m_gridIndex;
        size_t gridCellIndex = wrsf.m_wellHead.m_gridCellIndex;

        if (gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T)
        {
            const cvf::UByteArray* cellVisibility = rvMan->cellVisibility(visGridPart, gridIndex, frameIndex);
            if (gridCellIndex < cellVisibility->size() && (*cellVisibility)[gridCellIndex])
            {
                return true;
            }
        }

        // Then check the rest of the well, with all the branches

        const std::vector<RigWellResultBranch>& wellResSegments = wrsf.m_wellResultBranches;
        for (const RigWellResultBranch& branchSegment : wellResSegments)
        {
            const std::vector<RigWellResultPoint>& wsResCells = branchSegment.m_branchResultPoints;
            for (const RigWellResultPoint& wellResultPoint : wsResCells)
            {
                if (wellResultPoint.isCell())
                {
                    gridIndex = wellResultPoint.m_gridIndex;
                    gridCellIndex = wellResultPoint.m_gridCellIndex;

                    const cvf::UByteArray* cellVisibility = rvMan->cellVisibility(visGridPart, gridIndex, frameIndex);
                    if (gridCellIndex < cellVisibility->size() && (*cellVisibility)[gridCellIndex])
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::intersectsStaticWellCellsFilteredCells() const
{
    if (this->simWellData() == nullptr) return false;

    // NOTE: Read out static well cells, union of well cells across all time steps
    const RigWellResultFrame& wrsf = this->simWellData()->staticWellCells();

    // NOTE: Use first time step for visibility evaluation
    size_t frameIndex = 0;

    return intersectsWellCellsFilteredCells(wrsf, frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Visibility");
    appearanceGroup->add(&showWellLabel);
    appearanceGroup->add(&showWellHead);
    appearanceGroup->add(&showWellPipe);
    appearanceGroup->add(&showWellSpheres);
    
    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Well Cells and Fence");
    filterGroup->add(&showWellCells);
    filterGroup->add(&showWellCellFence);

    showWellCellFence.uiCapability()->setUiReadOnly(!showWellCells());
    caf::PdmUiGroup* sizeScalingGroup = uiOrdering.addNewGroup("Size Scaling");
    sizeScalingGroup->add(&wellHeadScaleFactor);
    sizeScalingGroup->add(&pipeScaleFactor);

    caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup("Colors");
    colorGroup->add(&wellPipeColor);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    for (RimSimWellFracture* fracture : simwellFractureCollection()->simwellFractures())
    {
        uiTreeOrdering.add(fracture);
    }
    uiTreeOrdering.skipRemainingChildren(true);
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    const RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);
    if (!reservoirView) return;

    if (reservoirView->rangeFilterCollection() && !reservoirView->rangeFilterCollection()->hasActiveFilters())
    {
        this->uiCapability()->setUiReadOnly(false);

        return;
    }
    
    const RimEclipseWellCollection* wellColl = nullptr;
    this->firstAncestorOrThisOfType(wellColl);
    if (!wellColl) return;

    if (wellColl->showWellsIntersectingVisibleCells() && !this->intersectsDynamicWellCellsFilteredCells(static_cast<size_t>(reservoirView->currentTimeStep())))
    {
        // Mark well as read only if well is not intersecting visible cells

        this->uiCapability()->setUiReadOnly(true);
    }
    else
    {
        this->uiCapability()->setUiReadOnly(false);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellCellsVisible() const
{
    const RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);

    if (reservoirView == nullptr) return false;
    if (this->simWellData() == nullptr) return false;

    if (!reservoirView->wellCollection()->isActive())
        return false;

    if (!this->showWell())
        return false;

    if (!this->showWellCells())
        return false;

    if (reservoirView->crossSectionCollection()->hasActiveIntersectionForSimulationWell(this))
        return true;

    if (reservoirView->wellCollection()->showWellsIntersectingVisibleCells()
        && reservoirView->rangeFilterCollection()->hasActiveFilters())
    {
        return intersectsStaticWellCellsFilteredCells();
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellPipeVisible(size_t frameIndex) const
{
    const RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);

    if (reservoirView == nullptr) return false;
    if (this->simWellData() == nullptr) return false;

    if (frameIndex >= this->simWellData()->m_resultTimeStepIndexToWellTimeStepIndex.size())
    {
        return false;
    }

    size_t wellTimeStepIndex = this->simWellData()->m_resultTimeStepIndexToWellTimeStepIndex[frameIndex];
    if (wellTimeStepIndex == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    if (!reservoirView->wellCollection()->isActive())
        return false;

    if (!this->showWell())
        return false;

    if (!this->showWellPipe())
        return false;


    if (reservoirView->crossSectionCollection()->hasActiveIntersectionForSimulationWell(this))
        return true;

    if (reservoirView->wellCollection()->showWellsIntersectingVisibleCells()
        && reservoirView->rangeFilterCollection()->hasActiveFilters())
    {
        return intersectsDynamicWellCellsFilteredCells(frameIndex);
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellSpheresVisible(size_t frameIndex) const
{
    const RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);

    if (reservoirView == nullptr) return false;
    if (this->simWellData() == nullptr) return false;

    if (frameIndex >= this->simWellData()->m_resultTimeStepIndexToWellTimeStepIndex.size())
    {
        return false;
    }

    size_t wellTimeStepIndex = this->simWellData()->m_resultTimeStepIndexToWellTimeStepIndex[frameIndex];
    if (wellTimeStepIndex == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    if (!reservoirView->wellCollection()->isActive())
        return false;

    if (!this->showWell())
        return false;

    if (!this->showWellSpheres())
        return false;

    if (reservoirView->crossSectionCollection()->hasActiveIntersectionForSimulationWell(this))
        return true;

    if (reservoirView->wellCollection()->showWellsIntersectingVisibleCells()
        && reservoirView->rangeFilterCollection()->hasActiveFilters())
    {
        return intersectsDynamicWellCellsFilteredCells(frameIndex);
    }
    else
    {
        return true;
    }

    CVF_ASSERT(false); // Never end here. have you added new pipe visibility modes ?

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isUsingCellCenterForPipe() const
{
    const RimEclipseWellCollection* wellColl = nullptr;
    this->firstAncestorOrThisOfType(wellColl);

    return (wellColl && wellColl->wellPipeCoordType() == RimEclipseWellCollection::WELLPIPE_CELLCENTER);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::setSimWellData(RigSimWellData* simWellData, size_t resultWellIndex)
{
    m_simWellData = simWellData;
    m_resultWellIndex = resultWellIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSimWellData* RimSimWellInView::simWellData()
{
    return m_simWellData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigSimWellData* RimSimWellInView::simWellData() const
{
    return m_simWellData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimSimWellInView::resultWellIndex() const
{
    return m_resultWellIndex;
}

