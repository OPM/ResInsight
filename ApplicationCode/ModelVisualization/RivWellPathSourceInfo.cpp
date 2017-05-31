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

#include "RivWellPathSourceInfo.h"

#include "RigWellPath.h"

#include "RimCase.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RivWellPathPartMgr.h"

#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathSourceInfo::RivWellPathSourceInfo(RimWellPath* wellPath)
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RivWellPathSourceInfo::wellPath() const
{
    return m_wellPath.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RivWellPathSourceInfo::measuredDepth(size_t triangleIndex, const cvf::Vec3d& globalIntersection) const
{
    size_t firstSegmentIndex = cvf::UNDEFINED_SIZE_T;
    double norm = 0.0;

    normalizedIntersection(triangleIndex, globalIntersection, &firstSegmentIndex, &norm);

    double firstDepth = m_wellPath->wellPathGeometry()->m_measuredDepths[firstSegmentIndex];
    double secDepth = m_wellPath->wellPathGeometry()->m_measuredDepths[firstSegmentIndex + 1];

    return firstDepth * (1.0 - norm) + norm * secDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivWellPathSourceInfo::trueVerticalDepth(size_t triangleIndex, const cvf::Vec3d& globalIntersection) const
{
    size_t firstSegmentIndex = cvf::UNDEFINED_SIZE_T;
    double norm = 0.0;

    normalizedIntersection(triangleIndex, globalIntersection, &firstSegmentIndex, &norm);

    cvf::Vec3d firstDepth = m_wellPath->wellPathGeometry()->m_wellPathPoints[firstSegmentIndex];
    cvf::Vec3d secDepth = m_wellPath->wellPathGeometry()->m_wellPathPoints[firstSegmentIndex + 1];

    return firstDepth * (1.0 - norm) + norm * secDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathSourceInfo::normalizedIntersection(size_t triangleIndex, const cvf::Vec3d& globalIntersection,
                                                    size_t* firstSegmentIndex, double* normalizedSegmentIntersection) const
{
    size_t segIndex = segmentIndex(triangleIndex);

    RigWellPath* rigWellPath = m_wellPath->wellPathGeometry();

    cvf::Vec3d segmentStart = rigWellPath->m_wellPathPoints[segIndex];
    cvf::Vec3d segmentEnd = rigWellPath->m_wellPathPoints[segIndex + 1];

    double norm = 0.0;
    cvf::Vec3d pointOnLine = cvf::GeometryTools::projectPointOnLine(segmentStart, segmentEnd, globalIntersection, &norm);
    norm = cvf::Math::clamp(norm, 0.0, 1.0);

    *firstSegmentIndex = segIndex;
    *normalizedSegmentIntersection = norm;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RivWellPathSourceInfo::segmentIndex(size_t triangleIndex) const
{
    return m_wellPath->partMgr()->segmentIndexFromTriangleIndex(triangleIndex);
}

