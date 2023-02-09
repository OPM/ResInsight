/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RifOpmGridTools.h"

#include "RiaWeightedMeanCalculator.h"
#include "RifReaderEclipseOutput.h"
#include "RigMainGrid.h"

#include "cvfGeometryTools.h"

#include "opm/io/eclipse/EGrid.hpp"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmGridTools::importCoordinatesForRadialGrid( const std::string& gridFilePath, RigMainGrid* riMainGrid )
{
    CAF_ASSERT( riMainGrid );

    try
    {
        Opm::EclIO::EGrid opmMainGrid( gridFilePath );

        if ( opmMainGrid.is_radial() )
        {
            transferCoordinates( opmMainGrid, opmMainGrid, riMainGrid, riMainGrid );
        }

        auto lgrNames = opmMainGrid.list_of_lgrs();
        for ( const auto& lgrName : lgrNames )
        {
            Opm::EclIO::EGrid opmLgrGrid( gridFilePath, lgrName );

            if ( opmLgrGrid.is_radial() )
            {
                for ( size_t i = 0; i < riMainGrid->gridCount(); i++ )
                {
                    auto riLgrGrid = riMainGrid->gridByIndex( i );
                    if ( riLgrGrid->gridName() == lgrName )
                    {
                        transferCoordinates( opmMainGrid, opmLgrGrid, riMainGrid, riLgrGrid );
                    }
                }
            }
        }
    }
    catch ( ... )
    {
    }
}

//--------------------------------------------------------------------------------------------------
//
// A radial grid is defined by a center point and a set of cylindrical coordinates. The coordinates at the
// outer edge of the grid are defined as a circle, and nodes do not match the geometry of the host cell.
// Adjust the coordinates for these cells to make sure the grid is continuous
//
// Cylindrical coordinates from file gives grid cell with a radial grid in center
//  -------------
//  |  /---- \  |
//  | /   -   \ |
//  ||   | |   ||
//  | \   -   / |
//  |  \-----/  |
//  -------------

//
// Adjusted coordinates to match the grid geometry of host cell. The coordinates for the inner nodes are
// unchanged.
//  -------------
//  ||---------||
//  ||    -    ||
//  ||   | |   ||
//  ||    -    ||
//  ||---------||
//  -------------

