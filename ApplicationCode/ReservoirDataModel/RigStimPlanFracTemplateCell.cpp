/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigStimPlanFracTemplateCell.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFracTemplateCell::RigStimPlanFracTemplateCell()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFracTemplateCell::RigStimPlanFracTemplateCell(std::vector<cvf::Vec3d> polygon, size_t i, size_t j)
{
    m_polygon = polygon;
    m_i = i;
    m_j = j;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStimPlanFracTemplateCell::~RigStimPlanFracTemplateCell()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFracTemplateCell::cellSizeX() const
{
    //The polygon corners are always stored in the same order
    if (m_polygon.size()>1) return (m_polygon[1] - m_polygon[0]).length();
    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigStimPlanFracTemplateCell::cellSizeZ() const
{
    if (m_polygon.size()>2) return (m_polygon[2] - m_polygon[1]).length();
    return cvf::UNDEFINED_DOUBLE;
}

