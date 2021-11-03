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

#include "RivExtrudedCurveIntersectionSourceInfo.h"

#include "RimExtrudedCurveIntersection.h"
#include "RivExtrudedCurveIntersectionGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivExtrudedCurveIntersectionSourceInfo::RivExtrudedCurveIntersectionSourceInfo(
    RivExtrudedCurveIntersectionGeometryGenerator* geometryGenerator,
    RimExtrudedCurveIntersection*                  intersection )
    : m_intersectionGeometryGenerator( geometryGenerator )
    , m_intersection( intersection )
{
    CVF_ASSERT( m_intersectionGeometryGenerator.notNull() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivExtrudedCurveIntersectionSourceInfo::triangleToCellIndex() const
{
    CVF_ASSERT( m_intersectionGeometryGenerator.notNull() );

    return m_intersectionGeometryGenerator->triangleToCellIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3f, 3> RivExtrudedCurveIntersectionSourceInfo::triangle( int triangleIdx ) const
{
    std::array<cvf::Vec3f, 3> tri;
    tri[0] = ( *m_intersectionGeometryGenerator->triangleVxes() )[triangleIdx * 3];
    tri[1] = ( *m_intersectionGeometryGenerator->triangleVxes() )[triangleIdx * 3 + 1];
    tri[2] = ( *m_intersectionGeometryGenerator->triangleVxes() )[triangleIdx * 3 + 2];

    return tri;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtrudedCurveIntersection* RivExtrudedCurveIntersectionSourceInfo::intersection() const
{
    return m_intersection;
}
