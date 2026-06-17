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

#include "RigNestedHybridGridReconstructor.h"

#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNncConnection.h"

#include "RiaDefines.h"

#include <array>
#include <cmath>
#include <limits>
#include <map>

namespace
{
size_t naturalIndex( size_t i, size_t j, size_t k, size_t nx, size_t ny )
{
    return i + j * nx + k * nx * ny;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNestedHybridGridReconstructor::reconstruct( RigEclipseCaseData* caseData, const std::vector<int>& hostNum, QString* errorMessage )
{
    auto setError = [&]( const QString& msg )
    {
        if ( errorMessage ) *errorMessage = msg;
        RiaLogging::warning( ( "Nested hybrid grid: " + msg ).toStdString() );
        return false;
    };

    if ( !caseData || !caseData->mainGrid() ) return setError( "No grid data." );

    RigMainGrid* grid = caseData->mainGrid();
    if ( hostNum.size() != grid->cellCount() )
    {
        return setError( QString( "HOSTNUM size %1 does not match main grid cell count %2." ).arg( hostNum.size() ).arg( grid->cellCount() ) );
    }

    if ( grid->gridCount() > 1 )
    {
        return setError( "Grid already contains local grids; skipping reconstruction." );
    }

    RegionInfo region;
    if ( !detectSingleRegion( hostNum, grid, &region, errorMessage ) ) return false;

    int                 gridId    = (int)grid->gridCount(); // main grid is 0
    size_t              cellStart = grid->totalCellCount();
    std::vector<size_t> lgrToFlatCell;
    RigLocalGrid*       localGrid = buildLocalGrid( caseData, region, gridId, lgrToFlatCell );
    if ( !localGrid ) return setError( "Failed to build local grid." );

    // Record the LGR cell -> source flat cell mapping (global indices) so result values can be
    // copied onto the LGR cells, both now and on each subsequent (lazy) result load.
    std::map<size_t, size_t> sourceCells;
    for ( size_t glc = 0; glc < lgrToFlatCell.size(); glc++ )
    {
        if ( lgrToFlatCell[glc] != cvf::UNDEFINED_SIZE_T ) sourceCells[cellStart + glc] = lgrToFlatCell[glc];
    }
    grid->setNestedHybridLgrSourceCells( sourceCells );

    updateActiveCellInfo( caseData, localGrid, lgrToFlatCell );
    extendFullLengthResults( caseData, cellStart, lgrToFlatCell );
    repointNncConnections( caseData, cellStart, lgrToFlatCell );

    // Fill LGR cells of any already-loaded active-cell-indexed results from their source flat cells.
    for ( auto model : { RiaDefines::PorosityModelType::MATRIX_MODEL, RiaDefines::PorosityModelType::FRACTURE_MODEL } )
    {
        if ( RigCaseCellResultsData* results = caseData->results( model ) ) results->extendNestedHybridLgrResults();
    }

    // The grid count changed - invalidate per-grid caches that were sized for the flat grid.
    caseData->clearWellCellsInGridCache();

    RiaLogging::info( QString( "Nested hybrid grid: created LGR with %1 cells (refinement %2x%3x%4)" )
                          .arg( localGrid->cellCount() )
                          .arg( region.refinement.x() )
                          .arg( region.refinement.y() )
                          .arg( region.refinement.z() )
                          .toStdString() );

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Detect the refined block extent, the parent coarse region and the per-axis refinement factor.
/// Currently handles a single refined region (sufficient for the supplied test model). The block
/// is the index bounding box of all refined cells; the parent region is the index bounding box of
/// the referenced HOSTNUM parents; the factor is blockDims / parentDims.
//--------------------------------------------------------------------------------------------------
bool RigNestedHybridGridReconstructor::detectSingleRegion( const std::vector<int>& hostNum,
                                                           const RigMainGrid*      grid,
                                                           RegionInfo*             region,
                                                           QString*                errorMessage )
{
    auto setError = [&]( const QString& msg )
    {
        if ( errorMessage ) *errorMessage = msg;
        RiaLogging::warning( ( "Nested hybrid grid: " + msg ).toStdString() );
        return false;
    };

    const size_t nx = grid->cellCountI();
    const size_t ny = grid->cellCountJ();
    const size_t nz = grid->cellCountK();

    const size_t SMAX = std::numeric_limits<size_t>::max();
    size_t       bi0 = SMAX, bi1 = 0, bj0 = SMAX, bj1 = 0, bk0 = SMAX, bk1 = 0; // refined block
    size_t       pi0 = SMAX, pi1 = 0, pj0 = SMAX, pj1 = 0, pk0 = SMAX, pk1 = 0; // parent region
    size_t       refinedCount = 0;

    for ( size_t k = 0; k < nz; k++ )
    {
        for ( size_t j = 0; j < ny; j++ )
        {
            for ( size_t i = 0; i < nx; i++ )
            {
                size_t flat = naturalIndex( i, j, k, nx, ny );
                int    host = hostNum[flat];
                if ( host <= 0 ) continue;

                refinedCount++;
                bi0 = std::min( bi0, i );
                bi1 = std::max( bi1, i );
                bj0 = std::min( bj0, j );
                bj1 = std::max( bj1, j );
                bk0 = std::min( bk0, k );
                bk1 = std::max( bk1, k );

                size_t parentFlat = (size_t)( host - 1 );
                size_t pi         = parentFlat % nx;
                size_t pj         = ( parentFlat / nx ) % ny;
                size_t pk         = parentFlat / ( nx * ny );
                pi0               = std::min( pi0, pi );
                pi1               = std::max( pi1, pi );
                pj0               = std::min( pj0, pj );
                pj1               = std::max( pj1, pj );
                pk0               = std::min( pk0, pk );
                pk1               = std::max( pk1, pk );
            }
        }
    }

    if ( refinedCount == 0 ) return setError( "No refined cells (HOSTNUM all zero)." );

    cvf::Vec3st blockDims( bi1 - bi0 + 1, bj1 - bj0 + 1, bk1 - bk0 + 1 );
    cvf::Vec3st parentDims( pi1 - pi0 + 1, pj1 - pj0 + 1, pk1 - pk0 + 1 );

    if ( blockDims.x() % parentDims.x() != 0 || blockDims.y() % parentDims.y() != 0 || blockDims.z() % parentDims.z() != 0 )
    {
        return setError( QString( "Refined block %1x%2x%3 is not an integer multiple of parent region %4x%5x%6." )
                             .arg( blockDims.x() )
                             .arg( blockDims.y() )
                             .arg( blockDims.z() )
                             .arg( parentDims.x() )
                             .arg( parentDims.y() )
                             .arg( parentDims.z() ) );
    }

    region->blockOrigin  = cvf::Vec3st( bi0, bj0, bk0 );
    region->blockDims    = blockDims;
    region->parentOrigin = cvf::Vec3st( pi0, pj0, pk0 );
    region->refinement   = cvf::Vec3st( blockDims.x() / parentDims.x(), blockDims.y() / parentDims.y(), blockDims.z() / parentDims.z() );
    return true;
}

//--------------------------------------------------------------------------------------------------
/// Build one RigLocalGrid for the region by appending contiguous cells/nodes to the global arrays,
/// copying the real geometry from the scattered flat refined cells, linking parents, and hiding
/// the original flat cells. Mirrors RicCreateTemporaryLgrFeature::createLgrForGrid.
//--------------------------------------------------------------------------------------------------
RigLocalGrid* RigNestedHybridGridReconstructor::buildLocalGrid( RigEclipseCaseData*  caseData,
                                                                const RegionInfo&    region,
                                                                int                  gridId,
                                                                std::vector<size_t>& lgrToFlatCell )
{
    RigMainGrid* grid = caseData->mainGrid();

    const size_t nx = grid->cellCountI();
    const size_t ny = grid->cellCountJ();

    const size_t Di = region.blockDims.x();
    const size_t Dj = region.blockDims.y();
    const size_t Dk = region.blockDims.z();

    const size_t lgrCellCount = Di * Dj * Dk;
    if ( lgrCellCount == 0 ) return nullptr;

    RigLocalGrid* localGrid = new RigLocalGrid( grid );
    localGrid->setGridId( gridId );
    localGrid->setGridName( "NestedHybridLGR" );
    localGrid->setCellCounts( region.blockDims );
    localGrid->setAsReconstructedGrid( true ); // synthesized in memory, not present in the result file

    const size_t cellStart = grid->totalCellCount();
    const size_t nodeStart = grid->nodes().size();
    localGrid->setIndexToStartOfCells( cellStart );
    grid->addLocalGrid( localGrid );

    {
        RigCell defaultCell;
        defaultCell.setHostGrid( localGrid );
        grid->reservoirCells().resize( cellStart + lgrCellCount, defaultCell );
        grid->nodes().resize( nodeStart + lgrCellCount * 8, cvf::Vec3d( 0, 0, 0 ) );
    }

    // Remember which flat cell each LGR cell was copied from, for active-cell/result transfer.
    lgrToFlatCell.assign( lgrCellCount, cvf::UNDEFINED_SIZE_T );

    size_t glc = 0;
    for ( size_t lk = 0; lk < Dk; lk++ )
    {
        for ( size_t lj = 0; lj < Dj; lj++ )
        {
            for ( size_t li = 0; li < Di; li++, glc++ )
            {
                size_t flatI = region.blockOrigin.x() + li;
                size_t flatJ = region.blockOrigin.y() + lj;
                size_t flatK = region.blockOrigin.z() + lk;
                size_t flat  = naturalIndex( flatI, flatJ, flatK, nx, ny );

                size_t pi         = region.parentOrigin.x() + li / region.refinement.x();
                size_t pj         = region.parentOrigin.y() + lj / region.refinement.y();
                size_t pk         = region.parentOrigin.z() + lk / region.refinement.z();
                size_t parentFlat = naturalIndex( pi, pj, pk, nx, ny );

                RigCell& parentCell = grid->cell( parentFlat );
                parentCell.setSubGrid( localGrid );

                RigCell& flatCell = grid->cell( flat );
                RigCell& lgrCell  = grid->cell( cellStart + glc );
                lgrCell.setHostGrid( localGrid );
                lgrCell.setGridLocalCellIndex( glc );
                lgrCell.setParentCellIndex( parentFlat );

                // Copy the real corner geometry from the scattered flat refined cell.
                for ( size_t c = 0; c < 8; c++ )
                {
                    size_t newNodeIdx          = nodeStart + glc * 8 + c;
                    grid->nodes()[newNodeIdx]  = grid->nodes()[flatCell.cornerIndices()[c]];
                    lgrCell.cornerIndices()[c] = newNodeIdx;
                }
                lgrCell.setInvalid( flatCell.isInvalid() );

                // Hide the original scattered cell - the LGR cell now represents it.
                flatCell.setInvalid( true );

                lgrToFlatCell[glc] = flat;
            }
        }
    }

    localGrid->setParentGrid( grid );

    return localGrid;
}

//--------------------------------------------------------------------------------------------------
/// Register the LGR cells as additional active cells, for both porosity models. Each active LGR
/// cell gets a NEW result index appended after the existing active cells, which preserves the
/// invariant "result index == position in the active-cell list" that result calculators rely on
/// (e.g. INDEX_I/J/K). The original flat refined cells are kept active (but hidden), so the file
/// reader's active-cell ordering still matches; the LGR cells' result values are copied from the
/// source flat cells on each load (see RigCaseCellResultsData / nestedHybridLgrSourceCells).
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridReconstructor::updateActiveCellInfo( RigEclipseCaseData*        caseData,
                                                             const RigLocalGrid*        localGrid,
                                                             const std::vector<size_t>& lgrToFlatCell )
{
    RigMainGrid* grid         = caseData->mainGrid();
    const size_t cellStart    = localGrid->reservoirCellIndex( 0 );
    const size_t lgrCellCount = localGrid->cellCount();
    const size_t newGridIndex = grid->gridCount() - 1; // the LGR just added is the last grid

    const std::array<RiaDefines::PorosityModelType, 2> models = { RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                  RiaDefines::PorosityModelType::FRACTURE_MODEL };

    for ( auto model : models )
    {
        RigActiveCellInfo* actInfo = caseData->activeCellInfo( model );
        if ( !actInfo ) continue;

        // Skip porosity models that are not in use (e.g. the fracture model in a single-porosity
        // case). Adding cells to an unused model makes its active count non-zero, which makes the
        // result reader treat the case as dual-porosity and mis-split every dynamic result.
        const size_t oldActiveCellCount = actInfo->reservoirActiveCellCount();
        if ( oldActiveCellCount == 0 ) continue;

        const size_t oldReservoirCellCount = actInfo->reservoirCellCount();
        actInfo->setReservoirCellCount( oldReservoirCellCount + lgrCellCount );
        actInfo->setGridCount( grid->gridCount() );

        size_t lgrActiveCount = 0;
        for ( size_t glc = 0; glc < lgrCellCount; glc++ )
        {
            size_t flat = lgrToFlatCell[glc];
            if ( flat == cvf::UNDEFINED_SIZE_T ) continue;
            if ( !actInfo->isActive( ReservoirCellIndex( flat ) ) ) continue;

            // Distinct, appended result index - keeps result index == active-list position.
            actInfo->setCellResultIndex( ReservoirCellIndex( cellStart + glc ), ActiveCellIndex( oldActiveCellCount + lgrActiveCount ) );
            lgrActiveCount++;
        }

        actInfo->setGridActiveCellCounts( newGridIndex, lgrActiveCount );
        actInfo->computeDerivedData();
    }
}

//--------------------------------------------------------------------------------------------------
/// Full-length (all-cells) result arrays are indexed by global reservoir cell index, so the new
/// LGR cells (appended at the end) need their own entries. Copy each LGR cell's value from the
/// source flat cell it was built from. Active-cell-indexed results are addressed via the result
/// index that was already transferred in updateActiveCellInfo, so they are left untouched.
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridReconstructor::extendFullLengthResults( RigEclipseCaseData*        caseData,
                                                                size_t                     lgrCellStart,
                                                                const std::vector<size_t>& lgrToFlatCell )
{
    const size_t lgrCellCount = lgrToFlatCell.size();
    const size_t newTotal     = lgrCellStart + lgrCellCount;

    const std::array<RiaDefines::PorosityModelType, 2> models = { RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                  RiaDefines::PorosityModelType::FRACTURE_MODEL };

    for ( auto model : models )
    {
        RigCaseCellResultsData* results = caseData->results( model );
        if ( !results ) continue;

        for ( const RigEclipseResultAddress& addr : results->existingResults() )
        {
            std::vector<std::vector<double>>* timesteps = results->modifiableCellScalarResultTimesteps( addr );
            if ( !timesteps ) continue;

            for ( std::vector<double>& values : *timesteps )
            {
                // Only full-length (all-cells) arrays need extending: their length equals the
                // pre-reconstruction cell count. Active-cell-indexed arrays have the (smaller)
                // active-cell length and are handled via the transferred result index.
                if ( values.size() != lgrCellStart ) continue;

                values.resize( newTotal, HUGE_VAL );
                for ( size_t glc = 0; glc < lgrCellCount; glc++ )
                {
                    size_t flat = lgrToFlatCell[glc];
                    if ( flat != cvf::UNDEFINED_SIZE_T && flat < lgrCellStart )
                    {
                        values[lgrCellStart + glc] = values[flat];
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// The file NNCs connect coarse cells to the original scattered refined cells. After those cells
/// were moved into the LGR and hidden, re-point each affected connection to the corresponding LGR
/// cell. Connection result data (transmissibilities) and polygons are indexed/located by position
/// and geometry, both unchanged, so only the cell indices need updating.
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridReconstructor::repointNncConnections( RigEclipseCaseData*        caseData,
                                                              size_t                     lgrCellStart,
                                                              const std::vector<size_t>& lgrToFlatCell )
{
    RigMainGrid* grid = caseData->mainGrid();
    if ( !grid ) return;

    RigNNCData* nncData = grid->nncData();
    if ( !nncData ) return;

    // Inverse map: original flat cell index -> new LGR global cell index.
    std::vector<size_t> flatToLgr( lgrCellStart, cvf::UNDEFINED_SIZE_T );
    for ( size_t glc = 0; glc < lgrToFlatCell.size(); glc++ )
    {
        size_t flat = lgrToFlatCell[glc];
        if ( flat != cvf::UNDEFINED_SIZE_T && flat < lgrCellStart )
        {
            flatToLgr[flat] = lgrCellStart + glc;
        }
    }

    auto mapped = [&]( size_t idx ) { return ( idx < lgrCellStart && flatToLgr[idx] != cvf::UNDEFINED_SIZE_T ) ? flatToLgr[idx] : idx; };

    RigConnectionContainer& connections = nncData->allConnections();
    for ( size_t i = 0; i < connections.size(); i++ )
    {
        RigConnection& conn = connections[i];
        size_t         a    = conn.c1GlobIdx();
        size_t         b    = conn.c2GlobIdx();
        size_t         na   = mapped( a );
        size_t         nb   = mapped( b );
        if ( na != a || nb != b )
        {
            connections[i] = RigConnection( na, nb, conn.face(), conn.polygon() );
        }
    }
}
