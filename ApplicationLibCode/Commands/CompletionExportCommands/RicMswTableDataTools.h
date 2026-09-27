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
/// MD range where the spacing between consecutive segment nodes is controlled. At most one of the
/// lengths is set. A rule applies to the segments whose node lies inside [startMD, endMD].
//--------------------------------------------------------------------------------------------------
struct SegmentationInterval
{
    double                startMD;
    double                endMD;
    std::optional<double> minLength;
    std::optional<double> maxLength;
};

std::vector<std::pair<double, double>> createSubSegmentMDPairs( double                                        startMD,
                                                                double                                        endMD,
                                                                double                                        maxSegmentLength,
                                                                const std::vector<std::pair<double, double>>& customSegmentIntervals = {} );

std::vector<SegmentationInterval> segmentationIntervals( const RimSegmentCollection*     segmentCollection,
                                                         const std::optional<QDateTime>& exportDate );

//--------------------------------------------------------------------------------------------------
/// Place the segment nodes of a branch. Candidate nodes are the cell (piece) centres in MD order.
/// Inside a min length interval, candidates closer than the min length to the previous node are
/// dropped. Inside a max length interval, evenly spaced nodes are inserted until no spacing exceeds
/// the max length. The spacing of the first node is measured from the outlet node.
//--------------------------------------------------------------------------------------------------
std::vector<double>
    placeSegmentNodes( double outletMD, const std::vector<double>& candidateNodes, const std::vector<SegmentationInterval>& intervals );

//--------------------------------------------------------------------------------------------------
/// Index of the nearest node upstream of md, i.e. the node with the largest MD not greater than md.
/// If md is upstream of all nodes, the most upstream node is used. Returns std::nullopt if there are
/// no nodes.
//--------------------------------------------------------------------------------------------------
std::optional<size_t> upstreamNodeIndex( const std::vector<double>& nodes, double md );

double tvdFromMeasuredDepth( const RimWellPath* wellPath, double measuredDepth );

inline constexpr double valveSegmentLength = 0.1;

} // namespace RicMswTableDataTools
