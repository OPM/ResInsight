/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RicMswBranch.h"

#include "RicMswCompletions.h"
#include "RicMswSegment.h"

#include "RimWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswBranch::RicMswBranch( const QString& label, const RimWellPath* wellPath, double initialMD, double initialTVD )
    : RicMswItem( label )
    , m_initialMD( initialMD )
    , m_initialTVD( initialTVD )
    , m_branchNumber( -1 )
    , m_wellPath( wellPath )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::addSegment( std::unique_ptr<RicMswSegment> segment )
{
    m_segments.push_back( std::move( segment ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::insertAfterSegment( const RicMswSegment* insertAfter, std::unique_ptr<RicMswSegment> insertItem )
{
    auto it = std::find_if( m_segments.begin(), m_segments.end(), [insertAfter]( auto& item ) {
        return item.get() == insertAfter;
    } );

    m_segments.insert( it, std::move( insertItem ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::sortSegments()
{
    std::stable_sort( m_segments.begin(),
                      m_segments.end(),
                      []( const std::unique_ptr<RicMswSegment>& lhs, const std::unique_ptr<RicMswSegment>& rhs ) {
                          return *lhs < *rhs;
                      } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPath* RicMswBranch::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::startMD() const
{
    return m_initialMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::startTVD() const
{
    return m_initialTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::endMD() const
{
    if ( !m_segments.empty() )
    {
        return m_segments.back()->endMD();
    }
    return m_initialMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::endTVD() const
{
    if ( !m_segments.empty() )
    {
        return m_segments.back()->endTVD();
    }
    return m_initialTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicMswBranch::branchNumber() const
{
    return m_branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::setBranchNumber( int branchNumber )
{
    m_branchNumber = branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RicMswSegment*> RicMswBranch::segments() const
{
    std::vector<const RicMswSegment*> allSegments;
    for ( const auto& segment : m_segments )
    {
        allSegments.push_back( segment.get() );
    }
    return allSegments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicMswSegment*> RicMswBranch::segments()
{
    std::vector<RicMswSegment*> allSegments;
    for ( auto& segment : m_segments )
    {
        allSegments.push_back( segment.get() );
    }
    return allSegments;
}

//--------------------------------------------------------------------------------------------------
/// TODO: Marked as obsolete, delete if lowerMD variant works as expected
//--------------------------------------------------------------------------------------------------
RicMswSegment* RicMswBranch::findClosestSegmentByMidpoint_obsolete( double measuredDepthLocation )
{
    if ( measuredDepthLocation < startMD() )
    {
        return segmentCount() > 0 ? segments().front() : nullptr;
    }

    if ( measuredDepthLocation > endMD() )
    {
        return segmentCount() > 0 ? segments().back() : nullptr;
    }

    RicMswSegment* closestSegment   = nullptr;
    double         smallestDistance = std::numeric_limits<double>::infinity();

    for ( auto seg : segments() )
    {
        // WELSEGS is reported as the midpoint of the segment
        double midpointMD = 0.5 * ( seg->startMD() + seg->endMD() );

        double candidateDistance = std::abs( midpointMD - measuredDepthLocation );
        if ( candidateDistance < smallestDistance )
        {
            closestSegment   = seg;
            smallestDistance = candidateDistance;
        }
    }

    return closestSegment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswSegment* RicMswBranch::findClosestSegmentWithLowerMD( double measuredDepthLocation )
{
    if ( measuredDepthLocation < startMD() )
    {
        return segmentCount() > 0 ? segments().front() : nullptr;
    }

    if ( measuredDepthLocation > endMD() )
    {
        return segmentCount() > 0 ? segments().back() : nullptr;
    }

    RicMswSegment* closestSegment   = nullptr;
    double         smallestDistance = std::numeric_limits<double>::infinity();

    for ( auto seg : segments() )
    {
        // WELSEGS is reported as the midpoint of the segment
        double midpointMD = 0.5 * ( seg->startMD() + seg->endMD() );

        double candidateDistance = measuredDepthLocation - midpointMD;
        if ( candidateDistance > 0.0 && candidateDistance < smallestDistance )
        {
            closestSegment   = seg;
            smallestDistance = candidateDistance;
        }
    }

    return closestSegment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswBranch::segmentCount() const
{
    return m_segments.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RicMswBranch*> RicMswBranch::branches() const
{
    std::vector<const RicMswBranch*> branches;
    for ( const auto& branch : m_branches )
    {
        branches.push_back( branch.get() );
    }
    return branches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicMswBranch*> RicMswBranch::branches()
{
    std::vector<RicMswBranch*> branches;
    for ( auto& branch : m_branches )
    {
        branches.push_back( branch.get() );
    }
    return branches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::addChildBranch( std::unique_ptr<RicMswBranch> branch )
{
    m_branches.push_back( std::move( branch ) );
}
