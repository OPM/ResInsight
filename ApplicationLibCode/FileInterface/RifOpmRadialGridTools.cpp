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

#include "RifOpmRadialGridTools.h"

#include "RiaAngleUtils.h"
#include "RiaLogging.h"
#include "RiaPreferencesSystem.h"
#include "RiaWeightedMeanCalculator.h"

#include "ExportCommands/RicExportLgrFeature.h"
#include "RicCreateTemporaryLgrFeature.h"

#include "RifReaderEclipseOutput.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "cafAssert.h"

#include "opm/io/eclipse/EGrid.hpp"

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmRadialGridTools::importCylindricalCoordinates( const std::string& gridFilePath, RigEclipseCaseData* caseData )
{
    auto riMainGrid = caseData->mainGrid();
    CAF_ASSERT( riMainGrid );

    try
    {
        bool isRadialGridPresent = false;

        {
            // Open the file and only check "GRIDHEAD" to be able to do an early return if no radial grids are present

            Opm::EclIO::EclFile gridFile( gridFilePath );
            auto                arrays = gridFile.getList();

            int index = 0;
            for ( const auto& [name, arrayType, arraySize] : arrays )
            {
                if ( name == "GRIDHEAD" )
                {
                    auto gridhead = gridFile.get<int>( index );
                    if ( gridhead.size() > 26 && gridhead[26] > 0 )
                    {
                        isRadialGridPresent = true;
                        break;
                    }
                }
                index++;
            }
        }

        if ( !isRadialGridPresent ) return false;

        bool lgrIsCreated = false;

        Opm::EclIO::EGrid opmMainGrid( gridFilePath );

        if ( opmMainGrid.is_radial() )
        {
            transferCylindricalCoords( opmMainGrid, opmMainGrid, riMainGrid, riMainGrid );

            auto minimumAngularCellCount = RiaPreferencesSystem::current()->minimumAngularCellCount();

            if ( opmMainGrid.is_radial() && opmMainGrid.dimension().at( 1 ) < minimumAngularCellCount )
            {
                RiaLogging::info( QString( "Radial grid with less than 4 cells in J direction detected, creating refinement : %1" )
                                      .arg( QString::fromStdString( gridFilePath ) ) );

                int radialRefinement = ( minimumAngularCellCount / opmMainGrid.dimension().at( 1 ) ) + 1;

                createAngularGridRefinement( caseData, radialRefinement );

                lgrIsCreated = true;
            }
        }

        return lgrIsCreated;
    }
    catch ( ... )
    {
        RiaLogging::warning(
            QString( "Failed to open grid case for import of radial coordinates : %1" ).arg( QString::fromStdString( gridFilePath ) ) );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmRadialGridTools::importCoordinatesForRadialGrid( const std::string& gridFilePath, RigMainGrid* riMainGrid )
{
    try
    {
        auto isRadialGrid = []( const std::string& gridFilePath )
        {
            // Open the file and only check "GRIDHEAD" to be able to do an early return if no radial grids are present

            Opm::EclIO::EclFile gridFile( gridFilePath );
            auto                arrays = gridFile.getList();

            int index = 0;
            for ( const auto& [name, arrayType, arraySize] : arrays )
            {
                if ( name == "GRIDHEAD" )
                {
                    auto gridhead = gridFile.get<int>( index );
                    if ( gridhead.size() > 26 && gridhead[26] > 0 )
                    {
                        return true;
                    }
                }
                index++;
            }

            return false;
        };

        if ( !isRadialGrid( gridFilePath ) ) return false;

        Opm::EclIO::EGrid opmMainGrid( gridFilePath );

        if ( opmMainGrid.is_radial() )
        {
            transferCoordinatesRadial( opmMainGrid, opmMainGrid, riMainGrid, riMainGrid );
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
                        transferCoordinatesRadial( opmMainGrid, opmLgrGrid, riMainGrid, riLgrGrid );
                    }
                }
            }
        }
    }
    catch ( ... )
    {
        RiaLogging::warning(
            QString( "Failed to open grid case for import of radial coordinates : %1" ).arg( QString::fromStdString( gridFilePath ) ) );
    }

    return true;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmRadialGridTools::transferCylindricalCoords( Opm::EclIO::EGrid& opmMainGrid,
                                                       Opm::EclIO::EGrid& opmGrid,
                                                       RigMainGrid*       riMainGrid,
                                                       RigGridBase*       riGrid )
{
    size_t cellCount = opmGrid.totalNumberOfCells();
    if ( cellCount != riGrid->cellCount() ) return;

    // Read out the corner coordinates from the EGRID file using cylindrical coordinates.
    // Prefix OPM structures with _opm_and ResInsight structures with _ri_

    std::array<double, 8> opmX{};
    std::array<double, 8> opmY{};
    std::array<double, 8> opmZ{};

    const auto    hostCellGlobalIndices = opmGrid.hostCellsGlobalIndex();
    const size_t* cellMappingECLRi      = RifReaderEclipseOutput::eclipseCellIndexMapping();
    auto&         riNodes               = riMainGrid->nodes();

    bool convertCylindricalCoords = false;

    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( cellCount ); opmCellIndex++ )
    {
        auto ijkCell = opmGrid.ijk_from_global_index( opmCellIndex );
        opmGrid.getCellCorners( ijkCell, opmX, opmY, opmZ, convertCylindricalCoords );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto riNodeStartIndex = riGrid->reservoirCellIndex( opmCellIndex ) * 8;

        for ( size_t opmNodeIndex = 0; opmNodeIndex < 8; opmNodeIndex++ )
        {
            size_t riNodeIndex = riNodeStartIndex + cellMappingECLRi[opmNodeIndex];

            // The radial grid is specified with (0,0) as center, add grid center to get correct global coordinates
            auto& riNode = riNodes[riNodeIndex];
            riNode.x()   = opmX[opmNodeIndex];
            riNode.y()   = opmY[opmNodeIndex];
            riNode.z()   = -opmZ[opmNodeIndex];
        }
    }
}

