/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivSourceInfo.h"

#include "cvfStructGridGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSourceInfo::RivSourceInfo(caf::PdmObject* pdmObject, size_t gridIndex)
    : RivObjectSourceInfo(pdmObject)
    , m_gridIndex(gridIndex)
    , m_cellSetType(ALL_CELLS)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivSourceInfo::gridIndex() const
{
    return m_gridIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivSourceInfo::hasCellFaceMapping() const
{
    return m_cellFaceFromTriangleMapper.notNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivSourceInfo::hasNNCIndices() const
{
    return m_NNCIndices.notNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivCellSetEnum RivSourceInfo::cellSetType() const
{
    return m_cellSetType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSourceInfo::setCellSetType(RivCellSetEnum cellSetEnum)
{
    m_cellSetType = cellSetEnum;
}
