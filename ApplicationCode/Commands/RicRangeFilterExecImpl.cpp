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

#include "RicRangeFilterExecImpl.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRangeFilterExecImpl::RicRangeFilterExecImpl(RimCellRangeFilterCollection* rangeFilterCollection, RimCellRangeFilter* rangeFilter)
    : CmdExecuteCommand(nullptr)
{
    CVF_ASSERT(rangeFilterCollection);
    m_cellRangeFilterCollection = rangeFilterCollection;

    m_cellRangeFilter = rangeFilter;

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
RicRangeFilterExecImpl::~RicRangeFilterExecImpl()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RicRangeFilterExecImpl::createRangeFilter()
{
    CVF_ASSERT(m_cellRangeFilterCollection);

    RimCellRangeFilter* rangeFilter = new RimCellRangeFilter();

    size_t flterIndex = m_cellRangeFilterCollection->rangeFilters().size() + 1;

    rangeFilter->name = QString("New Filter (%1)").arg(flterIndex);

    if (m_iSlice)
    {
        rangeFilter->name = QString("Slice I (%1)").arg(flterIndex);
    }

    if (m_jSlice)
    {
        rangeFilter->name = QString("Slice J (%1)").arg(flterIndex);
    }

    if (m_kSlice)
    {
        rangeFilter->name = QString("Slice K (%1)").arg(flterIndex);
    }

    return rangeFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRangeFilterExecImpl::applyCommandDataOnFilter(RimCellRangeFilter* rangeFilter)
{
    if (m_iSlice)
    {
        rangeFilter->cellCountI = 1;
    }

    if (m_jSlice)
    {
        rangeFilter->cellCountJ = 1;
    }

    if (m_kSlice)
    {
        rangeFilter->cellCountK = 1;
    }

    if (m_iSliceStart > -1) rangeFilter->startIndexI = m_iSliceStart;
    if (m_jSliceStart > -1) rangeFilter->startIndexJ = m_jSliceStart;
    if (m_kSliceStart > -1) rangeFilter->startIndexK = m_kSliceStart;
}

