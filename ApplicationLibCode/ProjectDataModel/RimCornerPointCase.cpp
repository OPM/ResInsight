/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimCornerPointCase.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaLogging.h"

#include "RifInputPropertyLoader.h"
#include "RifRoffFileTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseInputProperty.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafProgressInfo.h"

#include <QDir>
#include <QFileInfo>

#include <chrono>

#ifdef USE_OPENMP
#include <omp.h>
#endif

using namespace std::chrono;

CAF_PDM_SOURCE_INIT( RimCornerPointCase, "CornerPointCase", "RimCornerPointCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCornerPointCase::RimCornerPointCase()
    : RimEclipseCase()
{
    CAF_PDM_InitScriptableObject( "RimCornerPointCase", ":/EclipseInput48x48.png" );
    setReservoirData( new RigEclipseCaseData( this ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCornerPointCase::~RimCornerPointCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCornerPointCase::openEclipseGridFile()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimCornerPointCase*, QString> RimCornerPointCase::createFromCoordinatesArray( const int                 nx,
                                                                                            const int                 ny,
                                                                                            const int                 nz,
                                                                                            const std::vector<float>& coord,
                                                                                            const std::vector<float>& zcorn,
                                                                                            const std::vector<float>& actnum )
{
    CAF_ASSERT( nx > 0 );
    CAF_ASSERT( ny > 0 );
    CAF_ASSERT( nz > 0 );

    size_t ncoord = ( nx + 1 ) * ( ny + 1 ) * 2 * 3;
    size_t nzcorn = nx * ny * nz * 8;
    size_t ntot   = nx * ny * nz;

    if ( coord.size() != ncoord )
        return std::unexpected( QString( "Wrong size of coord array. Expected %1, but got %2" ).arg( ncoord ).arg( coord.size() ) );

    if ( zcorn.size() != nzcorn )
        return std::unexpected( QString( "Wrong size of zcorn array. Expected %1, but got %2" ).arg( nzcorn ).arg( zcorn.size() ) );

    if ( actnum.size() != ntot )
        return std::unexpected( QString( "Wrong size of actnum array. Expected %1, but got %2" ).arg( ntot ).arg( actnum.size() ) );

    auto cornerPointCase = new RimCornerPointCase;

    buildGrid( *cornerPointCase->eclipseCaseData(), nx, ny, nz, coord, zcorn, actnum );
    cornerPointCase->computeCachedData();

    return cornerPointCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8> RimCornerPointCase::getCorners( const RigMainGrid&        grid,
                                                          const std::vector<float>& coord,
                                                          const std::vector<float>& zcorn,
                                                          const size_t              cellIdx,
                                                          const cvf::Vec3d&         offset,
                                                          const cvf::Vec3d&         scale )
{
    size_t i;
    size_t j;
    size_t k;
    grid.ijkFromCellIndex( cellIdx, &i, &j, &k );

    const size_t nx = grid.cellCountI();
    const size_t ny = grid.cellCountJ();

    // Get depths from zcorn
    std::array<size_t, 8> zind;
    zind[0] = ( k * nx * ny * 8 + j * nx * 4 + i * 2 );
    zind[1] = ( zind[0] + 1 );
    zind[2] = ( zind[0] + nx * 2 );
    zind[3] = ( zind[2] + 1 );

    for ( size_t n = 0; n < 4; n++ )
        zind[n + 4] = ( zind[n] + nx * ny * 4 );

    std::array<cvf::Vec3d, 8> corners;
    for ( size_t n = 0; n < 8; n++ )
        corners[n].z() = zcorn[zind[n]];

    // calculate indices for grid pillars in COORD arrray
    std::array<size_t, 4> pind;
    pind[0] = j * ( nx + 1 ) * 6 + i * 6;
    pind[1] = pind[0] + 6;
    pind[2] = pind[0] + ( nx + 1 ) * 6;
    pind[3] = pind[2] + 6;

    for ( size_t n = 0; n < 4; n++ )
    {
        const double zt = coord[pind[n] + 2];
        const double zb = coord[pind[n] + 5];

        const double xt = coord[pind[n]];
        const double yt = coord[pind[n] + 1];
        const double xb = coord[pind[n] + 3];
        const double yb = coord[pind[n] + 4];

        const double diffZ = zt - zb;

        if ( diffZ == 0.0 )
        {
            corners[n].x()     = xt;
            corners[n + 4].x() = xt;
            corners[n].y()     = yt;
            corners[n + 4].y() = yt;
        }
        else
        {
            corners[n].x()     = xt + ( xb - xt ) / diffZ * ( zt - corners[n].z() );
            corners[n + 4].x() = xt + ( xb - xt ) / diffZ * ( zt - corners[n + 4].z() );
            corners[n].y()     = yt + ( yb - yt ) / diffZ * ( zt - corners[n].z() );
            corners[n + 4].y() = yt + ( yb - yt ) / diffZ * ( zt - corners[n + 4].z() );
        }
    }

    return corners;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCornerPointCase::buildGrid( RigEclipseCaseData&       eclipseCaseData,
                                    const int                 nx,
                                    const int                 ny,
                                    const int                 nz,
                                    const std::vector<float>& coord,
                                    const std::vector<float>& zcorn,
                                    const std::vector<float>& actnum )
{
    auto startTime = high_resolution_clock::now();

    RigActiveCellInfo* activeCellInfo = eclipseCaseData.activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    CVF_ASSERT( activeCellInfo );

    RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData.activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );
    CVF_ASSERT( fractureActiveCellInfo );

    RigMainGrid* mainGrid = eclipseCaseData.mainGrid();
    CVF_ASSERT( mainGrid );

    mainGrid->setCellCounts( cvf::Vec3st( nx, ny, nz ) );

    mainGrid->setGridName( "Main grid" );

    size_t totalCellCount = nx * ny * nz;

    activeCellInfo->setGridCount( 1 );
    fractureActiveCellInfo->setGridCount( 1 );

    activeCellInfo->setReservoirCellCount( totalCellCount );
    fractureActiveCellInfo->setReservoirCellCount( totalCellCount );

    // Reserve room for the cells and nodes and fill them with data
    mainGrid->reservoirCells().reserve( totalCellCount );
    mainGrid->nodes().reserve( 8 * totalCellCount );

    int               progTicks = 100;
    caf::ProgressInfo progInfo( progTicks, "" );

    int    cellCount      = static_cast<int>( totalCellCount );
    size_t cellStartIndex = mainGrid->reservoirCells().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid( mainGrid );
    mainGrid->reservoirCells().resize( cellStartIndex + cellCount, defaultCell );

    mainGrid->nodes().resize( nodeStartIndex + static_cast<size_t>( cellCount ) * 8, cvf::Vec3d( 0, 0, 0 ) );

    const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

    cvf::Vec3d offset( 0.0, 0.0, 0.0 );
    cvf::Vec3d scale( 1.0, 1.0, 1.0 );

    // Convert actnum to integer
    std::vector<int> activeCells( nx * ny * nz, 0 );
    CAF_ASSERT( activeCells.size() == actnum.size() );
    for ( size_t i = 0; i < activeCells.size(); i++ )
    {
        activeCells[i] = static_cast<int>( actnum[i] > 0.0 );
    }

    // Precompute the active cell matrix index
    size_t numActiveCells = RifRoffFileTools::computeActiveCellMatrixIndex( activeCells );

    // Loop over cells and fill them with data
#pragma omp for
    for ( int gridLocalCellIndex = 0; gridLocalCellIndex < cellCount; ++gridLocalCellIndex )
    {
        RigCell& cell = mainGrid->cell( cellStartIndex + gridLocalCellIndex );

        cell.setGridLocalCellIndex( gridLocalCellIndex );

        // Active cell index
        int matrixActiveIndex = activeCells[gridLocalCellIndex];
        if ( matrixActiveIndex != -1 )
        {
            activeCellInfo->setCellResultIndex( cellStartIndex + gridLocalCellIndex, matrixActiveIndex );
        }

        cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );

        std::array<cvf::Vec3d, 8> corners = getCorners( *mainGrid, coord, zcorn, gridLocalCellIndex, offset, scale );

        // Corner coordinates
        for ( int cIdx = 0; cIdx < 8; ++cIdx )
        {
            double* point = mainGrid->nodes()[nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cellMappingECLRi[cIdx]].ptr();

            point[0] = corners[cIdx].x();
            point[1] = corners[cIdx].y();
            point[2] = -corners[cIdx].z();

            cell.cornerIndices()[cIdx] = nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cIdx;
        }

        // Mark inactive long pyramid looking cells as invalid
        cell.setInvalid( cell.isLongPyramidCell() );
    }

    activeCellInfo->setGridActiveCellCounts( 0, numActiveCells );
    fractureActiveCellInfo->setGridActiveCellCounts( 0, 0 );

    mainGrid->initAllSubGridsParentGridPointer();
    activeCellInfo->computeDerivedData();
    fractureActiveCellInfo->computeDerivedData();

    auto endTime = high_resolution_clock::now();

    auto totalDuration = duration_cast<milliseconds>( endTime - startTime );
    RiaLogging::info( QString( "Total: %1 ms" ).arg( totalDuration.count() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCornerPointCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_caseUserDescription );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_caseId );
    uiOrdering.add( &m_caseFileName );

    auto group = uiOrdering.addNewGroup( "Case Options" );
    group->add( &m_activeFormationNames );
    group->add( &m_flipXAxis );
    group->add( &m_flipYAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCornerPointCase::locationOnDisc() const
{
    if ( gridFileName().isEmpty() ) return QString();

    QFileInfo fi( gridFileName() );
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCornerPointCase::importAsciiInputProperties( const QStringList& fileNames )
{
    return true;
}