// 1. If the node is at the outer edge of the radial grid, find the host cell
// 2. Find the closest point on the pillars of the host cell
// 3. Find the closes point on this pillar, and use this point as the adjusted coordinate for the node
//
//--------------------------------------------------------------------------------------------------
void RifOpmGridTools::transferCoordinates( Opm::EclIO::EGrid& opmMainGrid,
                                           Opm::EclIO::EGrid& opmGrid,
                                           RigMainGrid*       riMainGrid,
                                           RigGridBase*       riGrid )
{
    size_t cellCount = opmGrid.totalNumberOfCells();
    if ( cellCount != riGrid->cellCount() ) return;

    // Read out the corner coordinates from the EGRID file using radial coordinates.
    // Prefix OPM structures with _opm_and ResInsight structures with _ri_

    // Compute the center of the LGR radial grid cells for each K layer
    std::map<int, std::pair<double, double>> radialGridCenterTopLayerOpm =
        computeXyCenterForTopOfCells( opmMainGrid, opmGrid, riGrid );

    std::array<double, 8> opmX{};
    std::array<double, 8> opmY{};
    std::array<double, 8> opmZ{};

    const auto    hostCellGlobalIndices = opmGrid.hostCellsGlobalIndex();
    const size_t* cellMappingECLRi      = RifReaderEclipseOutput::eclipseCellIndexMapping();
    const auto    gridDimension         = opmGrid.dimension();
    auto&         riNodes               = riMainGrid->nodes();

    for ( size_t opmCellIndex = 0; opmCellIndex < cellCount; opmCellIndex++ )
    {
        opmGrid.getCellCorners( opmCellIndex, opmX, opmY, opmZ );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto riNodeStartIndex = riGrid->reservoirCellIndex( opmCellIndex ) * 8;
        auto ijkCell          = opmGrid.ijk_from_global_index( opmCellIndex );

        double xCenterCoordOpm = 0.0;
        double yCenterCoordOpm = 0.0;

        if ( radialGridCenterTopLayerOpm.count( ijkCell[2] ) > 0 )
        {
            const auto& [xCenter, yCenter] = radialGridCenterTopLayerOpm[ijkCell[2]];
            xCenterCoordOpm                = xCenter;
            yCenterCoordOpm                = yCenter;
        }

        for ( size_t opmNodeIndex = 0; opmNodeIndex < 8; opmNodeIndex++ )
        {
            size_t riNodeIndex = riNodeStartIndex + cellMappingECLRi[opmNodeIndex];

            // The radial grid is specified with (0,0) as center, add grid center to get correct global coordinates
            auto& riNode = riNodes[riNodeIndex];
            riNode.x()   = opmX[opmNodeIndex] + xCenterCoordOpm;
            riNode.y()   = opmY[opmNodeIndex] + yCenterCoordOpm;
            riNode.z()   = -opmZ[opmNodeIndex];

            // First grid dimension is radius, check if cell has are at the outer-most slice
            if ( !hostCellGlobalIndices.empty() && ( gridDimension[0] - 1 == ijkCell[0] ) )
            {
                std::array<double, 8> cellRadius{};
                std::array<double, 8> cellTheta{};
                std::array<double, 8> cellZ{};
                opmGrid.getRadialCellCorners( ijkCell, cellRadius, cellTheta, cellZ );

                double maxRadius = *std::max_element( cellRadius.begin(), cellRadius.end() );

                // Check if the radius is at the outer surface of the radial grid
                // Adjust the outer nodes to match the corner pillars of the host cell
                const double epsilon = 0.05;
                if ( fabs( maxRadius - cellRadius[opmNodeIndex] ) < epsilon * cellRadius[opmNodeIndex] )
                {
                    std::array<double, 8> hostCellX{};
                    std::array<double, 8> hostCellY{};
                    std::array<double, 8> hostCellZ{};

                    const auto hostCellIndex = hostCellGlobalIndices[opmCellIndex];
                    const auto hostGridIJK   = opmMainGrid.ijk_from_global_index( hostCellIndex );
                    opmMainGrid.getCellCorners( hostGridIJK, hostCellX, hostCellY, hostCellZ );

                    double     closestPillarDistance = std::numeric_limits<double>::max();
                    cvf::Vec3d closestPillarCoord;

                    const auto cylinderCoordX = opmX[opmNodeIndex] + xCenterCoordOpm;
                    const auto cylinderCoordY = opmY[opmNodeIndex] + yCenterCoordOpm;
                    const auto cylinderCoordZ = opmZ[opmNodeIndex];

                    const cvf::Vec3d coordinateOnCylinder = cvf::Vec3d( cylinderCoordX, cylinderCoordY, cylinderCoordZ );

                    for ( int pillarIndex = 0; pillarIndex < 4; pillarIndex++ )
                    {
                        const cvf::Vec3d p0( hostCellX[0 + pillarIndex],
                                             hostCellY[0 + pillarIndex],
                                             hostCellZ[0 + pillarIndex] );
                        const cvf::Vec3d p1( hostCellX[4 + pillarIndex],
                                             hostCellY[4 + pillarIndex],
                                             hostCellZ[4 + pillarIndex] );

                        const auto candidateCoord =
                            cvf::GeometryTools::projectPointOnLine( p0, p1, coordinateOnCylinder, nullptr );

                        double candidateDistance = candidateCoord.pointDistance( coordinateOnCylinder );
                        if ( candidateDistance < closestPillarDistance )
                        {
                            closestPillarDistance = candidateDistance;
                            closestPillarCoord    = candidateCoord;
                        }
                    }

                    riNode.x() = closestPillarCoord.x();
                    riNode.y() = closestPillarCoord.y();
                    riNode.z() = -closestPillarCoord.z();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, std::pair<double, double>> RifOpmGridTools::computeXyCenterForTopOfCells( Opm::EclIO::EGrid& opmMainGrid,
                                                                                        Opm::EclIO::EGrid& opmGrid,
                                                                                        RigGridBase*       riGrid )
{
    if ( !riGrid || riGrid->isMainGrid() ) return {};

    size_t cellCount = opmGrid.totalNumberOfCells();
    if ( cellCount != riGrid->cellCount() ) return {};

    // Read out the corner coordinates from the EGRID file using radial coordinates.
    // Prefix OPM structures with _opm_and ResInsight structures with _ri_

    // Compute the center of the LGR radial grid cells for each K layer
    std::map<int, std::pair<double, double>> radialGridCenterTopLayerOpm;

    {
        std::array<double, 8> opmX{};
        std::array<double, 8> opmY{};
        std::array<double, 8> opmZ{};

        std::map<int, std::vector<std::pair<double, double>>> xyCenterPerLayer;

        auto hostCellGlobalIndices = opmGrid.hostCellsGlobalIndex();

        for ( size_t cIdx = 0; cIdx < cellCount; cIdx++ )
        {
            auto mainGridCellIndex = hostCellGlobalIndices[cIdx];
            opmMainGrid.getCellCorners( mainGridCellIndex, opmX, opmY, opmZ );

            auto ijkLocalGrid = opmGrid.ijk_from_global_index( cIdx );
            auto layer        = ijkLocalGrid[2];

            // Four corners for top
            for ( size_t i = 4; i < 8; i++ )
            {
                auto& xyCoords = xyCenterPerLayer[layer];
                xyCoords.emplace_back( opmX[i], opmY[i] );
            }
        }

        for ( const auto& [k, xyCoords] : xyCenterPerLayer )
        {
            RiaWeightedMeanCalculator<double> xCoord;
            RiaWeightedMeanCalculator<double> yCoord;

            for ( const auto& [x, y] : xyCoords )
            {
                xCoord.addValueAndWeight( x, 1.0 );
                yCoord.addValueAndWeight( y, 1.0 );
            }

            radialGridCenterTopLayerOpm[k] = { xCoord.weightedMean(), yCoord.weightedMean() };
        }
    }

    return radialGridCenterTopLayerOpm;
}
