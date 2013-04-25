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

#include "RiaStdInclude.h"


#include "cafAppEnum.h"
#include "cafPdmFieldCvfColor.h"

#include "RigSingleWellResultsData.h"
#include "RimWellCollection.h"
#include "RimWell.h"
#include "RimReservoirView.h"

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
        addItem(RimWellCollection::RANGE_ADD_NONE,       "FORCE_ALL_OFF",      "Off");
        addItem(RimWellCollection::RANGE_ADD_INDIVIDUAL, "ALL_ON",             "Individually");
        addItem(RimWellCollection::RANGE_ADD_ALL,        "FORCE_ALL_ON",       "On");
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


CAF_PDM_SOURCE_INIT(RimWellCollection, "Wells");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellCollection::RimWellCollection()
{
    CAF_PDM_InitObject("Wells", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&showWells,           "ShowWells",        true,   "Show well", "", "", "");
    showWells.setUiHidden(true);

    CAF_PDM_InitField(&showWellHead,        "ShowWellHead",     true,   "Show well heads", "", "", "");
    CAF_PDM_InitField(&showWellLabel,       "ShowWellLabel",    true,   "Show well labels", "", "", "");
    CAF_PDM_InitField(&wellHeadScaleFactor, "WellHeadScale",    1.0,    "Well head scale", "", "", "");

    CAF_PDM_InitField(&wellPipeVisibility,  "GlobalWellPipeVisibility", WellVisibilityEnum(PIPES_INDIVIDUALLY), "Global well pipe visibility",  "", "", "");

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
    if (this->wellCellsToRangeFilterMode() == RANGE_ADD_NONE) return false;
    if (this->wells().size() == 0 ) return false;

    bool hasCells = false;
    for (size_t i = 0 ; !hasCells && i < this->wells().size(); ++i)
    {
        RimWell* well = this->wells()[i];
        if ( well && well->wellResults() && (well->showWellCells() || this->wellCellsToRangeFilterMode() == RANGE_ADD_ALL) )
        {
            for (size_t tIdx = 0; !hasCells &&  tIdx < well->wellResults()->m_wellCellsTimeSteps.size(); ++tIdx )
            {
                const RigWellResultFrame& wellResultFrame = well->wellResults()->m_wellCellsTimeSteps[tIdx];
                for (size_t wsIdx = 0; !hasCells &&  wsIdx < wellResultFrame.m_wellResultBranches.size(); ++wsIdx)
                {
                    if (wellResultFrame.m_wellResultBranches[wsIdx].m_wellCells.size() > 0  ) hasCells = true; 
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
    if (!this->showWells()) return false;
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
    if (&showWellLabel == changedField || &showWells == changedField)
    {
        if (m_reservoirView) 
        {
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    if (&wellCellsToRangeFilterMode == changedField)
    {
        if (m_reservoirView) 
        {
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    else if (&showWellCellFences == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    else if (&wellCellTransparencyLevel == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    else if (&wellCellFenceType == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    else if (&wellPipeVisibility == changedField)
    {
        if (m_reservoirView) 
        {   
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    else if (  &pipeCrossSectionVertexCount == changedField 
            || &pipeRadiusScaleFactor == changedField 
            || &wellHeadScaleFactor == changedField 
            || &showWellHead == changedField
            || &isAutoDetectingBranches == changedField)
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
void RimWellCollection::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Range filter");
    filterGroup->add(&wellCellsToRangeFilterMode);
    filterGroup->add(&showWellCellFences);
    filterGroup->add(&wellCellFenceType);

    caf::PdmUiGroup* wellHeadGroup = uiOrdering.addNewGroup("Well head");
    wellHeadGroup->add(&showWellHead);
    wellHeadGroup->add(&wellHeadScaleFactor);
    wellHeadGroup->add(&showWellLabel);

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
    return &showWells;
}
