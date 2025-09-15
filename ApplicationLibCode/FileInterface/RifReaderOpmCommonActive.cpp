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
#include "RigActiveCellLocalGrid.h"
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

    caf::ProgressInfo progInfo( 4, "Importing Eclipse Grid" );

    Opm::EclIO::EGrid opmGrid( m_gridFileName );

    const auto& dims = opmGrid.dimension();
    activeGrid->setCellCounts( cvf::Vec3st( dims[0], dims[1], dims[2] ) );
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

    auto totalCellCount           = opmGrid.totalNumberOfCells();
    auto totalActiveCellCount     = opmGrid.totalActiveCells();
    auto globalMatrixActiveSize   = opmGrid.activeCells();
    auto globalFractureActiveSize = opmGrid.activeFracCells();

    const auto& lgr_names = opmGrid.list_of_lgrs();
    m_gridNames.clear();
    m_gridNames.push_back( "global" );
    m_gridNames.insert( m_gridNames.end(), lgr_names.begin(), lgr_names.end() );
    const auto& lgr_parent_names = opmGrid.list_of_lgr_parents();
    const int   numLGRs          = (int)lgr_names.size();

    std::vector<Opm::EclIO::EGrid> lgrGrids;

    // init LGR grids
    for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
    {
        lgrGrids.emplace_back( Opm::EclIO::EGrid( m_gridFileName, lgr_names[lgrIdx] ) );
        RigActiveCellLocalGrid* localGrid = new RigActiveCellLocalGrid( activeGrid );

        const auto& lgrDims = lgrGrids[lgrIdx].dimension();
        localGrid->setCellCounts( cvf::Vec3st( lgrDims[0], lgrDims[1], lgrDims[2] ) );
        localGrid->setGridId( lgrIdx + 1 );
        localGrid->setGridName( lgr_names[lgrIdx] );
        localGrid->setIndexToStartOfCells( totalCellCount );
        activeGrid->addLocalGrid( localGrid );

        totalCellCount += lgrGrids[lgrIdx].totalNumberOfCells();
        totalActiveCellCount += lgrGrids[lgrIdx].totalActiveCells();
    }

    activeGrid->setTotalActiveCellCount( totalActiveCellCount );

    // active cell information
    {
        RigActiveCellInfo* activeCellInfo         = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
        RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

        activeCellInfo->setReservoirCellCount( totalCellCount );
        fractureActiveCellInfo->setReservoirCellCount( totalCellCount );
        activeCellInfo->setGridCount( 1 + numLGRs );
        fractureActiveCellInfo->setGridCount( 1 + numLGRs );

        auto task = progInfo.task( "Getting Active Cell Information" );

        for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
        {
            globalMatrixActiveSize += lgrGrids[lgrIdx].activeCells();
            globalFractureActiveSize += lgrGrids[lgrIdx].activeFracCells();
        }

        // in case init file and grid file disagrees with number of active cells, read extra porv information from init file to correct this
        if ( !verifyActiveCellInfo( globalMatrixActiveSize, globalFractureActiveSize ) )
        {
            updateActiveCellInfo( eclipseCaseData, opmGrid, lgrGrids, activeGrid );
        }

        globalMatrixActiveSize   = opmGrid.activeCells();
        globalFractureActiveSize = opmGrid.activeFracCells();

        activeCellInfo->setGridActiveCellCounts( 0, globalMatrixActiveSize );
        fractureActiveCellInfo->setGridActiveCellCounts( 0, globalFractureActiveSize );

        transferActiveCells( opmGrid, 0, eclipseCaseData, 0, 0 );
        size_t cellCount = opmGrid.totalNumberOfCells();

        for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
        {
            auto& lgrGrid = lgrGrids[lgrIdx];
            transferActiveCells( lgrGrid, cellCount, eclipseCaseData, globalMatrixActiveSize, globalFractureActiveSize );
            cellCount += lgrGrid.totalNumberOfCells();
            globalMatrixActiveSize += lgrGrid.activeCells();
            globalFractureActiveSize += lgrGrid.activeFracCells();
            activeCellInfo->setGridActiveCellCounts( lgrIdx + 1, lgrGrid.activeCells() );
            fractureActiveCellInfo->setGridActiveCellCounts( lgrIdx + 1, lgrGrid.activeFracCells() );
        }

        activeCellInfo->computeDerivedData();
        fractureActiveCellInfo->computeDerivedData();
    }

    // grid geometry
    {
        RiaLogging::info( QString( "Loading %0 active of %1 total cells in main grid." )
                              .arg( QString::fromStdString( RiaStdStringTools::formatThousandGrouping( opmGrid.totalActiveCells() ) ) )
                              .arg( QString::fromStdString( RiaStdStringTools::formatThousandGrouping( opmGrid.totalNumberOfCells() ) ) ),
                          "RifReaderOpmCommonActive" );

        auto task = progInfo.task( "Loading Active Cell Main Grid Geometry" );
        transferActiveGeometry( opmGrid, opmGrid, activeGrid, activeGrid, eclipseCaseData );

        bool hasParentInfo = ( lgr_parent_names.size() >= (size_t)numLGRs );

        auto task2 = progInfo.task( "Loading Active Cell LGR Grid Geometry" );

        for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
        {
            RiaLogging::info( QString( "Loading %0 active of %1 total cells in LGR grid %2." )
                                  .arg( QString::fromStdString( RiaStdStringTools::formatThousandGrouping( lgrGrids[lgrIdx].totalActiveCells() ) ) )
                                  .arg( QString::fromStdString(
                                      RiaStdStringTools::formatThousandGrouping( lgrGrids[lgrIdx].totalNumberOfCells() ) ) )
                                  .arg( lgrIdx + 1 ),
                              "RifReaderOpmCommonActive" );

            RigGridBase* parentGrid = hasParentInfo ? activeGrid->gridByName( lgr_parent_names[lgrIdx] ) : activeGrid;

            RigActiveCellLocalGrid* localGrid = dynamic_cast<RigActiveCellLocalGrid*>( activeGrid->gridById( lgrIdx + 1 ) );
            if ( localGrid != nullptr )
            {
                localGrid->setParentGrid( parentGrid );
                transferActiveGeometry( opmGrid, lgrGrids[lgrIdx], activeGrid, localGrid, eclipseCaseData );
            }
        }
    }

    activeGrid->initAllSubGridsParentGridPointer();

    if ( isNNCsEnabled() )
    {
        auto task = progInfo.task( "Loading NNC data" );
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
                                                       Opm::EclIO::EGrid&  opmGrid,
                                                       RigActiveCellGrid*  activeGrid,
                                                       RigGridBase*        localGrid,
                                                       RigEclipseCaseData* eclipseCaseData )
{
    int    cellCount      = opmGrid.totalActiveCells();
    size_t cellStartIndex = activeGrid->totalCellCount();
    size_t nodeStartIndex = activeGrid->nodes().size();

    const bool invalidateLongPyramidCells = invalidateLongThinCells();

    RigCell defaultCell;
    defaultCell.setHostGrid( localGrid );
    for ( size_t i = 0; i < 8; i++ )
        defaultCell.cornerIndices()[i] = 0;

    const auto newNodeCount = nodeStartIndex + 8 * cellCount;
    activeGrid->nodes().resize( newNodeCount, cvf::Vec3d( 0, 0, 0 ) );
    activeGrid->setTotalCellCount( cellStartIndex + opmGrid.totalNumberOfCells() );

    auto& riNodes = activeGrid->nodes();
    auto& riCells = activeGrid->nativeCells();

    opmGrid.loadData();
    opmGrid.load_grid_data();

    const bool  isRadialGrid          = opmGrid.is_radial();
    const auto& activeMatIndexes      = opmGrid.active_indexes();
    const auto& activeFracIndexes     = opmGrid.active_frac_indexes();
    const auto& gridDimension         = opmGrid.dimension();
    const auto& hostCellGlobalIndices = opmGrid.hostCellsGlobalIndex();

    // Compute the center of the LGR radial grid cells for each K layer
    auto radialGridCenterTopLayerOpm = isRadialGrid ? RifOpmRadialGridTools::computeXyCenterForTopOfCells( opmMainGrid, opmGrid, localGrid )
                                                    : std::map<int, std::pair<double, double>>();

    // use same mapping as resdata
    const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

    std::map<int, int> activeCellMap;
    int                nativeIdx = 0;

    // non-parallell loop to set up and initialize things so that we can run in parallell later
    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( opmGrid.totalNumberOfCells() ); opmCellIndex++ )
    {
        if ( ( activeMatIndexes[opmCellIndex] < 0 ) && ( activeFracIndexes[opmCellIndex] < 0 ) ) continue;

        auto opmIJK                          = opmGrid.ijk_from_global_index( opmCellIndex );
        auto localIndex                      = localGrid->cellIndexFromIJK( opmIJK[0], opmIJK[1], opmIJK[2] );
        riCells[cellStartIndex + localIndex] = defaultCell;
        activeCellMap[opmCellIndex]          = nativeIdx++;
    }

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( opmGrid.totalNumberOfCells() ); opmCellIndex++ )
    {
        if ( ( activeMatIndexes[opmCellIndex] < 0 ) && ( activeFracIndexes[opmCellIndex] < 0 ) ) continue;

        auto opmIJK = opmGrid.ijk_from_global_index( opmCellIndex );

        double xCenterCoordOpm = 0.0;
        double yCenterCoordOpm = 0.0;

        if ( isRadialGrid && radialGridCenterTopLayerOpm.contains( opmIJK[2] ) )
        {
            const auto& [xCenter, yCenter] = radialGridCenterTopLayerOpm[opmIJK[2]];
            xCenterCoordOpm                = xCenter;
            yCenterCoordOpm                = yCenter;
        }

        auto     localIndex = localGrid->cellIndexFromIJK( opmIJK[0], opmIJK[1], opmIJK[2] );
        RigCell& cell       = riCells[cellStartIndex + localIndex];
        cell.setGridLocalCellIndex( localIndex );

        // parent cell index
        if ( ( hostCellGlobalIndices.size() > (size_t)opmCellIndex ) && hostCellGlobalIndices[opmCellIndex] >= 0 )
        {
            cell.setParentCellIndex( hostCellGlobalIndices[opmCellIndex] );
        }
        else
        {
            cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );
        }

        // corner coordinates
        std::array<double, 8> opmX{};
        std::array<double, 8> opmY{};
        std::array<double, 8> opmZ{};
        opmGrid.getCellCorners( opmCellIndex, opmX, opmY, opmZ );

        // Each cell has 8 nodes, use active cell index and multiply to find first node index for cell
        auto localNodeIndex   = activeCellMap[opmCellIndex] * 8;
        auto riNodeStartIndex = nodeStartIndex + localNodeIndex;

        for ( size_t opmNodeIndex = 0; opmNodeIndex < 8; opmNodeIndex++ )
        {
            auto   riCornerIndex = cellMappingECLRi[opmNodeIndex];
            size_t riNodeIndex   = riNodeStartIndex + riCornerIndex;

            auto& riNode = riNodes[riNodeIndex];
            riNode.x()   = opmX[opmNodeIndex] + xCenterCoordOpm;
            riNode.y()   = opmY[opmNodeIndex] + yCenterCoordOpm;
            riNode.z()   = -opmZ[opmNodeIndex];

            cell.cornerIndices()[riCornerIndex] = riNodeIndex;

            // First grid dimension is radius, check if cell are at the outer-most slice
            if ( isRadialGrid && !hostCellGlobalIndices.empty() && ( gridDimension[0] - 1 == opmIJK[0] ) )
            {
                auto hostCellIndex = hostCellGlobalIndices[opmCellIndex];

                RifOpmRadialGridTools::lockToHostPillars( riNode,
                                                          opmMainGrid,
                                                          opmGrid,
                                                          opmIJK,
                                                          hostCellIndex,
                                                          opmCellIndex,
                                                          opmNodeIndex,
                                                          xCenterCoordOpm,
                                                          yCenterCoordOpm );
            }
        }

        if ( invalidateLongPyramidCells )
        {
            cell.setInvalid( cell.isLongPyramidCell() );
        }
    }

    // subgrid pointers
    RigLocalGrid* realLocalGrid = dynamic_cast<RigLocalGrid*>( localGrid );
    RigGridBase*  parentGrid    = realLocalGrid != nullptr ? realLocalGrid->parentGrid() : nullptr;

    if ( parentGrid != nullptr )
    {
        for ( auto localCellInGlobalIdx : hostCellGlobalIndices )
        {
            auto& cell = parentGrid->cell( localCellInGlobalIdx );
            if ( !cell.isInvalid() ) cell.setSubGrid( realLocalGrid );
        }
    }
}
