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

#include "RigSingleWellResultsData.h"

#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimIntersectionCollection.h"

#include "cvfMath.h"
#include "RigSimulationWellCenterLineCalculator.h"

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
    CAF_PDM_InitField(&showWellPipes,           "ShowWellPipe",         true,   "Show well pipe", "", "", "");
    CAF_PDM_InitField(&showWellSpheres,         "ShowWellSpheres",      true,   "Show well spheres", "", "", "");

    CAF_PDM_InitField(&pipeScaleFactor,         "WellPipeRadiusScale",  1.0,    "Well Pipe Scale Factor", "", "", "");
    CAF_PDM_InitField(&wellPipeColor,           "WellPipeColor",        cvf::Color3f(0.588f, 0.588f, 0.804f), "Well pipe color", "", "", "");

    CAF_PDM_InitField(&showWellCells,           "ShowWellCells",        true,   "Add cells to range filter", "", "", "");
    CAF_PDM_InitField(&showWellCellFence,       "ShowWellCellFence",    false,  "Use well fence", "", "", "");

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
        &showWellPipes == changedField ||
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
bool RimEclipseWell::visibleCellsInstersectsWell(size_t frameIndex)
{
    if (this->wellResults() == nullptr) return false;

    if (!wellResults()->hasWellResult(frameIndex)) return false;

    RimEclipseView* m_reservoirView = nullptr;
    this->firstAncestorOrThisOfType(m_reservoirView);

    const std::vector<RivCellSetEnum>& visGridParts = m_reservoirView->visibleGridParts();
    cvf::cref<RivReservoirViewPartMgr> rvMan = m_reservoirView->reservoirGridPartManager();

    for (size_t gpIdx = 0; gpIdx < visGridParts.size(); ++gpIdx)
    {
        const RigWellResultFrame& wrsf = this->wellResults()->wellResultFrame(frameIndex);

        // First check the wellhead:

        size_t gridIndex = wrsf.m_wellHead.m_gridIndex;
        size_t gridCellIndex = wrsf.m_wellHead.m_gridCellIndex;

        if (gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T)
        {
            cvf::cref<cvf::UByteArray> cellVisibility = rvMan->cellVisibility(visGridParts[gpIdx], gridIndex, frameIndex);
            if ((*cellVisibility)[gridCellIndex])
            {
                return true;
            }
        }

        // Then check the rest of the well, with all the branches

        const std::vector<RigWellResultBranch>& wellResSegments = wrsf.m_wellResultBranches;
        for (size_t wsIdx = 0; wsIdx < wellResSegments.size(); ++wsIdx)
        {
            const std::vector<RigWellResultPoint>& wsResCells = wellResSegments[wsIdx].m_branchResultPoints;
            for (size_t cIdx = 0; cIdx < wsResCells.size(); ++cIdx)
            {
                if (wsResCells[cIdx].isCell())
                {
                    gridIndex = wsResCells[cIdx].m_gridIndex;
                    gridCellIndex = wsResCells[cIdx].m_gridCellIndex;

                    cvf::cref<cvf::UByteArray> cellVisibility = rvMan->cellVisibility(visGridParts[gpIdx], gridIndex, frameIndex);
                    if ((*cellVisibility)[gridCellIndex])
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
    pipeGroup->add(&showWellPipes);
    pipeGroup->add(&showWellSpheres);
    
    pipeGroup->add(&pipeScaleFactor);
    
    pipeGroup->add(&wellPipeColor);

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Range filter");
    filterGroup->add(&showWellCells);
    filterGroup->add(&showWellCellFence);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWell::isWellPipeVisible(size_t frameIndex)
{
    RimEclipseView* m_reservoirView = nullptr;
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

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimEclipseWellCollection::PIPES_FORCE_ALL_ON)
        return true;

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimEclipseWellCollection::PIPES_FORCE_ALL_OFF)
        return false;

    if (this->showWell() == false)
        return false;

    if (this->showWellPipes() == false)
        return false;

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimEclipseWellCollection::PIPES_INDIVIDUALLY)
        return true;

    if (m_reservoirView->crossSectionCollection()->hasActiveIntersectionForSimulationWell(this))
        return true;

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimEclipseWellCollection::PIPES_OPEN_IN_VISIBLE_CELLS)
    {
        return visibleCellsInstersectsWell(frameIndex);
    }

    CVF_ASSERT(false); // Never end here. have you added new pipe visibility modes ?

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWell::isWellSpheresVisible(size_t frameIndex)
{
    RimEclipseView* m_reservoirView = nullptr;
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

    if (m_reservoirView->wellCollection()->wellSphereVisibility() == RimEclipseWellCollection::PIPES_FORCE_ALL_ON)
        return true;

    if (m_reservoirView->wellCollection()->wellSphereVisibility() == RimEclipseWellCollection::PIPES_FORCE_ALL_OFF)
        return false;

    if (this->showWell() == false)
        return false;

    if (this->showWellSpheres() == false)
        return false;

    if (m_reservoirView->wellCollection()->wellSphereVisibility() == RimEclipseWellCollection::PIPES_INDIVIDUALLY)
        return true;

    if (m_reservoirView->crossSectionCollection()->hasActiveIntersectionForSimulationWell(this))
        return true;

    if (m_reservoirView->wellCollection()->wellSphereVisibility() == RimEclipseWellCollection::PIPES_OPEN_IN_VISIBLE_CELLS)
    {
        return visibleCellsInstersectsWell(frameIndex);
    }

    CVF_ASSERT(false); // Never end here. have you added new pipe visibility modes ?

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWell::isUsingCellCenterForPipe()
{
    RimEclipseWellCollection* wellColl = nullptr;
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
size_t RimEclipseWell::resultWellIndex()
{
    return m_resultWellIndex;
}

