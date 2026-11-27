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

#include "RigGeoGuidedWellPlacementTools.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellLogExtractor.h"
#include "RigWellPathIntersectionTools.h"

#include "cvfBoundingBox.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigGeoGuidedWellPlacementTools::ZInterval>
    RigGeoGuidedWellPlacementTools::filterIntervals( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                     const std::function<bool( size_t )>&             isCellIncluded,
                                                     double                                           gapTolerance )
{
    std::vector<ZInterval> intervals;

    std::optional<double> lastEndMd;
    for ( const auto& intersection : intersections )
    {
        if ( !isCellIncluded( intersection.globCellIndex ) ) continue;

        const double top    = std::max( intersection.startPoint.z(), intersection.endPoint.z() );
        const double bottom = std::min( intersection.startPoint.z(), intersection.endPoint.z() );

        const bool isContinuation = !intervals.empty() && lastEndMd && intersection.startMD - *lastEndMd <= gapTolerance;
        if ( isContinuation )
        {
            intervals.back().top    = std::max( intervals.back().top, top );
            intervals.back().bottom = std::min( intervals.back().bottom, bottom );
        }
        else
        {
            intervals.push_back( { top, bottom } );
        }
        lastEndMd = intersection.endMD;
    }

    return intervals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigGeoGuidedWellPlacementTools::ZInterval>
    RigGeoGuidedWellPlacementTools::closestInterval( const std::vector<ZInterval>& intervals, double z )
{
    std::optional<ZInterval> closest;
    double                   minDistance = std::numeric_limits<double>::max();

    for ( const auto& interval : intervals )
    {
        double distance = 0.0;
        if ( z > interval.top )
            distance = z - interval.top;
        else if ( z < interval.bottom )
            distance = interval.bottom - z;

        if ( distance < minDistance )
        {
            minDistance = distance;
            closest     = interval;
        }
    }

    return closest;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoGuidedWellPlacementTools::zAtFraction( const ZInterval& interval, double fraction )
{
    return interval.bottom + fraction * ( interval.top - interval.bottom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> RigGeoGuidedWellPlacementTools::verticalRayIntersections( const RigEclipseCaseData* caseData,
                                                                                                    const cvf::Vec2d&         xy )
{
    if ( !caseData || !caseData->mainGrid() ) return {};

    const cvf::BoundingBox bb = caseData->mainGrid()->boundingBox();
    if ( !bb.isValid() ) return {};

    const double margin = 1.0;
    const double zTop   = bb.max().z() + margin;
    const double zBase  = bb.min().z() - margin;

    const std::vector<cvf::Vec3d> coords = { { xy.x(), xy.y(), zTop }, { xy.x(), xy.y(), zBase } };
    const std::vector<double>     mds    = { 0.0, zTop - zBase };

    return RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( caseData, "Geo-guided ray", coords, mds );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<double, QString> RigGeoGuidedWellPlacementTools::computeAdjustedZ( const RigEclipseCaseData* caseData,
                                                                                 const cvf::UByteArray&    reservoirCellVisibility,
                                                                                 const cvf::Vec3d&         point,
                                                                                 double                    fraction )
{
    const auto intersections = verticalRayIntersections( caseData, { point.x(), point.y() } );

    auto isIncluded = [&reservoirCellVisibility]( size_t cellIndex )
    { return cellIndex < reservoirCellVisibility.size() && reservoirCellVisibility[cellIndex] != 0; };

    const auto intervals = filterIntervals( intersections, isIncluded );
    const auto interval  = closestInterval( intervals, point.z() );
    if ( !interval )
    {
        return std::unexpected( QString( "The vertical line through (%1, %2) does not intersect the cell filter" )
                                    .arg( point.x(), 0, 'f', 1 )
                                    .arg( point.y(), 0, 'f', 1 ) );
    }

    return zAtFraction( *interval, fraction );
}
