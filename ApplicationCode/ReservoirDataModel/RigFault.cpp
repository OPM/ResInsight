/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigFault.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFault::RigFault()
{
    m_cellRangesForFaces.resize(6);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFault::addCellRangeForFace(cvf::StructGridInterface::FaceType face, const cvf::CellRange& cellRange)
{
    size_t faceIndex = static_cast<size_t>(face);
    CVF_ASSERT(faceIndex < m_cellRangesForFaces.size());

    m_cellRangesForFaces[faceIndex].push_back(cellRange);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFault::setName(const QString& name)
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigFault::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::CellRange>& RigFault::cellRangeForFace(cvf::StructGridInterface::FaceType face) const
{
    size_t faceIndex = static_cast<size_t>(face);
    CVF_ASSERT(faceIndex < m_cellRangesForFaces.size());

    return m_cellRangesForFaces[faceIndex];
}
