/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "RifReaderOpmCommonActive.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "RifOpmRadialGridTools.h"

#include "RigActiveCellGrid.h"
#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"

#include "cafProgressInfo.h"

#include "opm/io/eclipse/EGrid.hpp"

#include <QStringList>

using namespace Opm;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommonActive::RifReaderOpmCommonActive()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommonActive::~RifReaderOpmCommonActive()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommonActive::importGrid( RigMainGrid* /* mainGrid*/, RigEclipseCaseData* eclipseCaseData )
{
    RigActiveCellGrid* activeGrid = new RigActiveCellGrid();
    eclipseCaseData->setMainGrid( activeGrid );

    caf::ProgressInfo progInfo( 5, "Importing Eclipse Grid" );

    Opm::EclIO::EGrid opmGrid( m_gridFileName );

    const auto& dims = opmGrid.dimension();
    activeGrid->setGridPointDimensions( cvf::Vec3st( dims[0] + 1, dims[1] + 1, dims[2] + 1 ) );
    activeGrid->setGridName( "Main grid" );
    activeGrid->setDualPorosity( opmGrid.porosity_mode() > 0 );

    // assign grid unit, if found (1 = Metric, 2 = Field, 3 = Lab)
    auto gridUnitStr = RiaStdStringTools::toUpper( opmGrid.grid_unit() );
    if ( gridUnitStr.starts_with( 'M' ) )
        m_gridUnit = 1;
    else if ( gridUnitStr.starts_with( 'F' ) )
        m_gridUnit = 2;
    else if ( gridUnitStr.starts_with( 'C' ) )
        m_gridUnit = 3;

    auto globalMatrixActiveSize   = opmGrid.activeCells();
    auto globalFractureActiveSize = opmGrid.activeFracCells();

    m_gridNames.clear();
    m_gridNames.push_back( "global" );

    std::vector<Opm::EclIO::EGrid> lgrGrids; // lgrs not supported here for now

    // active cell information
    {
        auto task = progInfo.task( "Getting Active Cell Information", 1 );

        // in case init file and grid file disagrees with number of active cells, read extra porv information from init file to correct this
        if ( !verifyActiveCellInfo( globalMatrixActiveSize, globalFractureActiveSize ) )
        {
            updateActiveCellInfo( eclipseCaseData, opmGrid, lgrGrids, activeGrid );
        }

        activeGrid->transferActiveInformation( eclipseCaseData,
                                               opmGrid.totalActiveCells(),
                                               opmGrid.activeCells(),
                                               opmGrid.activeFracCells(),
                                               opmGrid.active_indexes(),
                                               opmGrid.active_frac_indexes() );
    }

    // grid geometry
    {
        RiaLogging::info( QString( "Loading %0 active of %1 total cells." )
                              .arg( QString::fromStdString( RiaStdStringTools::formatThousandGrouping( opmGrid.totalActiveCells() ) ) )
                              .arg( QString::fromStdString( RiaStdStringTools::formatThousandGrouping( opmGrid.totalNumberOfCells() ) ) ) );

        auto task = progInfo.task( "Loading Active Cell Main Grid Geometry", 1 );
        transferActiveGeometry( opmGrid, activeGrid, eclipseCaseData );
    }

    activeGrid->initAllSubGridsParentGridPointer();

    if ( isNNCsEnabled() )
    {
        auto task = progInfo.task( "Loading NNC data", 1 );
        transferStaticNNCData( opmGrid, lgrGrids, activeGrid );
    }

    auto opmMapAxes = opmGrid.get_mapaxes();
    if ( opmMapAxes.size() == 6 )
    {
        std::array<double, 6> mapAxes;
        for ( size_t i = 0; i < opmMapAxes.size(); ++i )
        {
            mapAxes[i] = opmMapAxes[i];
        }

        double norm_denominator = mapAxes[2] * mapAxes[5] - mapAxes[4] * mapAxes[3];

        // Set the map axes transformation matrix on the main grid
        activeGrid->setMapAxes( mapAxes );
        activeGrid->setUseMapAxes( norm_denominator != 0.0 );

        auto transform = activeGrid->mapAxisTransform();

        // Invert the transformation matrix to convert from file coordinates to domain coordinates
        transform.invert();

#pragma omp parallel for
        for ( long i = 0; i < static_cast<long>( activeGrid->nodes().size() ); i++ )
        {
            auto& n = activeGrid->nodes()[i];
            n.transformPoint( transform );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommonActive::transferActiveGeometry( Opm::EclIO::EGrid&  opmMainGrid,
                                                       RigActiveCellGrid*  activeGrid,
                                                       RigEclipseCaseData* eclipseCaseData )
{
    int cellCount = opmMainGrid.totalActiveCells();

    RigCell defaultCell;
    defaultCell.setHostGrid( activeGrid );
    for ( size_t i = 0; i < 8; i++ )
        defaultCell.cornerIndices()[i] = 0;

    activeGrid->globalCellArray().resize( cellCount + 1, defaultCell );
    activeGrid->globalCellArray()[cellCount].setInvalid( true );

    activeGrid->nodes().resize( ( cellCount + 1 ) * 8, cvf::Vec3d( 0, 0, 0 ) );

    auto& riNodes = activeGrid->nodes();

    opmMainGrid.loadData();
    opmMainGrid.load_grid_data();

    const bool  isRadialGrid      = opmMainGrid.is_radial();
    const auto& activeMatIndexes  = opmMainGrid.active_indexes();
    const auto& activeFracIndexes = opmMainGrid.active_frac_indexes();

    // Compute the center of the LGR radial grid cells for each K layer
    auto radialGridCenterTopLayerOpm = isRadialGrid
                                           ? RifOpmRadialGridTools::computeXyCenterForTopOfCells( opmMainGrid, opmMainGrid, activeGrid )
                                           : std::map<int, std::pair<double, double>>();

    const bool invalidateLongPyramidCells = invalidateLongThinCells();

    // use same mapping as resdata
    const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( opmMainGrid.totalNumberOfCells() ); opmCellIndex++ )
    {
        if ( ( activeMatIndexes[opmCellIndex] < 0 ) && ( activeFracIndexes[opmCellIndex] < 0 ) ) continue;

        auto opmIJK = opmMainGrid.ijk_from_global_index( opmCellIndex );

        double xCenterCoordOpm = 0.0;
        double yCenterCoordOpm = 0.0;

        if ( isRadialGrid && radialGridCenterTopLayerOpm.contains( opmIJK[2] ) )
        {
            const auto& [xCenter, yCenter] = radialGridCenterTopLayerOpm[opmIJK[2]];
            xCenterCoordOpm                = xCenter;
            yCenterCoordOpm                = yCenter;
        }

        auto     riReservoirIndex = activeGrid->cellIndexFromIJK( opmIJK[0], opmIJK[1], opmIJK[2] );
        RigCell& cell             = activeGrid->globalCellArray()[riReservoirIndex];
        cell.setGridLocalCellIndex( riReservoirIndex );
        cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );

        // corner coordinates
        std::array<double, 8> opmX{};
        std::array<double, 8> opmY{};
        std::array<double, 8> opmZ{};
        opmMainGrid.getCellCorners( opmCellIndex, opmX, opmY, opmZ );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto riNodeStartIndex = riReservoirIndex * 8;

        for ( size_t opmNodeIndex = 0; opmNodeIndex < 8; opmNodeIndex++ )
        {
            auto   riCornerIndex = cellMappingECLRi[opmNodeIndex];
            size_t riNodeIndex   = riNodeStartIndex + riCornerIndex;

            auto& riNode = riNodes[riNodeIndex];
            riNode.x()   = opmX[opmNodeIndex] + xCenterCoordOpm;
            riNode.y()   = opmY[opmNodeIndex] + yCenterCoordOpm;
            riNode.z()   = -opmZ[opmNodeIndex];

            cell.cornerIndices()[riCornerIndex] = riNodeIndex;
        }

        if ( invalidateLongPyramidCells )
        {
            cell.setInvalid( cell.isLongPyramidCell() );
        }
    }
}
