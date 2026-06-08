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

#include "RigFaultDistanceCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigFault.h"
#include "RigMainGrid.h"

#include <nanoflann.hpp>

#include <cmath>
#include <limits>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Adaptor exposing the collected fault-face-center points to nanoflann without copying them.
/// cvf::Vec3d stores its three doubles contiguously, so ptr()[dim] is a direct coordinate lookup.
//--------------------------------------------------------------------------------------------------
struct FaceCenterCloud
{
    const std::vector<cvf::Vec3d>& points;

    inline size_t kdtree_get_point_count() const { return points.size(); }
    inline double kdtree_get_pt( size_t idx, size_t dim ) const { return points[idx].ptr()[dim]; }

    template <class BBOX>
    bool kdtree_get_bbox( BBOX& ) const
    {
        return false;
    }
};

using FaceCenterKdTree = nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<double, FaceCenterCloud>, FaceCenterCloud, 3>;
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultDistanceCalculator::computeFaultDistances( const RigMainGrid*                  mainGrid,
                                                        const RigActiveCellInfo*            activeCellInfo,
                                                        const std::vector<const RigFault*>& faultsToInclude,
                                                        std::vector<double>&                resultValues )
{
    if ( !mainGrid || !activeCellInfo ) return;

    const auto activeCells = activeCellInfo->activeReservoirCellIndices();
    if ( activeCells.empty() ) return;

    if ( resultValues.size() < activeCells.size() )
    {
        resultValues.resize( activeCells.size(), std::numeric_limits<double>::infinity() );
    }

    // Collect fault face centers from the requested subset of faults.
    std::vector<cvf::Vec3d> faultFaceCenters;
    for ( const RigFault* fault : faultsToInclude )
    {
        if ( !fault ) continue;
        for ( const RigFault::FaultFace& faultFace : fault->faultFaces() )
        {
            if ( faultFace.m_nativeReservoirCellIndex >= mainGrid->cellCount() ) continue;
            const RigCell& cell = mainGrid->cell( faultFace.m_nativeReservoirCellIndex );
            if ( cell.isInvalid() ) continue;
            faultFaceCenters.push_back( cell.faceCenter( faultFace.m_nativeFace ) );
        }
    }

    if ( faultFaceCenters.empty() ) return;

    // Build a KD-tree over the fault face centers and query the nearest one for every active cell.
    // This is an exact nearest-point search in O(log N) per cell, replacing the previous expanding
    // bounding-box heuristic.
    FaceCenterCloud  cloud{ faultFaceCenters };
    FaceCenterKdTree kdTree( 3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams( 10 /* max leaf size */ ) );
    kdTree.buildIndex();

#pragma omp parallel for
    for ( int activeIndex = 0; activeIndex < static_cast<int>( activeCells.size() ); activeIndex++ )
    {
        auto cellIdx = activeCells[activeIndex];
        if ( cellIdx.value() == cvf::UNDEFINED_SIZE_T ) continue;

        const RigCell& cell = mainGrid->cell( cellIdx.value() );
        if ( cell.isInvalid() ) continue;

        const cvf::Vec3d cellCenter   = cell.center();
        const double     queryPoint[] = { cellCenter.x(), cellCenter.y(), cellCenter.z() };

        size_t                          nearestIndex           = 0;
        double                          nearestDistanceSquared = std::numeric_limits<double>::infinity();
        nanoflann::KNNResultSet<double> resultSet( 1 );
        resultSet.init( &nearestIndex, &nearestDistanceSquared );
        kdTree.findNeighbors( resultSet, queryPoint );

        resultValues[activeIndex] = std::sqrt( nearestDistanceSquared );
    }
}
