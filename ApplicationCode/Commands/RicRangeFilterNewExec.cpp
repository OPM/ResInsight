/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2014 Ceetron Solutions AS, USFOS AS, AMOS - NTNU
// 
//  RPM is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  RPM is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicRangeFilterNewExec.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimView.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRangeFilterNewExec::RicRangeFilterNewExec(caf::NotificationCenter* notificationCenter)
    : CmdExecuteCommand(notificationCenter)
{
    m_iSlice = false;
    m_jSlice = false;
    m_kSlice = false;

    m_iSliceStart = -1;
    m_jSliceStart = -1;
    m_kSliceStart = -1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicRangeFilterNewExec::name()
{
    if (m_iSlice)
        return "Create I Slice Filter";
    else if (m_jSlice)
        return "Create J Slice Filter";
    else if (m_kSlice)
        return "Create K Slice Filter";

    return "Create Range Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewExec::redo()
{
    assert(cellRangeFilterCollection);

    RimCellRangeFilter* rangeFilter = new RimCellRangeFilter();
    rangeFilter->setParentContainer(cellRangeFilterCollection);
    rangeFilter->setDefaultValues();

    rangeFilter->name = QString("Range Filter (%1)").arg(cellRangeFilterCollection->rangeFilters().size());

    if (m_iSlice)
    {
        rangeFilter->cellCountI = 1;
        rangeFilter->name = QString("I Slice Filter (%1)").arg(cellRangeFilterCollection->rangeFilters().size());
    }
    
    if (m_jSlice)
    {
        rangeFilter->cellCountJ = 1;
        rangeFilter->name = QString("J Slice Filter (%1)").arg(cellRangeFilterCollection->rangeFilters().size());
    }
    
    if (m_kSlice)
    {
        rangeFilter->cellCountK = 1;
        rangeFilter->name = QString("K Slice Filter (%1)").arg(cellRangeFilterCollection->rangeFilters().size());
    }

    if (m_iSliceStart > -1) rangeFilter->startIndexI = m_iSliceStart;
    if (m_jSliceStart > -1) rangeFilter->startIndexJ = m_jSliceStart;
    if (m_kSliceStart > -1) rangeFilter->startIndexK = m_kSliceStart;

    cellRangeFilterCollection->rangeFilters.push_back(rangeFilter);

    cellRangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED);
    cellRangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

    caf::PdmUiFieldHandle::updateConnectedUiEditors(cellRangeFilterCollection->parentField());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewExec::undo()
{
    assert(cellRangeFilterCollection);

    cellRangeFilterCollection->rangeFilters.erase(cellRangeFilterCollection->rangeFilters.size() - 1);

    caf::PdmUiFieldHandle::updateConnectedUiEditors(cellRangeFilterCollection->parentField());
}
