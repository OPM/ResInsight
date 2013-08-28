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

//#include "RiaStdInclude.h"
#include "RimWell.h"


#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "RivReservoirViewPartMgr.h"
#include "RimReservoirView.h"
#include "RimWellCollection.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimCellRangeFilterCollection.h"

CAF_PDM_SOURCE_INIT(RimWell, "Well");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWell::RimWell()
{
    CAF_PDM_InitObject("Well", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&name,       "WellName",             "Name", "", "", "");

    CAF_PDM_InitField(&showWellLabel,         "ShowWellLabel",      true, "Show well label", "", "", "");

    CAF_PDM_InitField(&showWellPipes,       "ShowWellPipe",         true,   "Show well pipe", "", "", "");
    CAF_PDM_InitField(&pipeRadiusScaleFactor, "WellPipeRadiusScale",1.0,    "Pipe radius scale", "", "", "");
    CAF_PDM_InitField(&wellPipeColor,       "WellPipeColor",        cvf::Color3f(0.588f, 0.588f, 0.804f), "Well pipe color", "", "", "");

    CAF_PDM_InitField(&showWellCells,       "ShowWellCells",        true,   "Add cells to range filter", "", "", "");
    CAF_PDM_InitField(&showWellCellFence,   "ShowWellCellFence",    false,  "Use well fence", "", "", "");

    name.setUiHidden(true);
    name.setUiReadOnly(true);

    m_wellIndex = cvf::UNDEFINED_SIZE_T;

    m_reservoirView = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWell::~RimWell()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWell::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWell::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWell::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&showWellLabel == changedField)
    {
        if (m_reservoirView) 
        {
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    else if (&showWellCells == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->createDisplayModelAndRedraw();
        }

    }
    else if (&showWellCellFence == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->createDisplayModelAndRedraw();
        }

    }
    else if (&showWellPipes == changedField)
    {
        if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
    }
    else if (&wellPipeColor == changedField)
    {
        if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
    }
    else if (&pipeRadiusScaleFactor == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->schedulePipeGeometryRegen();
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWell::objectToggleField()
{
    return &showWellPipes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWell::calculateWellPipeVisibility(size_t frameIndex)
{
    if (m_reservoirView == NULL) return false;
    if (this->wellResults() == NULL) return false;

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

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimWellCollection::PIPES_FORCE_ALL_ON)
        return true;

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimWellCollection::PIPES_FORCE_ALL_OFF)
        return false;

    if ( this->showWellPipes() == false )
        return false;

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimWellCollection::PIPES_INDIVIDUALLY)
        return true;

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimWellCollection::PIPES_OPEN_IN_VISIBLE_CELLS)
    {
        const std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType>& visGridParts = m_reservoirView->visibleGridParts();     
        cvf::cref<RivReservoirViewPartMgr> rvMan = m_reservoirView->reservoirGridPartManager();

        for (size_t gpIdx = 0; gpIdx < visGridParts.size(); ++gpIdx)
        {
            const RigWellResultFrame& wrsf = this->wellResults()->wellResultFrame(frameIndex);

            // First check the wellhead:

            size_t gridIndex = wrsf.m_wellHead.m_gridIndex; 
            size_t gridCellIndex = wrsf.m_wellHead.m_gridCellIndex;

            cvf::cref<cvf::UByteArray> cellVisibility = rvMan->cellVisibility(visGridParts[gpIdx], gridIndex, frameIndex);
            if ((*cellVisibility)[gridCellIndex]) 
            {
                return true;
            }

            // Then check the rest of the well, with all the branches

            const std::vector<RigWellResultBranch>& wellResSegments = wrsf.m_wellResultBranches;
            for (size_t wsIdx = 0; wsIdx < wellResSegments.size(); ++wsIdx)
            {
                const std::vector<RigWellResultPoint>& wsResCells = wellResSegments[wsIdx].m_branchResultPoints;
                for (size_t cIdx = 0; cIdx < wsResCells.size(); ++ cIdx)
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

    CVF_ASSERT(false); // Never end here. have you added new pipe visibility modes ?

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWell::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* pipeGroup = uiOrdering.addNewGroup("Well pipe");
    pipeGroup->add(&showWellPipes);
    pipeGroup->add(&pipeRadiusScaleFactor);
    pipeGroup->add(&wellPipeColor);
    pipeGroup->add(&showWellLabel);

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Range filter");
    filterGroup->add(&showWellCells);
    filterGroup->add(&showWellCellFence);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWell::isWellPipeVisible(size_t frameIndex)
{
    CVF_ASSERT(m_wellIndex != cvf::UNDEFINED_SIZE_T);

    // Return the possibly cached value
    return m_reservoirView->wellCollection()->isWellPipesVisible(frameIndex)[m_wellIndex];
}

