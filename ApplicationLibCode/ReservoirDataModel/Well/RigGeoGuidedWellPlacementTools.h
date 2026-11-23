/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cvfArray.h"
#include "cvfVector2.h"
#include "cvfVector3.h"

#include <QString>

#include <expected>
#include <functional>
#include <optional>
#include <vector>

class RigEclipseCaseData;
struct WellPathCellIntersectionInfo;

//==================================================================================================
/// Geometry helpers for geo-guided well placement: find where a vertical ray through a point passes
/// through the cells of a cell filter, and place the point at a fraction of that zone.
//==================================================================================================
class RigGeoGuidedWellPlacementTools
{
public:
    // Elevations (z, positive up) of the top and bottom of a contiguous zone along a vertical ray
    struct ZInterval
    {
        double top;
        double bottom;
    };

    // Merge consecutive included intersections along a downward ray into zones. Intersections must be
    // ordered by measured depth along the ray.
    static std::vector<ZInterval> filterIntervals( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                   const std::function<bool( size_t )>&             isCellIncluded,
                                                   double                                           gapTolerance = 0.1 );

    // The zone containing z, or the zone closest to z if no zone contains it
    static std::optional<ZInterval> closestInterval( const std::vector<ZInterval>& intervals, double z );

    // Fraction 0 is the bottom of the zone, 1 is the top
    static double zAtFraction( const ZInterval& interval, double fraction );

    // Intersections of a vertical ray through the full height of the main grid at xy, ordered top to bottom
    static std::vector<WellPathCellIntersectionInfo> verticalRayIntersections( const RigEclipseCaseData* caseData, const cvf::Vec2d& xy );

    // Adjusted elevation of the point, placed at the fraction of the zone defined by the reservoir cell visibility
    static std::expected<double, QString> computeAdjustedZ( const RigEclipseCaseData* caseData,
                                                            const cvf::UByteArray&    reservoirCellVisibility,
                                                            const cvf::Vec3d&         point,
                                                            double                    fraction );
};