//--------------------------------------------------------------------------------------------------
// 1. If the node is at the outer edge of the radial grid, find the host cell
// 2. Find the closest point on the pillars of the host cell
// 3. Find the closest point on this pillar, and use this point as the adjusted coordinate for the node
//--------------------------------------------------------------------------------------------------
void RifOpmRadialGridTools::transferCoordinatesRadial( Opm::EclIO::EGrid& opmMainGrid,
                                                       Opm::EclIO::EGrid& opmGrid,
                                                       RigMainGrid*       riMainGrid,
                                                       RigGridBase*       riGrid )
{
    size_t cellCount = opmGrid.totalNumberOfCells();
    if ( cellCount != riGrid->cellCount() ) return;

    // Read out the corner coordinates from the EGRID file using radial coordinates.
    // Prefix OPM structures with _opm_and ResInsight structures with _ri_

    // Compute the center of the LGR radial grid cells for each K layer
    std::map<int, std::pair<double, double>> radialGridCenterTopLayerOpm = computeXyCenterForTopOfCells( opmMainGrid, opmGrid, riGrid );

    std::array<double, 8> opmX{};
    std::array<double, 8> opmY{};
    std::array<double, 8> opmZ{};

    const auto    hostCellGlobalIndices = opmGrid.hostCellsGlobalIndex();
    const size_t* cellMappingECLRi      = RifReaderEclipseOutput::eclipseCellIndexMapping();
    const auto    gridDimension         = opmGrid.dimension();
    auto&         riNodes               = riMainGrid->nodes();

    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( cellCount ); opmCellIndex++ )
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
                auto hostCellIndex = hostCellGlobalIndices[opmCellIndex];

                lockToHostPillars( riNode, opmMainGrid, opmGrid, ijkCell, hostCellIndex, opmCellIndex, opmNodeIndex, xCenterCoordOpm, yCenterCoordOpm );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
