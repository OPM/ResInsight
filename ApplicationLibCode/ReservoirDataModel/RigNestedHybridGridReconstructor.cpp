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
#include "RigGridBase.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNestedHybridGridResultTools.h"
#include "RigNncConnection.h"

#include "RiaDefines.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <map>
#include <set>

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
bool RigNestedHybridGridReconstructor::reconstruct( RigEclipseCaseData* caseData, const NestedHybridInput& input, QString* errorMessage )
{
    auto setError = [&]( const QString& msg )
    {
        if ( errorMessage ) *errorMessage = msg;
        RiaLogging::warning( ( "Nested hybrid grid: " + msg ).toStdString() );
        return false;
    };

    if ( !caseData || !caseData->mainGrid() ) return setError( "No grid data." );

    RigMainGrid* grid      = caseData->mainGrid();
    const size_t cellCount = grid->cellCount();

    if ( input.refine.size() != cellCount )
    {
        return setError( QString( "REFINE size %1 does not match main grid cell count %2." ).arg( input.refine.size() ).arg( cellCount ) );
    }
    if ( input.oldI.size() != cellCount || input.oldJ.size() != cellCount || input.oldK.size() != cellCount )
    {
        return setError( "OLDI/OLDJ/OLDK size does not match main grid cell count." );
    }
    if ( grid->gridCount() > 1 )
    {
        return setError( "Grid already contains local grids; skipping reconstruction." );
    }

    const size_t nx = grid->cellCountI();
    const size_t ny = grid->cellCountJ();
    const size_t nz = grid->cellCountK();

    // Coarse grid dimensions are the maximum 1-based OLD indices. The flat grid embeds each coarse
    // cell at (OLDI-1, OLDJ-1, (OLDK-1)*KF), where KF is the global K refinement factor.
    int coarseNx = 0, coarseNy = 0, coarseNz = 0;
    for ( size_t f = 0; f < cellCount; f++ )
    {
        coarseNx = std::max( coarseNx, input.oldI[f] );
        coarseNy = std::max( coarseNy, input.oldJ[f] );
        coarseNz = std::max( coarseNz, input.oldK[f] );
    }
    const size_t kFactor = ( coarseNz > 0 && nz % (size_t)coarseNz == 0 ) ? nz / (size_t)coarseNz : 1;

    // Group all refined cells (REFINE > 1) by level, and record each refined cell's parent COARSE
    // cell natural index (for the volume-weighted QC aggregate).
    std::map<int, std::vector<size_t>> cellsByLevel;
    std::map<size_t, size_t>           coarseParents; // flat cell -> coarse natural index
    for ( size_t f = 0; f < cellCount; f++ )
    {
        const int level = input.refine[f];
        if ( level <= 1 ) continue;
        const int oi = input.oldI[f];
        const int oj = input.oldJ[f];
        const int ok = input.oldK[f];
        if ( oi < 1 || oj < 1 || ok < 1 ) continue; // padding / unmapped

        cellsByLevel[level].push_back( f );
        coarseParents[f] = naturalIndex( (size_t)( oi - 1 ), (size_t)( oj - 1 ), (size_t)( ok - 1 ), (size_t)coarseNx, (size_t)coarseNy );
    }

    if ( cellsByLevel.empty() ) return setError( "No refined cells (REFINE all <= 1)." );

    const size_t             origCellCount = grid->totalCellCount(); // before any LGR cells are appended
    std::map<size_t, size_t> sourceCells; // LGR global cell index -> source flat cell index
    int                      nextGridId = (int)grid->gridCount(); // main grid is 0

    using CoarseIjk = std::array<int, 3>;
    using Factor    = std::array<size_t, 3>;

    // Build every refinement level directly below the main grid. OLDIJK identifies the coarse parent,
    // while the distinct flat-grid coordinates within each parent determine the local cell ordering.
    // Adjacent parents with equal refinement dimensions are combined into one regular LGR.
    for ( const auto& [level, cells] : cellsByLevel )
    {
        std::map<CoarseIjk, std::vector<size_t>> cellsByParent;
        for ( size_t f : cells )
            cellsByParent[{ input.oldI[f], input.oldJ[f], input.oldK[f] }].push_back( f );

        std::map<CoarseIjk, std::array<std::vector<size_t>, 3>> coordinatesByParent;
        std::map<Factor, std::set<CoarseIjk>>                   parentsByFactor;
        for ( const auto& [parent, parentCells] : cellsByParent )
        {
            auto& coordinates = coordinatesByParent[parent];
            for ( size_t f : parentCells )
            {
                const std::array<size_t, 3> flatIjk = { f % nx, ( f / nx ) % ny, f / ( nx * ny ) };
                for ( int axis = 0; axis < 3; axis++ )
                    coordinates[axis].push_back( flatIjk[axis] );
            }

            Factor factor;
            for ( int axis = 0; axis < 3; axis++ )
            {
                auto& values = coordinates[axis];
                std::sort( values.begin(), values.end() );
                values.erase( std::unique( values.begin(), values.end() ), values.end() );
                factor[axis] = values.size();
            }
            parentsByFactor[factor].insert( parent );
        }

        // Collect the connected components first so the component count is known before naming.
        std::vector<std::pair<Factor, std::vector<CoarseIjk>>> components;
        for ( const auto& [factor, parents] : parentsByFactor )
        {
            std::set<CoarseIjk> remaining = parents;
            while ( !remaining.empty() )
            {
                std::vector<CoarseIjk> component;
                std::vector<CoarseIjk> stack = { *remaining.begin() };
                remaining.erase( stack.front() );

                while ( !stack.empty() )
                {
                    const CoarseIjk parent = stack.back();
                    stack.pop_back();
                    component.push_back( parent );

                    for ( int axis = 0; axis < 3; axis++ )
                        for ( int direction : { -1, 1 } )
                        {
                            CoarseIjk neighbour = parent;
                            neighbour[axis] += direction;
                            auto it = remaining.find( neighbour );
                            if ( it == remaining.end() ) continue;
                            stack.push_back( *it );
                            remaining.erase( it );
                        }
                }

                components.push_back( { factor, std::move( component ) } );
            }
        }

        for ( size_t componentIndex = 0; componentIndex < components.size(); componentIndex++ )
        {
            const auto& [factor, component] = components[componentIndex];

            CoarseIjk c0 = { INT_MAX, INT_MAX, INT_MAX };
            CoarseIjk c1 = { 0, 0, 0 };
            for ( const CoarseIjk& parent : component )
                for ( int axis = 0; axis < 3; axis++ )
                {
                    c0[axis] = std::min( c0[axis], parent[axis] );
                    c1[axis] = std::max( c1[axis], parent[axis] );
                }

            const cvf::Vec3st   dims( (size_t)( c1[0] - c0[0] + 1 ) * factor[0],
                                    (size_t)( c1[1] - c0[1] + 1 ) * factor[1],
                                    (size_t)( c1[2] - c0[2] + 1 ) * factor[2] );
            std::vector<size_t> boxToParent( dims.x() * dims.y() * dims.z(), cvf::UNDEFINED_SIZE_T );
            for ( size_t lk = 0; lk < dims.z(); lk++ )
                for ( size_t lj = 0; lj < dims.y(); lj++ )
                    for ( size_t li = 0; li < dims.x(); li++ )
                    {
                        size_t local       = naturalIndex( li, lj, lk, dims.x(), dims.y() );
                        size_t oi          = (size_t)c0[0] + li / factor[0];
                        size_t oj          = (size_t)c0[1] + lj / factor[1];
                        size_t ok          = (size_t)c0[2] + lk / factor[2];
                        boxToParent[local] = naturalIndex( oi - 1, oj - 1, ( ok - 1 ) * kFactor, nx, ny );
                    }

            std::vector<size_t> boxToFlat( dims.x() * dims.y() * dims.z(), cvf::UNDEFINED_SIZE_T );
            for ( const CoarseIjk& parent : component )
            {
                const auto& coordinates = coordinatesByParent[parent];
                for ( size_t f : cellsByParent[parent] )
                {
                    const std::array<size_t, 3> flatIjk = { f % nx, ( f / nx ) % ny, f / ( nx * ny ) };
                    size_t                      localIjk[3];
                    for ( int axis = 0; axis < 3; axis++ )
                    {
                        const auto coordinateIt = std::lower_bound( coordinates[axis].begin(), coordinates[axis].end(), flatIjk[axis] );
                        localIjk[axis]          = (size_t)( parent[axis] - c0[axis] ) * factor[axis] +
                                         (size_t)( coordinateIt - coordinates[axis].begin() );
                    }
                    boxToFlat[naturalIndex( localIjk[0], localIjk[1], localIjk[2], dims.x(), dims.y() )] = f;
                }
            }

            // Single component on a level keeps the bare level name; several get 1-based suffixes.
            QString gridName = QString( "LGR_NHG_L%1" ).arg( level );
            if ( components.size() > 1 ) gridName += QString( "_%1" ).arg( componentIndex + 1 );
            buildLocalGrid( caseData, grid, nextGridId++, gridName, dims, boxToFlat, boxToParent, sourceCells );
        }
    }

    if ( sourceCells.empty() ) return setError( "No refined regions could be reconstructed." );

    grid->setNestedHybridLgrSourceCells( sourceCells );
    grid->setNestedHybridCoarseParents( coarseParents );

    updateActiveCellInfo( caseData, sourceCells );
    extendFullLengthResults( caseData, origCellCount, sourceCells );
    repointNncConnections( caseData, origCellCount, sourceCells );

    // Fill LGR cells of any already-loaded active-cell-indexed results from their source flat cells.
    for ( auto model : { RiaDefines::PorosityModelType::MATRIX_MODEL, RiaDefines::PorosityModelType::FRACTURE_MODEL } )
    {
        if ( RigCaseCellResultsData* results = caseData->results( model ) ) RigNestedHybridGridResultTools::extendLgrResults( results );
    }

    // The grid count changed - invalidate per-grid caches that were sized for the flat grid.
    caseData->clearWellCellsInGridCache();

    RiaLogging::info(
        QString( "Nested hybrid grid: created %1 LGRs (%2 cells)" ).arg( grid->gridCount() - 1 ).arg( sourceCells.size() ).toStdString() );

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Build one RigLocalGrid by appending contiguous cells/nodes to the global arrays, copying the real
/// geometry from the scattered flat refined cells, hiding the originals, and linking every LGR cell
/// to the parent (coarse) cell it subdivides. boxToFlat maps each LGR local cell to its source flat
/// cell (UNDEFINED for holes); boxToParent maps it to its parent cell. The LGR may span many parent
/// cells; each parent owning a real cell gets its subGrid set. Mirrors RicCreateTemporaryLgrFeature.
//--------------------------------------------------------------------------------------------------
RigLocalGrid* RigNestedHybridGridReconstructor::buildLocalGrid( RigEclipseCaseData*        caseData,
                                                                RigGridBase*               parentGrid,
                                                                int                        gridId,
                                                                const QString&             gridName,
                                                                const cvf::Vec3st&         dims,
                                                                const std::vector<size_t>& boxToFlat,
                                                                const std::vector<size_t>& boxToParent,
                                                                std::map<size_t, size_t>&  sourceCells )
{
    RigMainGrid* grid = caseData->mainGrid();

    const size_t lgrCellCount = dims.x() * dims.y() * dims.z();
    if ( lgrCellCount == 0 ) return nullptr;

    RigLocalGrid* localGrid = new RigLocalGrid( grid );
    localGrid->setGridId( gridId );
    localGrid->setGridName( gridName.toStdString() );
    localGrid->setCellCounts( dims );
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

    for ( size_t glc = 0; glc < lgrCellCount; glc++ )
    {
        RigCell& lgrCell = grid->cell( cellStart + glc );
        lgrCell.setHostGrid( localGrid );
        lgrCell.setGridLocalCellIndex( glc );

        // boxToParent holds the parent cell index LOCAL to parentGrid; setParentCellIndex expects the
        // parent-grid-local index, while subGrid is set on the parent's global reservoir cell.
        const size_t parentLocal  = boxToParent[glc];
        const size_t parentGlobal = parentLocal != cvf::UNDEFINED_SIZE_T ? parentGrid->reservoirCellIndex( parentLocal )
                                                                         : cvf::UNDEFINED_SIZE_T;
        lgrCell.setParentCellIndex( parentLocal );

        const size_t flat = boxToFlat[glc];
        if ( flat == cvf::UNDEFINED_SIZE_T )
        {
            lgrCell.setInvalid( true ); // hole - keep the regular IJK box but no geometry
            continue;
        }

        RigCell& flatCell = grid->cell( flat );

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

        // This parent cell is really refined by this LGR.
        if ( parentGlobal != cvf::UNDEFINED_SIZE_T )
        {
            grid->cell( parentGlobal ).setSubGrid( localGrid );
        }

        sourceCells[cellStart + glc] = flat;
    }

    localGrid->setParentGrid( parentGrid );

    return localGrid;
}

//--------------------------------------------------------------------------------------------------
/// Register all appended LGR cells as additional active cells, for both porosity models. Each active
/// LGR cell gets a NEW result index appended after the existing active cells, which preserves the
/// invariant "result index == position in the active-cell list" that result calculators rely on.
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridReconstructor::updateActiveCellInfo( RigEclipseCaseData* caseData, const std::map<size_t, size_t>& sourceCells )
{
    RigMainGrid* grid = caseData->mainGrid();

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

        actInfo->setReservoirCellCount( grid->totalCellCount() );
        actInfo->setGridCount( grid->gridCount() );

        size_t nextActiveIndex = oldActiveCellCount;
        for ( size_t gridIdx = 1; gridIdx < grid->gridCount(); gridIdx++ )
        {
            RigGridBase* lgr            = grid->gridByIndex( gridIdx );
            size_t       lgrActiveCount = 0;
            for ( size_t local = 0; local < lgr->cellCount(); local++ )
            {
                size_t global = lgr->reservoirCellIndex( local );
                auto   it     = sourceCells.find( global );
                if ( it == sourceCells.end() ) continue;
                if ( !actInfo->isActive( ReservoirCellIndex( it->second ) ) ) continue;

                actInfo->setCellResultIndex( ReservoirCellIndex( global ), ActiveCellIndex( nextActiveIndex ) );
                nextActiveIndex++;
                lgrActiveCount++;
            }
            actInfo->setGridActiveCellCounts( gridIdx, lgrActiveCount );
        }

        actInfo->computeDerivedData();
    }
}

