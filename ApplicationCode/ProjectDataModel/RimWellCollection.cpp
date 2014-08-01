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

#include "RimWellCollection.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RigSingleWellResultsData.h"
#include "RimReservoirView.h"
#include "RimWell.h"
#include "RivReservoirViewPartMgr.h"


namespace caf
{
    template<>
    void RimWellCollection::WellVisibilityEnum::setUp()
    {
        addItem(RimWellCollection::PIPES_FORCE_ALL_OFF,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimWellCollection::PIPES_INDIVIDUALLY,        "ALL_ON",             "Individual");
        addItem(RimWellCollection::PIPES_OPEN_IN_VISIBLE_CELLS,"OPEN_IN_VISIBLE_CELLS", "Visible cells filtered");
        addItem(RimWellCollection::PIPES_FORCE_ALL_ON,        "FORCE_ALL_ON",       "All On");
    }
}


namespace caf
{
    template<>
    void RimWellCollection::WellCellsRangeFilterEnum::setUp()
    {
        addItem(RimWellCollection::RANGE_ADD_NONE,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimWellCollection::RANGE_ADD_INDIVIDUAL, "ALL_ON",             "Individually");
        addItem(RimWellCollection::RANGE_ADD_ALL,        "FORCE_ALL_ON",       "All On");
    }
}

namespace caf
{
    template<>
    void RimWellCollection::WellFenceEnum::setUp()
    {
        addItem(RimWellCollection::K_DIRECTION, "K_DIRECTION",    "K - Direction");
        addItem(RimWellCollection::J_DIRECTION, "J_DIRECTION",    "J - Direction");
        addItem(RimWellCollection::I_DIRECTION, "I_DIRECTION",    "I - Direction");
        setDefault(RimWellCollection::K_DIRECTION);
    }
}

namespace caf
{
    template<>
    void RimWellCollection::WellHeadPositionEnum::setUp()
    {
        addItem(RimWellCollection::WELLHEAD_POS_ACTIVE_CELLS_BB,    "WELLHEAD_POS_ACTIVE_CELLS_BB", "Top of active cells BB");
        addItem(RimWellCollection::WELLHEAD_POS_TOP_COLUMN,         "WELLHEAD_POS_TOP_COLUMN",      "Top of active cells IJ-column");
        setDefault(RimWellCollection::WELLHEAD_POS_TOP_COLUMN);
    }
}

