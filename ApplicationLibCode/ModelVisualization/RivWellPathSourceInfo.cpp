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

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimWellPath.h"
#include "RivPipeGeometryGenerator.h"

#include "RimWellPathCollection.h"

#include "RivWellPathPartMgr.h"

#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellPathSourceInfo::RivWellPathSourceInfo( RimWellPath* wellPath, RivPipeGeometryGenerator* pipeGeomGenerator )
    : m_wellPath( wellPath )
    , m_pipeGeomGenerator( pipeGeomGenerator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellPathSourceInfo::~RivWellPathSourceInfo()
{
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
double RivWellPathSourceInfo::measuredDepth( size_t triangleIndex, const cvf::Vec3d& globalIntersectionInDomain ) const
{
    size_t firstSegmentIndex = cvf::UNDEFINED_SIZE_T;
    double norm              = 0.0;

    CAF_ASSERT( m_wellPath.notNull() );
    auto wellPathGeometry = m_wellPath->wellPathGeometry();
    CAF_ASSERT( wellPathGeometry );

    normalizedIntersection( triangleIndex, globalIntersectionInDomain, &firstSegmentIndex, &norm );

    double firstDepth = wellPathGeometry->measuredDepths()[firstSegmentIndex];
    double secDepth   = wellPathGeometry->measuredDepths()[firstSegmentIndex + 1];

    return firstDepth * ( 1.0 - norm ) + norm * secDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivWellPathSourceInfo::closestPointOnCenterLine( size_t            triangleIndex,
                                                            const cvf::Vec3d& globalIntersectionInDomain ) const
{
    size_t firstSegmentIndex = cvf::UNDEFINED_SIZE_T;
    double norm              = 0.0;

    CAF_ASSERT( m_wellPath.notNull() );
    auto wellPathGeometry = m_wellPath->wellPathGeometry();
    CAF_ASSERT( wellPathGeometry );

    normalizedIntersection( triangleIndex, globalIntersectionInDomain, &firstSegmentIndex, &norm );

    cvf::Vec3d firstDepth = wellPathGeometry->wellPathPoints()[firstSegmentIndex];
    cvf::Vec3d secDepth   = wellPathGeometry->wellPathPoints()[firstSegmentIndex + 1];

    return firstDepth * ( 1.0 - norm ) + norm * secDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathSourceInfo::normalizedIntersection( size_t            triangleIndex,
                                                    const cvf::Vec3d& globalIntersectionInDomain,
                                                    size_t*           firstSegmentIndex,
                                                    double*           normalizedSegmentIntersection ) const
{
    CAF_ASSERT( m_wellPath.notNull() );
    auto wellPathGeometry = m_wellPath->wellPathGeometry();
    CAF_ASSERT( wellPathGeometry );

    size_t segIndex = segmentIndex( triangleIndex );

    cvf::Vec3d segmentStart = wellPathGeometry->wellPathPoints()[segIndex];
    cvf::Vec3d segmentEnd   = wellPathGeometry->wellPathPoints()[segIndex + 1];

    double norm = 0.0;
    cvf::GeometryTools::projectPointOnLine( segmentStart, segmentEnd, globalIntersectionInDomain, &norm );
    norm = std::clamp( norm, 0.0, 1.0 );

    *firstSegmentIndex             = segIndex;
    *normalizedSegmentIntersection = norm;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivWellPathSourceInfo::segmentIndex( size_t triangleIndex ) const
{
    CAF_ASSERT( m_wellPath.notNull() );
    return m_pipeGeomGenerator->segmentIndexFromTriangleIndex( triangleIndex ) +
           m_wellPath->wellPathGeometry()->uniqueStartIndex();
}
