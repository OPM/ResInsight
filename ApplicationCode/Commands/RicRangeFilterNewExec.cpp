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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRangeFilterNewExec::RicRangeFilterNewExec(caf::NotificationCenter* notificationCenter)
    : CmdExecuteCommand(notificationCenter)
{
    m_filterI = false;
    m_filterJ = false;
    m_filterK = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicRangeFilterNewExec::name()
{
    return "New Range Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewExec::redo()
{
    assert(cellRangeFilterCollection);

    RimCellRangeFilter* newFilter = cellRangeFilterCollection->createAndAppendRangeFilter();
    if (m_filterI)
    {
        newFilter->cellCountI = 1;
    }
    
    if (m_filterJ)
    {
        newFilter->cellCountJ = 1;
    }
    
    if (m_filterK)
    {
        newFilter->cellCountK = 1;
    }

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