CAF_PDM_SOURCE_INIT(RimWellCollection, "Wells");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellCollection::RimWellCollection()
{
    CAF_PDM_InitObject("Wells", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&isActive,              "Active",        true,   "Active", "", "", "");
    isActive.setUiHidden(true);

    CAF_PDM_InitField(&showWellHead,        "ShowWellHead",     true,   "Show well heads", "", "", "");
    CAF_PDM_InitField(&showWellLabel,       "ShowWellLabel",    true,   "Show well labels", "", "", "");
    CAF_PDM_InitField(&wellHeadScaleFactor, "WellHeadScale",    1.0,    "Well head scale", "", "", "");
    CAF_PDM_InitField(&wellHeadPosition,    "WellHeadPosition", WellHeadPositionEnum(WELLHEAD_POS_TOP_COLUMN), "Well head position",  "", "", "");
    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField(&wellLabelColor,      "WellLabelColor",   defWellLabelColor, "Well label color",  "", "", "");

    CAF_PDM_InitField(&wellPipeVisibility,  "GlobalWellPipeVisibility", WellVisibilityEnum(PIPES_OPEN_IN_VISIBLE_CELLS), "Global well pipe visibility",  "", "", "");

    CAF_PDM_InitField(&pipeRadiusScaleFactor,       "WellPipeRadiusScale",    0.1,                        "Pipe radius scale", "", "", "");
    CAF_PDM_InitField(&pipeCrossSectionVertexCount, "WellPipeVertexCount", 12, "Pipe vertex count", "", "", "");
    pipeCrossSectionVertexCount.setUiHidden(true);

    CAF_PDM_InitField(&wellCellsToRangeFilterMode,  "GlobalWellCellVisibility", WellCellsRangeFilterEnum(RANGE_ADD_NONE),  "Add cells to range filter", "", "", "");
    CAF_PDM_InitField(&showWellCellFences,  "ShowWellFences",           false,                              "Use well fence", "", "", "");
    CAF_PDM_InitField(&wellCellFenceType,   "DefaultWellFenceDirection", WellFenceEnum(K_DIRECTION),        "Well Fence direction", "", "", "");

    CAF_PDM_InitField(&wellCellTransparencyLevel, "WellCellTransparency", 0.5, "Well cell transparency", "", "", "");

    CAF_PDM_InitField(&isAutoDetectingBranches, "IsAutoDetectingBranches", true, "Geometry based branch detection", "", "Toggle wether the well pipe visualization will try to detect when a part of the well \nis really a branch, and thus is starting from wellhead", "");

    CAF_PDM_InitFieldNoDefault(&wells, "Wells", "Wells",  "", "", "");

    m_reservoirView = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellCollection::~RimWellCollection()
{
   wells.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWell* RimWellCollection::findWell(QString name)
{
    for (size_t i = 0; i < this->wells().size(); ++i)
    {
        if (this->wells()[i]->name() == name)
        {
            return this->wells()[i];
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellCollection::hasVisibleWellCells()
{
    if (!this->isActive()) return false;
    if (this->wellCellsToRangeFilterMode() == RANGE_ADD_NONE) return false;
    if (this->wells().size() == 0 ) return false;

    bool hasCells = false;
    for (size_t i = 0 ; !hasCells && i < this->wells().size(); ++i)
    {
        RimWell* well = this->wells()[i];
        if ( well && well->wellResults() && ((well->showWell() && well->showWellCells()) || this->wellCellsToRangeFilterMode() == RANGE_ADD_ALL) )
        {
            for (size_t tIdx = 0; !hasCells &&  tIdx < well->wellResults()->m_wellCellsTimeSteps.size(); ++tIdx )
            {
                const RigWellResultFrame& wellResultFrame = well->wellResults()->m_wellCellsTimeSteps[tIdx];
                for (size_t wsIdx = 0; !hasCells &&  wsIdx < wellResultFrame.m_wellResultBranches.size(); ++wsIdx)
                {
                    if (wellResultFrame.m_wellResultBranches[wsIdx].m_branchResultPoints.size() > 0  ) hasCells = true; 
                }
            }
        }
    }

    if (!hasCells) return false;

    if (this->wellCellsToRangeFilterMode() == RANGE_ADD_INDIVIDUAL || this->wellCellsToRangeFilterMode() == RANGE_ADD_ALL) return true;

    // Todo: Handle range filter intersection

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Used to know if we need animation of timesteps due to the wells
//--------------------------------------------------------------------------------------------------
bool RimWellCollection::hasVisibleWellPipes()
{
    if (!this->isActive()) return false;
    if (this->wellPipeVisibility() == PIPES_FORCE_ALL_OFF) return false;
    if (this->wells().size() == 0 ) return false;
    if (this->wellPipeVisibility() == PIPES_FORCE_ALL_ON) return true;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&showWellLabel == changedField || &isActive == changedField)
    {
        this->updateUiIconFromToggleField();

        if (m_reservoirView) 
        {
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
    if (&wellCellsToRangeFilterMode == changedField)
    {
        if (m_reservoirView) 
        {
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
    else if (&showWellCellFences == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
    else if (&wellCellTransparencyLevel == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
    else if (&wellCellFenceType == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
    else if (&wellPipeVisibility == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
    else if (  &pipeCrossSectionVertexCount == changedField 
            || &pipeRadiusScaleFactor == changedField 
            || &wellHeadScaleFactor == changedField 
            || &showWellHead == changedField
            || &isAutoDetectingBranches == changedField
            || &wellHeadPosition == changedField
            || &wellLabelColor == changedField)
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
void RimWellCollection::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Well range filter");
    filterGroup->add(&wellCellsToRangeFilterMode);
    filterGroup->add(&showWellCellFences);
    filterGroup->add(&wellCellFenceType);

    caf::PdmUiGroup* wellHeadGroup = uiOrdering.addNewGroup("Well head");
    wellHeadGroup->add(&showWellHead);
    wellHeadGroup->add(&wellHeadScaleFactor);
    wellHeadGroup->add(&showWellLabel);
    wellHeadGroup->add(&wellHeadPosition);
    wellHeadGroup->add(&wellLabelColor);

    caf::PdmUiGroup* wellPipe = uiOrdering.addNewGroup("Well pipe");
    wellPipe->add(&wellPipeVisibility);
    wellPipe->add(&pipeRadiusScaleFactor);

    caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup("Advanced");
    advancedGroup->add(&wellCellTransparencyLevel);
    advancedGroup->add(&isAutoDetectingBranches);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellCollection::objectToggleField()
{
    return &isActive;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::ubyte>& RimWellCollection::isWellPipesVisible(size_t frameIndex)
{
    calculateIsWellPipesVisible(frameIndex);
    return m_isWellPipesVisible[frameIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellCollection::scheduleIsWellPipesVisibleRecalculation()
{
    m_isWellPipesVisible.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellCollection::calculateIsWellPipesVisible(size_t frameIndex)
{
    if (m_isWellPipesVisible.size() > frameIndex && m_isWellPipesVisible[frameIndex].size()) return;

    if (m_isWellPipesVisible.size() <= frameIndex)
        m_isWellPipesVisible.resize(frameIndex+1);

    if (m_isWellPipesVisible[frameIndex].size() <= wells().size())
        m_isWellPipesVisible[frameIndex].resize(wells().size(), false); 
    
    for (size_t i = 0; i < wells().size(); ++i)
    {
        m_isWellPipesVisible[frameIndex][i] = wells[i]->calculateWellPipeVisibility(frameIndex);
    }
}
