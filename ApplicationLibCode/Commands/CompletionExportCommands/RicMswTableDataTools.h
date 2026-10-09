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

#pragma once

#include <QDateTime>

#include <limits>
#include <optional>
#include <utility>
#include <vector>

class RimWellPath;
class RimSegmentCollection;

//--------------------------------------------------------------------------------------------------
/// Helper functions shared by the MSW table data export code paths.
//--------------------------------------------------------------------------------------------------
namespace RicMswTableDataTools
{

//--------------------------------------------------------------------------------------------------
/// MD range where the spacing between consecutive segment nodes is controlled. A fixed length cannot
/// be combined with min/max length, min and max length can be combined. A rule applies to the
/// segments whose node lies inside [startMD, endMD].
//--------------------------------------------------------------------------------------------------
struct SegmentationInterval
{
    double                startMD;
    double                endMD;
    std::optional<double> minLength;
    std::optional<double> maxLength;
    std::optional<double> fixedLength;
};

//--------------------------------------------------------------------------------------------------
/// Segmentation rules of a segment collection
//--------------------------------------------------------------------------------------------------
struct SegmentationSettings
{
    std::vector<SegmentationInterval> intervals;
    bool                              singleSegmentBeforeFirstPerforation = false;
    bool                              singleSegmentAfterLastPerforation   = false;
};

std::vector<std::pair<double, double>> createSubSegmentMDPairs( double                                        startMD,
                                                                double                                        endMD,
                                                                double                                        maxSegmentLength,
                                                                const std::vector<std::pair<double, double>>& customSegmentIntervals = {} );

std::vector<SegmentationInterval> segmentationIntervals( const RimSegmentCollection*     segmentCollection,
                                                         const std::optional<QDateTime>& exportDate );

SegmentationSettings segmentationSettings( const RimSegmentCollection* segmentCollection, const std::optional<QDateTime>& exportDate );

//--------------------------------------------------------------------------------------------------
/// Place the segment nodes of a branch. Candidate nodes are the cell (piece) centres in MD order.
/// Inside a fixed length interval, the candidates are replaced by nodes at the centre of segments of
/// the fixed length, starting at the interval start and limited to [outletMD, lastMD]. Inside a min
/// length interval, candidates closer than the min length to the previous node are dropped. Inside a
/// max length interval, evenly spaced nodes are inserted until no spacing exceeds the max length.
/// The spacing of the first node is measured from the outlet node.
//--------------------------------------------------------------------------------------------------
std::vector<double> placeSegmentNodes( double                                   outletMD,
                                       const std::vector<double>&               candidateNodes,
                                       const std::vector<SegmentationInterval>& intervals,
                                       double                                   lastMD = std::numeric_limits<double>::infinity() );

//--------------------------------------------------------------------------------------------------
/// Replace the nodes in [regionStartMD, firstPerforationMD) and/or (lastPerforationMD, regionEndMD]
/// by a single node at the centre of the region. Regions with a single node are unchanged.
//--------------------------------------------------------------------------------------------------
std::vector<double> collapseNodesOutsidePerforations( const std::vector<double>& nodes,
                                                      double                     regionStartMD,
                                                      double                     regionEndMD,
                                                      double                     firstPerforationMD,
                                                      double                     lastPerforationMD,
                                                      bool                       singleBefore,
                                                      bool                       singleAfter );

//--------------------------------------------------------------------------------------------------
/// Index of the nearest node upstream of md, i.e. the node with the largest MD not greater than md.
/// If md is upstream of all nodes, the most upstream node is used. Returns std::nullopt if there are
/// no nodes.
//--------------------------------------------------------------------------------------------------
std::optional<size_t> upstreamNodeIndex( const std::vector<double>& nodes, double md );

//--------------------------------------------------------------------------------------------------
/// Index of the node nearest to md, the first node wins ties. Mirrors the COMPSEGS connection to
/// segment assignment in OPM (Compsegs.cpp). Returns std::nullopt if there are no nodes.
//--------------------------------------------------------------------------------------------------
std::optional<size_t> nearestNodeIndex( const std::vector<double>& nodes, double md );

double tvdFromMeasuredDepth( const RimWellPath* wellPath, double measuredDepth );

inline constexpr double valveSegmentLength = 0.1;

} // namespace RicMswTableDataTools
