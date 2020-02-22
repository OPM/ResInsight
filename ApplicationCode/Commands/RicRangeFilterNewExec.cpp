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

#include "RicRangeFilterNewExec.h"

#include "RiaApplication.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimGridView.h"

#include "Riu3DMainWindowTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRangeFilterNewExec::RicRangeFilterNewExec( RimCellRangeFilterCollection* rangeFilterCollection,
                                              RimCellRangeFilter*           rangeFilter )
    : RicRangeFilterExecImpl( rangeFilterCollection, rangeFilter )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRangeFilterNewExec::~RicRangeFilterNewExec()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRangeFilterNewExec::name()
{
    if ( m_iSlice )
        return "Create I Slice Filter";
    else if ( m_jSlice )
        return "Create J Slice Filter";
    else if ( m_kSlice )
        return "Create K Slice Filter";

    return "Create Range Filter";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewExec::redo()
{
    RimCellRangeFilter* rangeFilter = createRangeFilter();
    if ( rangeFilter )
    {
        m_cellRangeFilterCollection->rangeFilters.push_back( rangeFilter );

        rangeFilter->setDefaultValues();
        applyCommandDataOnFilter( rangeFilter );

        m_cellRangeFilterCollection->updateDisplayModeNotifyManagedViews( nullptr );

        m_cellRangeFilterCollection->updateConnectedEditors();

        Riu3DMainWindowTools::selectAsCurrentItem( rangeFilter, false );

        // Trigger update of view following the range filter update
        RimGridView* view = nullptr;
        m_cellRangeFilterCollection->firstAncestorOrThisOfTypeAsserted( view );
        view->rangeFiltersUpdated();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewExec::undo()
{
    assert( m_cellRangeFilterCollection );

    m_cellRangeFilterCollection->rangeFilters.erase( m_cellRangeFilterCollection->rangeFilters.size() - 1 );

    m_cellRangeFilterCollection->updateDisplayModeNotifyManagedViews( nullptr );

    m_cellRangeFilterCollection->updateConnectedEditors();
}