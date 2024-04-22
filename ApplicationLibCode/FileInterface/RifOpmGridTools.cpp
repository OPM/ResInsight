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

#include "RiaLogging.h"
#include "RiaWeightedMeanCalculator.h"

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
void RifOpmGridTools::importCoordinatesForRadialGrid( const std::string& gridFilePath, RigMainGrid* riMainGrid )
{
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

        if ( !isRadialGridPresent ) return;

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
        RiaLogging::warning(
            QString( "Failed to open grid case for import of radial coordinates : %1" ).arg( QString::fromStdString( gridFilePath ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifOpmGridTools::cellCount( const std::string& gridFilePath )
{
    Opm::EclIO::EGrid opmGrid( gridFilePath );

    return opmGrid.totalNumberOfCells();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmGridTools::importGrid( const std::string& gridFilePath, RigMainGrid* mainGrid, RigEclipseCaseData* caseData )
{
    Opm::EclIO::EGrid opmGrid( gridFilePath );

    auto dims = opmGrid.dimension();
    mainGrid->setGridPointDimensions( cvf::Vec3st( dims[0] + 1, dims[1] + 1, dims[2] + 1 ) );

    RigCell defaultCell;
    defaultCell.setHostGrid( mainGrid );
    auto cellCount = opmGrid.totalNumberOfCells();
    mainGrid->globalCellArray().resize( cellCount, defaultCell );
    mainGrid->nodes().resize( 8 * cellCount );

    transferCoordinatesCartesian( opmGrid, opmGrid, mainGrid, mainGrid, caseData );

    auto opmMapAxes = opmGrid.get_mapaxes();
    if ( opmMapAxes.size() == 6 )
    {
        std::array<double, 6> mapAxes;
        for ( size_t i = 0; i < opmMapAxes.size(); ++i )
        {
            mapAxes[i] = opmMapAxes[i];
        }

        // Set the map axes transformation matrix on the main grid
        mainGrid->setMapAxes( mapAxes );
        mainGrid->setUseMapAxes( true );

        auto transform = mainGrid->mapAxisTransform();

        // Invert the transformation matrix to convert from file coordinates to domain coordinates
        transform.invert();

#pragma omp parallel for
        for ( long i = 0; i < static_cast<long>( mainGrid->nodes().size() ); i++ )
        {
            auto& n = mainGrid->nodes()[i];
            n.transformPoint( transform );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> RifOpmGridTools::activeCellsFromActnumKeyword( Opm::EclIO::EGrid& grid )
{
    auto arrayNames       = grid.arrayNames();
    int  actnumArrayIndex = -1;

    for ( size_t i = 0; i < arrayNames.size(); i++ )
    {
        if ( arrayNames[i] == "ACTNUM" )
        {
            actnumArrayIndex = static_cast<int>( i );
            break;
        }
    }

    if ( actnumArrayIndex < 0 ) return {};

    auto actnumMainGrid = grid.get<int>( actnumArrayIndex );

    return { actnumMainGrid };
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
// 3. Find the closest point on this pillar, and use this point as the adjusted coordinate for the node
//
//--------------------------------------------------------------------------------------------------
void RifOpmGridTools::transferCoordinates( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, RigMainGrid* riMainGrid, RigGridBase* riGrid )
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

    std::vector<cvf::Vec3d> snapToCoordinatesFromMainGrid;

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
                std::array<double, 8> cellRadius{};
                std::array<double, 8> cellTheta{};
                std::array<double, 8> cellZ{};
                opmGrid.getRadialCellCorners( ijkCell, cellRadius, cellTheta, cellZ );

                double maxRadius = *std::max_element( cellRadius.begin(), cellRadius.end() );

                // Check if the radius is at the outer surface of the radial grid
                // Adjust the outer nodes to match the corner pillars of the host cell
                const double epsilon = 0.15;
                if ( fabs( maxRadius - cellRadius[opmNodeIndex] ) < epsilon * cellRadius[opmNodeIndex] )
                {
                    const auto hostCellIndex = hostCellGlobalIndices[opmCellIndex];

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
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmGridTools::transferCoordinatesCartesian( Opm::EclIO::EGrid&  opmMainGrid,
                                                    Opm::EclIO::EGrid&  opmGrid,
                                                    RigMainGrid*        riMainGrid,
                                                    RigGridBase*        riGrid,
                                                    RigEclipseCaseData* caseData )
{
    // Prefix OPM structures with _opm_and ResInsight structures with _ri_

    auto& riNodes = riMainGrid->nodes();

    opmGrid.loadData();
    opmGrid.load_grid_data();

    auto riActiveCells     = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    auto riActiveCellsFrac = caseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );
    riActiveCellsFrac->setGridCount( 1 );
    riActiveCellsFrac->setGridActiveCellCounts( 0, 0 );

    riActiveCells->setReservoirCellCount( riMainGrid->cellCount() );

    // same mapping as resdata
    const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( riMainGrid->cellCount() ); opmCellIndex++ )
    {
        auto opmIJK = opmGrid.ijk_from_global_index( opmCellIndex );

        auto     riReservoirIndex = riGrid->cellIndexFromIJK( opmIJK[0], opmIJK[1], opmIJK[2] );
        RigCell& cell             = riMainGrid->globalCellArray()[riReservoirIndex];
        cell.setGridLocalCellIndex( riReservoirIndex );

        std::array<double, 8> opmX{};
        std::array<double, 8> opmY{};
        std::array<double, 8> opmZ{};
        opmGrid.getCellCorners( opmCellIndex, opmX, opmY, opmZ );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto riNodeStartIndex = riReservoirIndex * 8;

        for ( size_t opmNodeIndex = 0; opmNodeIndex < 8; opmNodeIndex++ )
        {
            auto   riCornerIndex = cellMappingECLRi[opmNodeIndex];
            size_t riNodeIndex   = riNodeStartIndex + riCornerIndex;

            auto& riNode = riNodes[riNodeIndex];
            riNode.x()   = opmX[opmNodeIndex];
            riNode.y()   = opmY[opmNodeIndex];
            riNode.z()   = -opmZ[opmNodeIndex];

            cell.cornerIndices()[riCornerIndex] = riNodeIndex;
        }

        if ( riActiveCells )
        {
            auto activeIndex = opmGrid.active_index( opmIJK[0], opmIJK[1], opmIJK[2] );
            if ( activeIndex > -1 )
            {
                riActiveCells->setCellResultIndex( riReservoirIndex, activeIndex );
            }
        }
    }

    riActiveCells->setGridCount( 1 );
    riActiveCells->setGridActiveCellCounts( 0, opmGrid.activeCells() );
    riActiveCells->computeDerivedData();

    riMainGrid->initAllSubGridsParentGridPointer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, std::pair<double, double>>
    RifOpmGridTools::computeXyCenterForTopOfCells( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, RigGridBase* riGrid )
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
std::vector<std::vector<cvf::Vec3d>>
    RifOpmGridTools::computeSnapToCoordinates( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, int mainGridCellIndex, int lgrCellIndex )
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