//--------------------------------------------------------------------------------------------------
/// Full-length (all-cells) result arrays are indexed by global reservoir cell index, so the new LGR
/// cells (appended at the end) need their own entries. Copy each LGR cell's value from the source
/// flat cell it was built from. Active-cell-indexed results are addressed via the result index that
/// was already transferred in updateActiveCellInfo, so they are left untouched.
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridReconstructor::extendFullLengthResults( RigEclipseCaseData*             caseData,
                                                                size_t                          origCellCount,
                                                                const std::map<size_t, size_t>& sourceCells )
{
    const size_t newTotal = caseData->mainGrid()->totalCellCount();

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
                if ( values.size() != origCellCount ) continue;

                values.resize( newTotal, HUGE_VAL );
                for ( const auto& [global, flat] : sourceCells )
                {
                    if ( flat < origCellCount ) values[global] = values[flat];
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
void RigNestedHybridGridReconstructor::repointNncConnections( RigEclipseCaseData*             caseData,
                                                              size_t                          origCellCount,
                                                              const std::map<size_t, size_t>& sourceCells )
{
    RigMainGrid* grid = caseData->mainGrid();
    if ( !grid ) return;

    RigNNCData* nncData = grid->nncData();
    if ( !nncData ) return;

    // Inverse map: original flat cell index -> new LGR global cell index.
    std::vector<size_t> flatToLgr( origCellCount, cvf::UNDEFINED_SIZE_T );
    for ( const auto& [global, flat] : sourceCells )
    {
        if ( flat < origCellCount ) flatToLgr[flat] = global;
    }

    auto mapped = [&]( size_t idx ) { return ( idx < origCellCount && flatToLgr[idx] != cvf::UNDEFINED_SIZE_T ) ? flatToLgr[idx] : idx; };

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
