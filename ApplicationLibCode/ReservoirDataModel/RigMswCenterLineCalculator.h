/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RigWellResultPoint.h"

#include "cvfVector3.h"

#include <map>
#include <vector>

class RigEclipseCaseData;
class RimSimWellInView;
class RigSimWellData;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigMswCenterLineCalculator
{
public:
    static std::vector<SimulationWellCellBranch> calculateMswWellPipeGeometry( RimSimWellInView* rimWell );

private:
    struct OutputSegment
    {
        int outputSegmentId       = -1;
        int outputSegmentBranchId = -1;
    };

    struct WellBranch
    {
        int m_branchId = -1;

        std::map<std::pair<size_t, size_t>, OutputSegment>    m_gridCellsConnectedToSegments;
        std::map<int, std::vector<std::pair<size_t, size_t>>> m_segmentsWithGridCells;

        bool containsGridCell( const std::pair<size_t, size_t>& candidateGridAndCellIndex ) const
        {
            for ( const auto& [segmentId, gridAndCellIndex] : m_segmentsWithGridCells )
            {
                if ( std::find( gridAndCellIndex.begin(), gridAndCellIndex.end(), candidateGridAndCellIndex ) != gridAndCellIndex.end() )
                {
                    return true;
                }
            }

            return false;
        };
    };

private:
    static std::vector<SimulationWellCellBranch> calculateMswWellPipeGeometryForTimeStep( const RigEclipseCaseData* eclipseCaseData,
                                                                                          const RigSimWellData*     simWellData,
                                                                                          int                       timeStepIndex );

    static SimulationWellCellBranch addCoordsAtCellFaceIntersectionsAndCreateBranch( const std::vector<cvf::Vec3d>          branchCoords,
                                                                                     const std::vector<RigWellResultPoint>& resultPoints,
                                                                                     const RigEclipseCaseData* eclipseCaseData );

    static std::vector<WellBranch> mergeShortBranchesIntoLongBranches( const std::vector<RigWellResultBranch>& resBranches );
};
