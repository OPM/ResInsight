/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RigStatisticsContourMapProjection.h"

#include "RigContourMapCalculator.h"
#include "RigContourMapGrid.h"

#include "cafAssert.h"

#include <cmath>
#include <utility>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStatisticsContourMapProjection::RigStatisticsContourMapProjection( const RigContourMapGrid& contourMapGrid )
    : RigContourMapProjection( contourMapGrid )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStatisticsContourMapProjection::~RigStatisticsContourMapProjection()
{
}

std::vector<std::vector<std::pair<size_t, double>>>
    RigStatisticsContourMapProjection::generateGridMapping( RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                            const std::vector<double>&                     weights,
                                                            const std::set<int>&                           kLayers,
                                                            const std::vector<std::vector<cvf::Vec3d>>&    limitToPolygons )
{
    // The grid mapping is usually necessary to produce the data. For the statistics projection
    // the data is already available, so we can just ignore it.
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigStatisticsContourMapProjection::generateAndSaveResults( const std::vector<double>& result )
{
    m_aggregatedResults = result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigStatisticsContourMapProjection::findIntersectingCells( const cvf::BoundingBox& bbox ) const
{
    CAF_ASSERT( false );
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigStatisticsContourMapProjection::kLayer( size_t globalCellIdx ) const
{
    CAF_ASSERT( false );
    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigStatisticsContourMapProjection::kLayers() const
{
    CAF_ASSERT( false );
    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStatisticsContourMapProjection::calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const
{
    CAF_ASSERT( false );
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStatisticsContourMapProjection::calculateRayLengthInCell( size_t            globalCellIdx,
                                                                    const cvf::Vec3d& highestPoint,
                                                                    const cvf::Vec3d& lowestPoint ) const
{
    CAF_ASSERT( false );
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStatisticsContourMapProjection::getParameterWeightForCell( size_t cellResultIdx, const std::vector<double>& cellWeights ) const
{
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<bool> RigStatisticsContourMapProjection::getMapCellVisibility( int viewStepIndex,
                                                                           RigContourMapCalculator::ResultAggregationType resultAggregation )

{
    return std::vector<bool>( numberOfCells(), true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>> RigStatisticsContourMapProjection::cellsAtIJ( unsigned int i, unsigned int j ) const
{
    size_t cellIndex = m_contourMapGrid.cellIndexFromIJ( i, j );
    if ( cellIndex < m_aggregatedResults.size() && !std::isinf( m_aggregatedResults[cellIndex] ) &&
         !std::isnan( m_aggregatedResults[cellIndex] ) )
    {
        return { std::make_pair( cellIndex, m_aggregatedResults[cellIndex] ) };
    }

    return std::vector<std::pair<size_t, double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigStatisticsContourMapProjection::isCellActive( size_t globalCellIdx ) const
{
    // For statistics projections, all cells are considered active
    return true;
}