void RifOpmRadialGridTools::lockToHostPillars( cvf::Vec3d&         riNode,
                                               Opm::EclIO::EGrid&  opmMainGrid,
                                               Opm::EclIO::EGrid&  opmGrid,
                                               std::array<int, 3>& ijkCell,
                                               int                 hostCellIndex,
                                               int                 opmCellIndex,
                                               size_t              opmNodeIndex,
                                               double              xCenterCoordOpm,
                                               double              yCenterCoordOpm )
{
    std::array<double, 8> cellRadius{};
    std::array<double, 8> cellTheta{};
    std::array<double, 8> cellZ{};
    bool                  convertToRadialCoords = false;
    opmGrid.getCellCorners( ijkCell, cellRadius, cellTheta, cellZ, convertToRadialCoords );

    double maxRadius = *std::max_element( cellRadius.begin(), cellRadius.end() );

    // Check if the radius is at the outer surface of the radial grid
    // Adjust the outer nodes to match the corner pillars of the host cell
    const double epsilon = 0.15;
    if ( fabs( maxRadius - cellRadius[opmNodeIndex] ) < epsilon * cellRadius[opmNodeIndex] )
    {
        std::array<double, 8> opmX{};
        std::array<double, 8> opmY{};
        std::array<double, 8> opmZ{};

        opmGrid.getCellCorners( opmCellIndex, opmX, opmY, opmZ );

        double closestPillarDistance = std::numeric_limits<double>::max();
        int    closestPillarIndex    = -1;

        const auto cylinderCoordX = opmX[opmNodeIndex] + xCenterCoordOpm;
        const auto cylinderCoordY = opmY[opmNodeIndex] + yCenterCoordOpm;
        const auto cylinderCoordZ = opmZ[opmNodeIndex];

        const cvf::Vec3d coordinateOnCylinder = cvf::Vec3d( cylinderCoordX, cylinderCoordY, cylinderCoordZ );

        const auto candidates = computeSnapToCoordinates( opmMainGrid, opmGrid, hostCellIndex, opmCellIndex );
        for ( int pillarIndex = 0; pillarIndex < static_cast<int>( candidates.size() ); pillarIndex++ )
        {
            for ( const auto& c : candidates[pillarIndex] )
            {
                double distance = coordinateOnCylinder.pointDistance( c );
                if ( distance < closestPillarDistance )
                {
                    closestPillarDistance = distance;
                    closestPillarIndex    = pillarIndex;
                }
            }
        }

        if ( closestPillarDistance < std::numeric_limits<double>::max() )
        {
            const auto& pillarCordinates = candidates[closestPillarIndex];

            int layerCount               = static_cast<int>( pillarCordinates.size() / 2 );
            int layerIndexInMainGridCell = ijkCell[2] % layerCount;
            int localNodeIndex           = opmNodeIndex % 8;

            cvf::Vec3d closestPillarCoord;
            if ( localNodeIndex < 4 )
            {
                // Top of cell
                int pillarCoordIndex = layerIndexInMainGridCell * 2;
                closestPillarCoord   = pillarCordinates[pillarCoordIndex];
            }
            else
            {
                // Bottom of cell
                int pillarCoordIndex = layerIndexInMainGridCell * 2 + 1;
                closestPillarCoord   = pillarCordinates[pillarCoordIndex];
            }

            riNode.x() = closestPillarCoord.x();
            riNode.y() = closestPillarCoord.y();
            riNode.z() = -closestPillarCoord.z();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmRadialGridTools::createAngularGridRefinement( RigEclipseCaseData* caseData, size_t angularRefinement )
{
    if ( !caseData || !caseData->mainGrid() ) return false;

    auto riMainGrid = caseData->mainGrid();

    const int id      = 100;
    const int nRadial = 1;
    const int nK      = 1;

    const caf::VecIjk mainGridStart( 0, 0, 0 );
    const caf::VecIjk mainGridEnd( riMainGrid->cellCountI() - 1, riMainGrid->cellCountJ() - 1, riMainGrid->cellCountK() - 1 );
    const caf::VecIjk refinement( nRadial, angularRefinement, nK );

    LgrInfo lgrInfo{ id, "Radial LGR", "", refinement, mainGridStart, mainGridEnd };

    RicCreateTemporaryLgrFeature::createLgr( lgrInfo, caseData );

    // Convert the radial grid to a Cartesian grid
    std::vector<cvf::Vec3d>& nodes = riMainGrid->nodes();
    convertNodesToCartesian( nodes );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmRadialGridTools::convertNodesToCartesian( std::vector<cvf::Vec3d>& nodes )
{
    for ( size_t i = 0; i < nodes.size(); i++ )
    {
        auto& node = nodes[i];

        auto cartesian = Ria::CoordinateConverter::cylindricalToCartesian( node.x(), Ria::Degreesd( node.y() ), node.z() );
        node.x()       = cartesian.x();
        node.y()       = cartesian.y();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, std::pair<double, double>>
    RifOpmRadialGridTools::computeXyCenterForTopOfCells( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, RigGridBase* riGrid )
{
    if ( !riGrid || riGrid->isMainGrid() ) return {};

    size_t cellCount = opmGrid.totalNumberOfCells();
    if ( cellCount != riGrid->cellCount() ) return {};

    // Read out the corner coordinates from the EGRID file using radial coordinates.

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

            auto ijkLocalGrid = opmGrid.ijk_from_global_index( static_cast<int>( cIdx ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RifOpmRadialGridTools::computeSnapToCoordinates( Opm::EclIO::EGrid& opmMainGrid,
                                                                                      Opm::EclIO::EGrid& opmGrid,
                                                                                      int                mainGridCellIndex,
                                                                                      int                lgrCellIndex )
{
    auto hostCellIndices = opmGrid.hostCellsGlobalIndex();
    auto lgrIjk          = opmGrid.ijk_from_global_index( lgrCellIndex );

    std::vector<double> zDistanceAlongPillar;

    for ( int gridCellIndex = 0; gridCellIndex < opmGrid.totalNumberOfCells(); gridCellIndex++ )
    {
        if ( hostCellIndices[gridCellIndex] == mainGridCellIndex )
        {
            auto ijk = opmGrid.ijk_from_global_index( gridCellIndex );

            // Find all LGR cells for the same IJ column
            if ( ijk[0] == lgrIjk[0] && ijk[1] == lgrIjk[1] )
            {
                std::array<double, 8> cellX{};
                std::array<double, 8> cellY{};
                std::array<double, 8> cellZ{};

                opmGrid.getCellCorners( gridCellIndex, cellX, cellY, cellZ );

                // Get top and bottom of one pillar
                zDistanceAlongPillar.push_back( cellZ[0] );
                zDistanceAlongPillar.push_back( cellZ[4] );
            }
        }
    }

    if ( zDistanceAlongPillar.size() < 2 ) return {};

    std::sort( zDistanceAlongPillar.begin(), zDistanceAlongPillar.end() );

    auto normalize = []( const std::vector<double>& values ) -> std::vector<double>
    {
        if ( values.size() < 2 ) return {};

        std::vector<double> normalizedValues;

        double firstValue = values.front();
        double lastValue  = values.back();
        double range      = lastValue - firstValue;

        // Normalize values to range [0..1]
        for ( const auto& value : values )
        {
            normalizedValues.emplace_back( ( value - firstValue ) / range );
        }

        return normalizedValues;
    };

    auto normalizedZValues = normalize( zDistanceAlongPillar );

    std::vector<std::vector<cvf::Vec3d>> allCoords;
    std::array<double, 8>                hostCellX{};
    std::array<double, 8>                hostCellY{};
    std::array<double, 8>                hostCellZ{};

    opmMainGrid.getCellCorners( mainGridCellIndex, hostCellX, hostCellY, hostCellZ );

    for ( int pillarIndex = 0; pillarIndex < 4; pillarIndex++ )
    {
        std::vector<cvf::Vec3d> pillarCoords;

        const cvf::Vec3d p1( hostCellX[0 + pillarIndex], hostCellY[0 + pillarIndex], hostCellZ[0 + pillarIndex] );
        const cvf::Vec3d p2( hostCellX[4 + pillarIndex], hostCellY[4 + pillarIndex], hostCellZ[4 + pillarIndex] );

        for ( auto t : normalizedZValues )
        {
            cvf::Vec3d pillarCoord = p1 * ( 1.0 - t ) + t * p2;
            pillarCoords.push_back( pillarCoord );
        }

        allCoords.push_back( pillarCoords );
    }

    return allCoords;
}
