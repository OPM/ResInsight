/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C)  Statoil ASA
//  Copyright (C)  Ceetron Solutions AS
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
struct RigWellResultPoint;
struct RigWellResultBranch;
class RigWellResultFrame;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigSimulationWellCenterLineCalculator
{
public:
    static std::vector<SimulationWellCellBranch> calculateMswWellPipeGeometry( RimSimWellInView* rimWell );
    static std::vector<SimulationWellCellBranch> calculateWellPipeStaticCenterline( RimSimWellInView* rimWell );

    static std::vector<SimulationWellCellBranch> calculateWellPipeCenterlineForTimeStep( const RigEclipseCaseData* eclipseCaseData,
                                                                                         const RigSimWellData*     simWellData,
                                                                                         int                       timeStepIndex,
                                                                                         bool                      isAutoDetectBranches,
                                                                                         bool                      useAllCellCenters );

    static std::pair<std::vector<std::vector<cvf::Vec3d>>, std::vector<std::vector<RigWellResultPoint>>>
        extractBranchData( const std::vector<SimulationWellCellBranch> simulationBranch );

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
    static void calculateWellPipeStaticCenterline( RimSimWellInView*                             rimWell,
                                                   std::vector<std::vector<cvf::Vec3d>>&         pipeBranchesCLCoords,
                                                   std::vector<std::vector<RigWellResultPoint>>& pipeBranchesCellIds );

    static void calculateWellPipeCenterlineForTimeStep( const RigEclipseCaseData*                     eclipseCaseData,
                                                        const RigSimWellData*                         simWellData,
                                                        int                                           timeStepIndex,
                                                        bool                                          isAutoDetectBranches,
                                                        bool                                          useAllCellCenters,
                                                        std::vector<std::vector<cvf::Vec3d>>&         pipeBranchesCLCoords,
                                                        std::vector<std::vector<RigWellResultPoint>>& pipeBranchesCellIds );

    static std::vector<SimulationWellCellBranch> calculateMswWellPipeGeometryForTimeStep( const RigEclipseCaseData* eclipseCaseData,
                                                                                          const RigSimWellData*     simWellData,
                                                                                          int                       timeStepIndex );

    static std::vector<WellBranch> buildAccumulatedWellBranches( const std::vector<RigWellResultBranch>& resBranches );

    static bool hasAnyValidDataCells( const RigWellResultBranch& branch );
    static void finishPipeCenterLine( std::vector<std::vector<cvf::Vec3d>>& pipeBranchesCLCoords, const cvf::Vec3d& lastCellCenter );

    static RigWellResultFrame splitIntoBranches( const RigWellResultFrame& wellResultFrame, const RigEclipseCaseData* eclipseCaseData );
    static void               addCellCenterPoints( const RigEclipseCaseData*                     eclipseCaseData,
                                                   std::vector<std::vector<cvf::Vec3d>>&         pipeBranchesCLCoords,
                                                   std::vector<std::vector<RigWellResultPoint>>& pipeBranchesCellIds );
};
