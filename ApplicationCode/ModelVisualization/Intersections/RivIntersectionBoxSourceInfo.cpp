/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RivIntersectionBoxSourceInfo.h"

#include "RivIntersectionBoxGeometryGenerator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxSourceInfo::RivIntersectionBoxSourceInfo(RivIntersectionBoxGeometryGenerator* geometryGenerator)
    : m_intersectionBoxGeometryGenerator(geometryGenerator)
{
    CVF_ASSERT(m_intersectionBoxGeometryGenerator.notNull());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivIntersectionBoxSourceInfo::triangleToCellIndex() const
{
    CVF_ASSERT(m_intersectionBoxGeometryGenerator.notNull());

    return m_intersectionBoxGeometryGenerator->triangleToCellIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3f, 3> RivIntersectionBoxSourceInfo::triangle(int triangleIdx) const
{
    std::array<cvf::Vec3f, 3> tri;
    tri[0] = (*m_intersectionBoxGeometryGenerator->triangleVxes())[triangleIdx*3];
    tri[1] = (*m_intersectionBoxGeometryGenerator->triangleVxes())[triangleIdx*3+1];
    tri[2] = (*m_intersectionBoxGeometryGenerator->triangleVxes())[triangleIdx*3+2];

    return tri;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBox* RivIntersectionBoxSourceInfo::intersectionBox() const
{
    return m_intersectionBoxGeometryGenerator->intersectionBox();
}

