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

#pragma once

#include "cafCmdExecuteCommand.h"
#include "cafPdmPointer.h"

class RimCellRangeFilter;
class RimCellRangeFilterCollection;

//==================================================================================================
/// 
//==================================================================================================
class RicRangeFilterExecImpl : public caf::CmdExecuteCommand
{
public:
    RicRangeFilterExecImpl(RimCellRangeFilterCollection* rangeFilterCollection, 
                           RimCellRangeFilter* insertBeforeCellRangeFilter = nullptr);
    ~RicRangeFilterExecImpl() override;

    QString name() override = 0;
    void redo() override = 0;
    void undo() override = 0;

public:
    bool m_iSlice;
    bool m_jSlice;
    bool m_kSlice;

    int m_gridIndex; 
    int m_iSliceStart;
    int m_jSliceStart;
    int m_kSliceStart;

protected:
    RimCellRangeFilter* createRangeFilter();
    void                applyCommandDataOnFilter(RimCellRangeFilter* filter);

protected:
    caf::PdmPointer<RimCellRangeFilterCollection>   m_cellRangeFilterCollection;
    caf::PdmPointer<RimCellRangeFilter>             m_insertBeforeCellRangeFilter;
};


