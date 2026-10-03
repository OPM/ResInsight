/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicMswTableDataTools.h"

#include "Well/RigWellPath.h"

#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"
#include "RimWellPath.h"

#include "cafAssert.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

//--------------------------------------------------------------------------------------------------
/// Custom intervals define exact segment boundaries where specified
/// Areas without custom intervals use max segment length subdivision (if maxSegmentLength > 0)
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>>
    RicMswTableDataTools::createSubSegmentMDPairs( double                                        startMD,
                                                   double                                        endMD,
                                                   double                                        maxSegmentLength,
                                                   const std::vector<std::pair<double, double>>& customSegmentIntervals )
{
    std::vector<std::pair<double, double>> subSegmentMDPairs;

    // If no custom intervals, use original logic with maxSegmentLength subdivision
    if ( customSegmentIntervals.empty() || maxSegmentLength <= 0.0 )
    {
        int    subSegmentCount  = maxSegmentLength > 0.0 ? (int)( std::trunc( ( endMD - startMD ) / maxSegmentLength ) + 1 ) : 1;
        double subSegmentLength = ( endMD - startMD ) / subSegmentCount;

        double subStartMD = startMD;
        double subEndMD   = startMD + subSegmentLength;
        for ( int i = 0; i < subSegmentCount; ++i )
        {
            subSegmentMDPairs.push_back( std::make_pair( subStartMD, subEndMD ) );
            subStartMD += subSegmentLength;
            subEndMD = std::min( subEndMD + subSegmentLength, endMD );
        }
        return subSegmentMDPairs;
    }

    // Combine custom intervals with maxSegmentLength subdivision
    // Collect all boundaries (start, end, custom interval boundaries) and sort them
    std::set<double> boundaries;
    boundaries.insert( startMD );
    boundaries.insert( endMD );

    // Add custom interval boundaries that overlap with [startMD, endMD]
    for ( const auto& [customStart, customEnd] : customSegmentIntervals )
    {
        // Check if custom interval overlaps with [startMD, endMD]
        if ( customEnd > startMD && customStart < endMD )
        {
            // Clip custom interval to [startMD, endMD] range
            double clippedStart = std::max( customStart, startMD );
            double clippedEnd   = std::min( customEnd, endMD );

            if ( clippedStart < clippedEnd )
            {
                boundaries.insert( clippedStart );
                boundaries.insert( clippedEnd );
            }
        }
    }

    // Convert boundaries to sorted vector
    std::vector<double> sortedBoundaries( boundaries.begin(), boundaries.end() );

    // For each gap between boundaries, either:
    // - Use exact boundary if it's from a custom interval
    // - Subdivide using maxSegmentLength if it's a gap
    for ( size_t i = 0; i + 1 < sortedBoundaries.size(); ++i )
    {
        double gapStart = sortedBoundaries[i];
        double gapEnd   = sortedBoundaries[i + 1];

        // Check if this gap is covered by a custom interval
        bool coveredByCustomInterval = false;
        for ( const auto& [customStart, customEnd] : customSegmentIntervals )
        {
            double clippedStart = std::max( customStart, startMD );
            double clippedEnd   = std::min( customEnd, endMD );

            // If the gap is fully within a custom interval, use exact boundaries
            if ( gapStart >= clippedStart && gapEnd <= clippedEnd && std::abs( gapStart - clippedStart ) < 1e-6 &&
                 std::abs( gapEnd - clippedEnd ) < 1e-6 )
            {
                coveredByCustomInterval = true;
                subSegmentMDPairs.push_back( std::make_pair( gapStart, gapEnd ) );
                break;
            }
        }

        // If not covered by custom interval, subdivide using maxSegmentLength
        if ( !coveredByCustomInterval && maxSegmentLength > 0.0 )
        {
            double gapLength = gapEnd - gapStart;
            int    subCount  = (int)( std::trunc( gapLength / maxSegmentLength ) + 1 );
            double subLength = gapLength / subCount;

            double subStart = gapStart;
            for ( int j = 0; j < subCount; ++j )
            {
                double subEnd = ( j == subCount - 1 ) ? gapEnd : subStart + subLength;
                subSegmentMDPairs.push_back( std::make_pair( subStart, subEnd ) );
                subStart = subEnd;
            }
        }
        else if ( !coveredByCustomInterval )
        {
            // No maxSegmentLength, use gap as-is
            subSegmentMDPairs.push_back( std::make_pair( gapStart, gapEnd ) );
        }
    }

    return subSegmentMDPairs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicMswTableDataTools::SegmentationInterval>
    RicMswTableDataTools::segmentationIntervals( const RimSegmentCollection* segmentCollection, const std::optional<QDateTime>& exportDate )
{
    std::vector<SegmentationInterval> result;
    if ( !segmentCollection ) return result;

    for ( const auto* interval : segmentCollection->segmentationIntervals( exportDate ) )
    {
        result.push_back( { interval->startMD(), interval->endMD(), interval->minSegmentLength(), interval->maxSegmentLength() } );
    }
    return result;
}

namespace
{
const RicMswTableDataTools::SegmentationInterval* intervalAtMD( const std::vector<RicMswTableDataTools::SegmentationInterval>& intervals,
                                                                double                                                         md )
{
    for ( const auto& interval : intervals )
    {
        if ( md >= interval.startMD && md <= interval.endMD ) return &interval;
    }
    return nullptr;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RicMswTableDataTools::placeSegmentNodes( double                                   outletMD,
                                                             const std::vector<double>&               candidateNodes,
                                                             const std::vector<SegmentationInterval>& intervals )
{
    if ( intervals.empty() ) return candidateNodes;

    const double tolerance = 1.0e-6;

    // Min length: greedily keep candidates at least the min length from the previous kept node
    std::vector<double> keptNodes;
    double              previousMD = outletMD;
    for ( size_t i = 0; i < candidateNodes.size(); )
    {
        const auto* interval = intervalAtMD( intervals, candidateNodes[i] );
        if ( !interval || !interval->minLength.has_value() )
        {
            keptNodes.push_back( candidateNodes[i] );
            previousMD = candidateNodes[i++];
            continue;
        }

        const double minLength = *interval->minLength;
        bool         anyKept   = false;
        size_t       j         = i;
        for ( ; j < candidateNodes.size() && intervalAtMD( intervals, candidateNodes[j] ) == interval; ++j )
        {
            if ( candidateNodes[j] - previousMD >= minLength - tolerance )
            {
                keptNodes.push_back( candidateNodes[j] );
                previousMD = candidateNodes[j];
                anyKept    = true;
            }
        }

        // Keep one node for an interval shorter than the min length, the last one gives the longest segment
        if ( !anyKept )
        {
            keptNodes.push_back( candidateNodes[j - 1] );
            previousMD = candidateNodes[j - 1];
        }
        i = j;
    }

    // Max length: insert evenly spaced nodes where the spacing to the previous node is too long
    std::vector<double> nodes;
    previousMD = outletMD;
    for ( double nodeMD : keptNodes )
    {
        const auto* interval = intervalAtMD( intervals, nodeMD );
        if ( interval && interval->maxLength.has_value() )
        {
            const double maxLength = *interval->maxLength;
            const double gap       = nodeMD - previousMD;
            if ( gap > maxLength + tolerance )
            {
                const int    segmentCount = static_cast<int>( std::ceil( ( gap - tolerance ) / maxLength ) );
                const double spacing      = gap / segmentCount;
                for ( int k = 1; k < segmentCount; ++k )
                {
                    nodes.push_back( previousMD + k * spacing );
                }
            }
        }
        nodes.push_back( nodeMD );
        previousMD = nodeMD;
    }

    return nodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<size_t> RicMswTableDataTools::upstreamNodeIndex( const std::vector<double>& nodes, double md )
{
    if ( nodes.empty() ) return std::nullopt;

    std::optional<size_t> upstream;
    double                minDistance = std::numeric_limits<double>::infinity();
    for ( size_t i = 0; i < nodes.size(); ++i )
    {
        const double distance = md - nodes[i];
        if ( distance >= 0.0 && distance < minDistance )
        {
            minDistance = distance;
            upstream    = i;
        }
    }

    // md is upstream of all nodes, use the most upstream node
    if ( !upstream ) upstream = std::ranges::min_element( nodes ) - nodes.begin();
    return upstream;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<size_t> RicMswTableDataTools::nearestNodeIndex( const std::vector<double>& nodes, double md )
{
    std::optional<size_t> nearest;
    double                minDistance = std::numeric_limits<double>::infinity();
    for ( size_t i = 0; i < nodes.size(); ++i )
    {
        const double distance = std::abs( md - nodes[i] );
        if ( distance < minDistance )
        {
            minDistance = distance;
            nearest     = i;
        }
    }
    return nearest;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswTableDataTools::tvdFromMeasuredDepth( const RimWellPath* wellPath, double measuredDepth )
{
    auto wellPathGeometry = wellPath->wellPathGeometry();
    CAF_ASSERT( wellPathGeometry );

    double tvdValue = -wellPathGeometry->interpolatedPointAlongWellPath( measuredDepth ).z();

    return tvdValue;
}
