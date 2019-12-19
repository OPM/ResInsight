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

#include "RicRangeFilterFeatureImpl.h"

#include "RicRangeFilterNewExec.h"

#include "RiaApplication.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimGridView.h"
#include "RimViewController.h"

#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterFeatureImpl::isRangeFilterCommandAvailable()
{
    RimCellRangeFilterCollection* rangeFilterCollection = findRangeFilterCollection();
    if ( !rangeFilterCollection ) return false;

    RimGridView* view;
    rangeFilterCollection->firstAncestorOrThisOfType( view );
    if ( view )
    {
        RimViewController* vc = view->viewController();
        if ( !vc ) return true;
        return ( !vc->isRangeFiltersControlled() );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRangeFilterNewExec* RicRangeFilterFeatureImpl::createRangeFilterExecCommand()
{
    RimCellRangeFilterCollection* rangeFilterCollection = findRangeFilterCollection();

    RicRangeFilterNewExec* filterExec = new RicRangeFilterNewExec( rangeFilterCollection );

    return filterExec;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection* RicRangeFilterFeatureImpl::findRangeFilterCollection()
{
    RimCellRangeFilterCollection* rangeFilterCollection = nullptr;

    rangeFilterCollection = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimCellRangeFilterCollection>();

    if ( !rangeFilterCollection )
    {
        RimGridView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
        if ( view )
        {
            rangeFilterCollection = view->rangeFilterCollection();
        }
    }

    return rangeFilterCollection;
}
