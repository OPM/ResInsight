/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicRangeFilterHelper.h"
#include "RicRangeFilterNewExec.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterHelper::isRangeFilterCommandAvailable()
{
    return findRangeFilterCollection() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRangeFilterNewExec* RicRangeFilterHelper::createRangeFilterExecCommand()
{
    RimCellRangeFilterCollection* rangeFilterCollection = findRangeFilterCollection();

    RicRangeFilterNewExec* filterExec = new RicRangeFilterNewExec(rangeFilterCollection);

    return filterExec;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection* RicRangeFilterHelper::findRangeFilterCollection()
{
    RimCellRangeFilterCollection* rangeFilterCollection = NULL;
    
    std::vector<RimCellRangeFilter*> selectedRangeFilter;
    caf::SelectionManager::instance()->objectsByType(&selectedRangeFilter);

    std::vector<RimCellRangeFilterCollection*> selectedRangeFilterCollection;
    caf::SelectionManager::instance()->objectsByType(&selectedRangeFilterCollection);

    if (selectedRangeFilterCollection.size() == 1)
    {
        rangeFilterCollection = selectedRangeFilterCollection[0];
    }
    else if (selectedRangeFilter.size() > 0)
    {
        selectedRangeFilter[0]->firstAnchestorOrThisOfType(rangeFilterCollection);
    }

    assert(rangeFilterCollection);

    // TODO : When a menu is created in the 3D view, add code to find collection based on a RimView
    // See RiuViewerCommands

    return rangeFilterCollection;
}
