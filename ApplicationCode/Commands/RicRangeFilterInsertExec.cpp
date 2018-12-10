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

#include "RicRangeFilterInsertExec.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "Rim3dView.h"
#include "Riu3DMainWindowTools.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRangeFilterInsertExec::RicRangeFilterInsertExec(RimCellRangeFilterCollection* rangeFilterCollection, RimCellRangeFilter* rangeFilter)
    : RicRangeFilterExecImpl(rangeFilterCollection, rangeFilter)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRangeFilterInsertExec::~RicRangeFilterInsertExec()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicRangeFilterInsertExec::name()
{
    return "Create Range Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterInsertExec::redo()
{
    RimCellRangeFilter* rangeFilter = createRangeFilter();
    if (rangeFilter)
    {
        size_t index = m_cellRangeFilterCollection->rangeFilters.index(m_insertBeforeCellRangeFilter);
        CVF_ASSERT(index < m_cellRangeFilterCollection->rangeFilters.size());

        m_cellRangeFilterCollection->rangeFilters.insertAt(static_cast<int>(index), rangeFilter);

        rangeFilter->setDefaultValues();
        applyCommandDataOnFilter(rangeFilter);

        m_cellRangeFilterCollection->updateDisplayModeNotifyManagedViews(nullptr);

        m_cellRangeFilterCollection->updateConnectedEditors();

        Riu3DMainWindowTools::selectAsCurrentItem(rangeFilter);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterInsertExec::undo()
{
}
