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

#include "RimEclipseWell.h"

#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSingleWellResultsData.h"

#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimIntersectionCollection.h"

#include "cvfMath.h"

CAF_PDM_SOURCE_INIT(RimEclipseWell, "Well");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell::RimEclipseWell()
{
    CAF_PDM_InitObject("Well", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&name,           "WellName", "Name", "", "", "");

    CAF_PDM_InitField(&showWell,                "ShowWell",             true,   "Show well ", "", "", "");
    showWell.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&showWellLabel,           "ShowWellLabel",        true,   "Show well label", "", "", "");
    CAF_PDM_InitField(&showWellHead,            "ShowWellHead",         true,   "Show well head", "", "", "");
    CAF_PDM_InitField(&showWellPipe,            "ShowWellPipe",         true,   "Show well pipe", "", "", "");
    CAF_PDM_InitField(&showWellSpheres,         "ShowWellSpheres",      false,  "Show well spheres", "", "", "");

    CAF_PDM_InitField(&pipeScaleFactor,         "WellPipeRadiusScale",  1.0,    "Well Pipe Scale Factor", "", "", "");
    CAF_PDM_InitField(&wellPipeColor,           "WellPipeColor",        cvf::Color3f(0.588f, 0.588f, 0.804f), "Well pipe color", "", "", "");

    CAF_PDM_InitField(&showWellCells,           "ShowWellCells",        true,   "Show Well Cells", "", "", "");
    CAF_PDM_InitField(&showWellCellFence,       "ShowWellCellFence",    false,  "Show Well Cell Fence", "", "", "");

    name.uiCapability()->setUiHidden(true);
    name.uiCapability()->setUiReadOnly(true);

    m_resultWellIndex = cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell::~RimEclipseWell()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEclipseWell::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWell::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseView* m_reservoirView = nullptr;
    this->firstAncestorOrThisOfType(m_reservoirView);
    if (!m_reservoirView) return;

    if (&showWellLabel == changedField ||
        &showWellHead == changedField ||
        &showWellPipe == changedField ||
        &showWellSpheres == changedField ||
        &wellPipeColor == changedField)
    {
        m_reservoirView->scheduleCreateDisplayModelAndRedraw();
    }
    else if (&showWell == changedField ||
             &showWellCells == changedField ||
             &showWellCellFence == changedField)
             
    {
        m_reservoirView->scheduleGeometryRegen(VISIBLE_WELL_CELLS);
        m_reservoirView->scheduleCreateDisplayModelAndRedraw();
    }
    else if (&pipeScaleFactor == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->schedulePipeGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEclipseWell::objectToggleField()
{
    return &showWell;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWell::calculateWellPipeStaticCenterLine(std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                       std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds)
{
    RigSimulationWellCenterLineCalculator::calculateWellPipeStaticCenterline(this, pipeBranchesCLCoords, pipeBranchesCellIds);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWell::calculateWellPipeDynamicCenterLine(size_t timeStepIdx, 
                                                 std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                 std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds)
{
    RigSimulationWellCenterLineCalculator::calculateWellPipeDynamicCenterline(this, timeStepIdx, pipeBranchesCLCoords, pipeBranchesCellIds);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWell::intersectsVisibleCells(size_t frameIndex) const
{
    if (this->wellResults() == nullptr) return false;

    if (!wellResults()->hasWellResult(frameIndex)) return false;

    RimEclipseView* m_reservoirView = nullptr;
    this->firstAncestorOrThisOfType(m_reservoirView);

    const std::vector<RivCellSetEnum>& visGridParts = m_reservoirView->visibleGridParts();
    cvf::cref<RivReservoirViewPartMgr> rvMan = m_reservoirView->reservoirGridPartManager();

    const RigWellResultFrame& wrsf = this->wellResults()->wellResultFrame(frameIndex);

    for (const RivCellSetEnum& visGridPart : visGridParts)
    {
        // First check the wellhead:

        size_t gridIndex = wrsf.m_wellHead.m_gridIndex;
        size_t gridCellIndex = wrsf.m_wellHead.m_gridCellIndex;

        if (gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T)
        {
            cvf::cref<cvf::UByteArray> cellVisibility = rvMan->cellVisibility(visGridPart, gridIndex, frameIndex);
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

                    cvf::cref<cvf::UByteArray> cellVisibility = rvMan->cellVisibility(visGridPart, gridIndex, frameIndex);
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
void RimEclipseWell::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* pipeGroup = uiOrdering.addNewGroup("Appearance");
    pipeGroup->add(&showWellLabel);
    pipeGroup->add(&showWellHead);
    pipeGroup->add(&showWellPipe);
    pipeGroup->add(&showWellSpheres);
    
    pipeGroup->add(&pipeScaleFactor);
    
    pipeGroup->add(&wellPipeColor);

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Well Cells");
    filterGroup->add(&showWellCells);
    filterGroup->add(&showWellCellFence);

    RimEclipseWellCollection* wellColl = nullptr;
    this->firstAncestorOrThisOfType(wellColl);
    if (wellColl)
    {
        showWellLabel.uiCapability()->setUiReadOnly(!wellColl->showWellLabel());
        showWellHead.uiCapability()->setUiReadOnly(!wellColl->showWellHead());
        showWellPipe.uiCapability()->setUiReadOnly(!wellColl->showWellPipe());
        showWellSpheres.uiCapability()->setUiReadOnly(!wellColl->showWellSpheres());
        showWellCells.uiCapability()->setUiReadOnly(!wellColl->showWellCells());

        bool isFenceEnabled = false;
        if (wellColl->showWellCells() && wellColl->showWellCellFence()) isFenceEnabled = true;
        showWellCellFence.uiCapability()->setUiReadOnly(!isFenceEnabled);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWell::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    const RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);
    if (!reservoirView) return;

    const RimEclipseWellCollection* wellColl = nullptr;
    this->firstAncestorOrThisOfType(wellColl);
    if (!wellColl) return;

    if (wellColl->showWellsIntersectingVisibleCells() && !this->intersectsVisibleCells(static_cast<size_t>(reservoirView->currentTimeStep())))
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
bool RimEclipseWell::isWellPipeVisible(size_t frameIndex) const
{
    const RimEclipseView* reservoirView = nullptr;
    this->firstAncestorOrThisOfType(reservoirView);

    if (reservoirView == nullptr) return false;
    if (this->wellResults() == nullptr) return false;

    if (frameIndex >= this->wellResults()->m_resultTimeStepIndexToWellTimeStepIndex.size())
    {
        return false;
    }

    size_t wellTimeStepIndex = this->wellResults()->m_resultTimeStepIndexToWellTimeStepIndex[frameIndex];
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

    if (!reservoirView->wellCollection()->showWellPipe())
        return false;

    if (reservoirView->crossSectionCollection()->hasActiveIntersectionForSimulationWell(this))
        return true;

    if (reservoirView->wellCollection()->showWellsIntersectingVisibleCells())
    {
        return intersectsVisibleCells(frameIndex);
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWell::isWellSpheresVisible(size_t frameIndex) const
{
    const RimEclipseView* m_reservoirView = nullptr;
    this->firstAncestorOrThisOfType(m_reservoirView);

    if (m_reservoirView == nullptr) return false;
    if (this->wellResults() == nullptr) return false;

    if (frameIndex >= this->wellResults()->m_resultTimeStepIndexToWellTimeStepIndex.size())
    {
        return false;
    }

    size_t wellTimeStepIndex = this->wellResults()->m_resultTimeStepIndexToWellTimeStepIndex[frameIndex];
    if (wellTimeStepIndex == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    if (!m_reservoirView->wellCollection()->isActive())
        return false;

    if (!this->showWell())
        return false;

    if (!m_reservoirView->wellCollection()->showWellSpheres())
        return false;

    if (!this->showWellSpheres())
        return false;

    if (m_reservoirView->crossSectionCollection()->hasActiveIntersectionForSimulationWell(this))
        return true;

    if (m_reservoirView->wellCollection()->showWellsIntersectingVisibleCells())
    {
        return intersectsVisibleCells(frameIndex);
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
bool RimEclipseWell::isUsingCellCenterForPipe() const
{
    const RimEclipseWellCollection* wellColl = nullptr;
    this->firstAncestorOrThisOfType(wellColl);

    return (wellColl && wellColl->wellPipeCoordType() == RimEclipseWellCollection::WELLPIPE_CELLCENTER);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWell::setWellResults(RigSingleWellResultsData* wellResults, size_t resultWellIndex)
{
    m_wellResults = wellResults;
    m_resultWellIndex = resultWellIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSingleWellResultsData* RimEclipseWell::wellResults()
{
    return m_wellResults.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigSingleWellResultsData* RimEclipseWell::wellResults() const
{
    return m_wellResults.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimEclipseWell::resultWellIndex() const
{
    return m_resultWellIndex;
}

