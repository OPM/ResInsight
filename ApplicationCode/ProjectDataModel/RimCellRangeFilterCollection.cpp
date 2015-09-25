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

#include "RimCellRangeFilterCollection.h"

#include "RigCaseData.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigGridBase.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimViewLink.h"
#include "RimViewLinker.h"

#include "cafPdmUiEditorHandle.h"


CAF_PDM_SOURCE_INIT(RimCellRangeFilterCollection, "CellRangeFilterCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection::RimCellRangeFilterCollection()
{
    CAF_PDM_InitObject("Range Filters", ":/CellFilter_Range.png", "", "");

    CAF_PDM_InitFieldNoDefault(&rangeFilters,   "RangeFilters", "Range Filters", "", "", "");
    rangeFilters.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isActive,                  "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection::~RimCellRangeFilterCollection()
{
    rangeFilters.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// RimCellRangeFilter is using Eclipse 1-based indexing, adjust filter values before 
//  populating cvf::CellRangeFilter (which is 0-based)
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::compoundCellRangeFilter(cvf::CellRangeFilter* cellRangeFilter, size_t gridIndex) const
{
    CVF_ASSERT(cellRangeFilter);

    for (size_t i = 0; i < rangeFilters.size(); i++)
    {
        RimCellRangeFilter* rangeFilter = rangeFilters[i];

        if (rangeFilter && rangeFilter->isActive() && static_cast<size_t>(rangeFilter->gridIndex()) == gridIndex)
        {
            if (rangeFilter->filterMode == RimCellFilter::INCLUDE)
            {
                cellRangeFilter->addCellIncludeRange(
                    rangeFilter->startIndexI - 1,
                    rangeFilter->startIndexJ - 1,
                    rangeFilter->startIndexK - 1,
                    rangeFilter->startIndexI - 1 + rangeFilter->cellCountI,
                    rangeFilter->startIndexJ - 1 + rangeFilter->cellCountJ,
                    rangeFilter->startIndexK - 1 + rangeFilter->cellCountK,
                    rangeFilter->propagateToSubGrids());
            }
            else
            {
                cellRangeFilter->addCellExcludeRange(
                    rangeFilter->startIndexI - 1,
                    rangeFilter->startIndexJ - 1,
                    rangeFilter->startIndexK - 1,
                    rangeFilter->startIndexI - 1 + rangeFilter->cellCountI,
                    rangeFilter->startIndexJ - 1 + rangeFilter->cellCountJ,
                    rangeFilter->startIndexK - 1 + rangeFilter->cellCountK, 
                    rangeFilter->propagateToSubGrids());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimCellRangeFilterCollection::mainGrid() const
{
    RimEclipseView* eclipseView = this->eclipseView();
    if (eclipseView &&
        eclipseView->eclipseCase() &&
        eclipseView->eclipseCase()->reservoirData() &&
        eclipseView->eclipseCase()->reservoirData()->mainGrid())
    {

        return eclipseView->eclipseCase()->reservoirData()->mainGrid();
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimCellRangeFilterCollection::activeCellInfo() const
{
    RimEclipseView* eclipseView = this->eclipseView();
    if (eclipseView)
    {
        return eclipseView->currentActiveCellInfo();
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateIconState();
    
    updateDisplayModeNotifyManagedViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::updateDisplayModeNotifyManagedViews()
{
    RimView* view = NULL;
    firstAnchestorOrThisOfType(view);

    view->scheduleGeometryRegen(RANGE_FILTERED);
    view->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

    view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimCellRangeFilterCollection::eclipseView() const
{
    return dynamic_cast<RimEclipseView*>(baseView());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::initAfterRead()
{
    for (size_t i = 0; i < rangeFilters.size(); i++)
    {
        RimCellRangeFilter* rangeFilter = rangeFilters[i];
        rangeFilter->updateIconState();
    }

    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellRangeFilterCollection::hasActiveFilters() const
{
    if (!isActive()) return false; 

    for (size_t i = 0; i < rangeFilters.size(); i++)
    {
        if (rangeFilters[i]->isActive()) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellRangeFilterCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellRangeFilterCollection::hasActiveIncludeFilters() const
{
    if (!isActive) return false; 

    for (size_t i = 0; i < rangeFilters.size(); i++)
    {
        if (rangeFilters[i]->isActive() && rangeFilters[i]->filterMode() == RimCellFilter::INCLUDE) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::StructGridInterface* RimCellRangeFilterCollection::gridByIndex(int gridIndex) const
{
    RigMainGrid* mnGrid = mainGrid();
    RigFemPartCollection* femPartColl = this->femPartColl();

    if (mnGrid)
    {
        RigGridBase* grid = NULL;

        grid = mnGrid->gridByIndex(gridIndex);

        CVF_ASSERT(grid);

        return grid;
    }
    else if (femPartColl)
    {
        if (gridIndex < femPartColl->partCount())
            return femPartColl->part(gridIndex)->structGrid();
        else 
            return NULL;
    }
    
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimCellRangeFilterCollection::gridCount() const
{
    RigMainGrid* mnGrid = mainGrid();
    RigFemPartCollection* femPartColl = this->femPartColl();

    if (mnGrid)
    {
        return (int)mnGrid->gridCount();
    }
    else if (femPartColl)
    {
        return femPartColl->partCount();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCellRangeFilterCollection::gridName(int gridIndex) const
{
    RigMainGrid* mnGrid = mainGrid();
    RigFemPartCollection* femPartColl = this->femPartColl();

    if (mnGrid)
    {
        return mnGrid->gridByIndex(gridIndex)->gridName().c_str();
    }
    else if (femPartColl)
    {
        return QString::number(gridIndex);
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartCollection* RimCellRangeFilterCollection::femPartColl() const
{
    RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(baseView());
    if (geoView &&
        geoView->geoMechCase() &&
        geoView->geoMechCase()->geoMechData() )
    {
        return geoView->geoMechCase()->geoMechData()->femParts();
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RimCellRangeFilterCollection::baseView() const
{
    RimView* rimView = NULL;
    firstAnchestorOrThisOfType(rimView);

    return rimView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::updateIconState()
{
    bool activeIcon = true;

    RimViewController* viewController = baseView()->viewController();
    if (viewController && viewController->isRangeFilterOveridden())
    {
        activeIcon = false;
    }

    if (!isActive)
    {
        activeIcon = false;
    }

    updateUiIconFromState(activeIcon);

    uiCapability()->updateConnectedEditors();
}

