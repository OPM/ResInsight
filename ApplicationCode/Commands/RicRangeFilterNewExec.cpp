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
        return "Add I-slice Filter";
    else if (m_jSlice)
        return "Add J-slice Filter";
    else if (m_kSlice)
        return "Add K-slice Filter";

    return "Add Range Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewExec::redo()
{
    assert(cellRangeFilterCollection);

    RimCellRangeFilter* newFilter = cellRangeFilterCollection->createAndAppendRangeFilter();
    if (m_iSlice)
    {
        newFilter->cellCountI = 1;
    }
    
    if (m_jSlice)
    {
        newFilter->cellCountJ = 1;
    }
    
    if (m_kSlice)
    {
        newFilter->cellCountK = 1;
    }

    if (m_iSliceStart > -1)    newFilter->startIndexI = m_iSliceStart;
    if (m_jSliceStart > -1)    newFilter->startIndexJ = m_jSliceStart;
    if (m_kSliceStart > -1)    newFilter->startIndexK = m_kSliceStart;

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
