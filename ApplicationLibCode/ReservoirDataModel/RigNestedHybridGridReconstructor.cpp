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
#include <iterator>
#include <map>
#include <tuple>

namespace
{
size_t naturalIndex( size_t i, size_t j, size_t k, size_t nx, size_t ny )
{
    return i + j * nx + k * nx * ny;
}

// A reconstructed cell, recorded so that a deeper (nested) refinement level can find it as a parent.
// Keyed by the cell's TMP: a nested cell carries its immediate parent's TMP, so a child looks up its
// own TMP among the cells one level shallower.
struct BuiltRef
{
    RigGridBase* grid      = nullptr;
    size_t       localCell = 0;
    int          level     = 0;
};
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
    const bool haveTmp = input.tmpI.size() == cellCount && input.tmpJ.size() == cellCount && input.tmpK.size() == cellCount;
    if ( !haveTmp ) return setError( "TMPI/TMPJ/TMPK size does not match main grid cell count." );

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
    size_t                   deferred   = 0;

    // Index every reconstructed cell by its TMP, so a deeper level can find its immediate parent.
    int tmpMax[3] = { 0, 0, 0 };
    for ( const auto& [level, cells] : cellsByLevel )
        for ( size_t f : cells )
        {
            tmpMax[0] = std::max( tmpMax[0], input.tmpI[f] );
            tmpMax[1] = std::max( tmpMax[1], input.tmpJ[f] );
            tmpMax[2] = std::max( tmpMax[2], input.tmpK[f] );
        }
    const size_t tmpDimX = (size_t)tmpMax[0] + 1;
    const size_t tmpDimY = (size_t)tmpMax[1] + 1;
    auto         tmpKey  = [&]( size_t f ) { return naturalIndex( input.tmpI[f], input.tmpJ[f], input.tmpK[f], tmpDimX, tmpDimY ); };

    std::map<size_t, std::vector<BuiltRef>> tmpToBuilt;

    // Record the cells appended since beforeTotal (the just-built level), keyed by their TMP.
    auto recordBuilt = [&]( size_t beforeTotal, int level )
    {
        for ( size_t gc = beforeTotal; gc < grid->totalCellCount(); gc++ )
        {
            auto it = sourceCells.find( gc );
            if ( it == sourceCells.end() ) continue; // hole
            size_t       localCell = 0;
            RigGridBase* cellGrid  = grid->gridAndGridLocalIdxFromGlobalCellIdx( gc, &localCell );
            tmpToBuilt[tmpKey( it->second )].push_back( { cellGrid, localCell, level } );
        }
    };

    // Build refinement levels from shallow to deep. For each level, first try to resolve every cell's
    // immediate parent among the already-built cells one level shallower (true nesting, any depth);
    // otherwise treat the level as a direct, uniform refinement of the coarse grid. Cells whose parent
    // cannot be resolved unambiguously are deferred (left as flat cells) rather than mis-nested.
    for ( const auto& [level, cells] : cellsByLevel )
    {
        // Try to resolve each cell's immediate parent (a unique built cell one level shallower whose
        // TMP equals this cell's TMP).
        std::map<size_t, std::pair<RigGridBase*, size_t>> cellToParent; // flat cell -> (parent grid, parent local cell)
        for ( size_t f : cells )
        {
            auto it = tmpToBuilt.find( tmpKey( f ) );
            if ( it == tmpToBuilt.end() ) continue;
            const BuiltRef* unique = nullptr;
            for ( const BuiltRef& b : it->second )
            {
                if ( b.level != level - 1 ) continue;
                if ( unique ) // more than one candidate -> ambiguous
                {
                    unique = nullptr;
                    break;
                }
                unique = &b;
            }
            if ( unique ) cellToParent[f] = { unique->grid, unique->localCell };
        }

        const size_t beforeTotal = grid->totalCellCount();

        if ( cellToParent.size() >= cells.size() * 95 / 100 && !cellToParent.empty() )
        {
            // Nested level: build LGR(s) inside the resolved parent grid(s).
            deferred += cells.size() - cellToParent.size(); // unresolved cells are left as flat cells
            buildNestedLevel( caseData, cells, input, cellToParent, nextGridId, sourceCells );
            recordBuilt( beforeTotal, level );
            continue;
        }

        // Primary level: a direct, uniform refinement of the coarse grid.
        int c0[3] = { INT_MAX, INT_MAX, INT_MAX }, c1[3] = { 0, 0, 0 };
        int t0[3] = { INT_MAX, INT_MAX, INT_MAX }, t1[3] = { 0, 0, 0 };
        for ( size_t f : cells )
        {
            const int t[3] = { input.tmpI[f], input.tmpJ[f], input.tmpK[f] };
            const int c[3] = { input.oldI[f], input.oldJ[f], input.oldK[f] };
            for ( int a = 0; a < 3; a++ )
            {
                t0[a] = std::min( t0[a], t[a] );
                t1[a] = std::max( t1[a], t[a] );
                c0[a] = std::min( c0[a], c[a] );
                c1[a] = std::max( c1[a], c[a] );
            }
        }

        const int coarseDim[3] = { c1[0] - c0[0] + 1, c1[1] - c0[1] + 1, c1[2] - c0[2] + 1 };
        const int tmpDim[3]    = { t1[0] - t0[0] + 1, t1[1] - t0[1] + 1, t1[2] - t0[2] + 1 };

        int factor[3];
        for ( int a = 0; a < 3; a++ )
            factor[a] = std::max( 1, (int)std::lround( (double)tmpDim[a] / (double)coarseDim[a] ) );

        // Verify the level is a uniform refinement of the coarse grid: the coarse cell implied by each
        // cell's TMP (via the factor) must equal its OLD index. If not, the level cannot be placed and
        // is deferred (left as flat cells).
        size_t matched = 0;
        for ( size_t f : cells )
        {
            const int pi = c0[0] + ( input.tmpI[f] - t0[0] ) / factor[0];
            const int pj = c0[1] + ( input.tmpJ[f] - t0[1] ) / factor[1];
            const int pk = c0[2] + ( input.tmpK[f] - t0[2] ) / factor[2];
            if ( pi == input.oldI[f] && pj == input.oldJ[f] && pk == input.oldK[f] ) matched++;
        }
        if ( matched < cells.size() * 95 / 100 )
        {
            deferred += cells.size();
            continue;
        }

        const cvf::Vec3st dims( (size_t)coarseDim[0] * factor[0], (size_t)coarseDim[1] * factor[1], (size_t)coarseDim[2] * factor[2] );

        // Map each LGR local cell to its parent (collapsed) coarse cell in the flat grid, and to its
        // source flat cell (UNDEFINED = hole).
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
        for ( size_t f : cells )
        {
            size_t li        = input.tmpI[f] - t0[0];
            size_t lj        = input.tmpJ[f] - t0[1];
            size_t lk        = input.tmpK[f] - t0[2];
            size_t local     = naturalIndex( li, lj, lk, dims.x(), dims.y() );
            boxToFlat[local] = f;
        }

        const QString gridName  = QString( "LGR_NHG_L%1" ).arg( level );
        RigLocalGrid* levelGrid = buildLocalGrid( caseData, grid, nextGridId++, gridName, dims, boxToFlat, boxToParent, sourceCells );

        // Record EVERY cell of this primary level (including holes), keyed by its TMP slot, so a deeper
        // level can resolve its parent here - the parent is often a hole cell (one that was further
        // refined and so has no geometry of its own until a child supplies it).
        for ( size_t lk = 0; lk < dims.z(); lk++ )
            for ( size_t lj = 0; lj < dims.y(); lj++ )
                for ( size_t li = 0; li < dims.x(); li++ )
                {
                    size_t local = naturalIndex( li, lj, lk, dims.x(), dims.y() );
                    size_t ti = (size_t)t0[0] + li, tj = (size_t)t0[1] + lj, tk = (size_t)t0[2] + lk;
                    tmpToBuilt[naturalIndex( ti, tj, tk, tmpDimX, tmpDimY )].push_back( { levelGrid, local, level } );
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

    RiaLogging::info( QString( "Nested hybrid grid: created %1 LGRs (%2 cells)%3" )
                          .arg( grid->gridCount() - 1 )
                          .arg( sourceCells.size() )
                          .arg( deferred > 0 ? QString( ", %1 nested-level cells left un-nested" ).arg( deferred ) : QString() )
                          .toStdString() );

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
                                                                std::map<size_t, size_t>&  sourceCells,
                                                                bool                       synthesizeHoleParentGeometry )
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

    // Track the child cells of each refined parent (global indices), for parent geometry synthesis.
    std::map<size_t, std::vector<size_t>> childrenByParentGlobal;

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
            childrenByParentGlobal[parentGlobal].push_back( cellStart + glc );
        }

        sourceCells[cellStart + glc] = flat;
    }

    localGrid->setParentGrid( parentGrid );

    // For nested LGRs the parent cell may itself be a hole in its (parent) grid - i.e. a coarser cell
    // that was subdivided and therefore has no geometry. Give it an axis-aligned bounding-box geometry
    // enclosing its children so picking / bounding boxes behave.
    if ( synthesizeHoleParentGeometry )
    {
        for ( const auto& [parentGlobal, children] : childrenByParentGlobal )
        {
            RigCell& parentCell = grid->cell( parentGlobal );
            if ( !parentCell.isInvalid() ) continue;

            cvf::Vec3d mn( HUGE_VAL, HUGE_VAL, HUGE_VAL ), mx( -HUGE_VAL, -HUGE_VAL, -HUGE_VAL );
            for ( size_t child : children )
            {
                for ( size_t c = 0; c < 8; c++ )
                {
                    const cvf::Vec3d& v = grid->nodes()[grid->cell( child ).cornerIndices()[c]];
                    mn.x()              = std::min( mn.x(), v.x() );
                    mn.y()              = std::min( mn.y(), v.y() );
                    mn.z()              = std::min( mn.z(), v.z() );
                    mx.x()              = std::max( mx.x(), v.x() );
                    mx.y()              = std::max( mx.y(), v.y() );
                    mx.z()              = std::max( mx.z(), v.z() );
                }
            }

            const std::array<cvf::Vec3d, 8> corners = { cvf::Vec3d( mn.x(), mn.y(), mn.z() ),
                                                        cvf::Vec3d( mx.x(), mn.y(), mn.z() ),
                                                        cvf::Vec3d( mx.x(), mx.y(), mn.z() ),
                                                        cvf::Vec3d( mn.x(), mx.y(), mn.z() ),
                                                        cvf::Vec3d( mn.x(), mn.y(), mx.z() ),
                                                        cvf::Vec3d( mx.x(), mn.y(), mx.z() ),
                                                        cvf::Vec3d( mx.x(), mx.y(), mx.z() ),
                                                        cvf::Vec3d( mn.x(), mx.y(), mx.z() ) };

            const size_t base = grid->nodes().size();
            grid->nodes().resize( base + 8, cvf::Vec3d( 0, 0, 0 ) );
            for ( size_t c = 0; c < 8; c++ )
            {
                grid->nodes()[base + c]       = corners[c];
                parentCell.cornerIndices()[c] = base + c;
            }
            parentCell.setInvalid( false );
        }
    }

    return localGrid;
}

//--------------------------------------------------------------------------------------------------
/// Build the nested LGR(s) for a level that refines another (parent) level. Each cell's parent cell is
/// (TMP - parentTmpOrigin) and its position within that parent cell comes from the flat-IJK sub-block.
/// The cells are merged into connected regions in the refined parent coordinate space, so a stack of
/// refined parent cells (e.g. across K) becomes a single LGR rather than one LGR per parent cell. One
/// compact LGR is built per connected region, placed inside parentGrid. Returns the number of cells
/// that could not be nested.
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridReconstructor::buildNestedLevel( RigEclipseCaseData*                                      caseData,
                                                         const std::vector<size_t>&                               cells,
                                                         const NestedHybridInput&                                 input,
                                                         const std::map<size_t, std::pair<RigGridBase*, size_t>>& cellToParent,
                                                         int&                                                     nextGridId,
                                                         std::map<size_t, size_t>&                                sourceCells )
{
    if ( cells.empty() || cellToParent.empty() ) return;

    RigMainGrid* grid  = caseData->mainGrid();
    const size_t nx    = grid->cellCountI();
    const size_t ny    = grid->cellCountJ();
    const int    level = input.refine[cells.front()];

    // Group the resolved cells by their parent grid (a nested level may refine cells in several parent
    // LGRs); each LGR can only refine cells of a single parent grid.
    std::map<RigGridBase*, std::vector<size_t>> byParentGrid;
    for ( size_t f : cells )
    {
        auto it = cellToParent.find( f );
        if ( it != cellToParent.end() ) byParentGrid[it->second.first].push_back( f );
    }

    int component = 0;
    for ( const auto& [parentGrid, gcells] : byParentGrid )
    {
        const size_t pDimX = parentGrid->cellCountI();
        const size_t pDimY = parentGrid->cellCountJ();

        // Parent-grid-local IJK of a cell's parent cell.
        auto parentIjk = [&]( size_t f, size_t ijk[3] )
        {
            size_t pl = cellToParent.at( f ).second;
            ijk[0]    = pl % pDimX;
            ijk[1]    = ( pl / pDimX ) % pDimY;
            ijk[2]    = pl / ( pDimX * pDimY );
        };

        // Per parent cell, the min flat-IJK of its children, and the per-axis sub-refinement factor.
        std::map<size_t, std::array<size_t, 3>> flatOrigin;
        std::map<size_t, std::array<size_t, 3>> flatExtent;
        for ( size_t f : gcells )
        {
            size_t ijk[3];
            parentIjk( f, ijk );
            size_t                plocal = naturalIndex( ijk[0], ijk[1], ijk[2], pDimX, pDimY );
            std::array<size_t, 3> fxyz   = { f % nx, ( f / nx ) % ny, f / ( nx * ny ) };
            auto                  oit    = flatOrigin.find( plocal );
            if ( oit == flatOrigin.end() )
            {
                flatOrigin[plocal] = fxyz;
                flatExtent[plocal] = fxyz;
            }
            else
            {
                for ( int a = 0; a < 3; a++ )
                {
                    oit->second[a]        = std::min( oit->second[a], fxyz[a] );
                    flatExtent[plocal][a] = std::max( flatExtent[plocal][a], fxyz[a] );
                }
            }
        }

        size_t sub[3] = { 1, 1, 1 };
        for ( const auto& [plocal, mn] : flatOrigin )
            for ( int a = 0; a < 3; a++ )
                sub[a] = std::max( sub[a], flatExtent[plocal][a] - mn[a] + 1 );

        // Position of each nested cell in the refined parent coordinate space:
        //   q = parentLocalIjk * sub + subPos   (subPos = flat IJK - the parent cell's flat-IJK origin)
        const size_t                       qDimX = pDimX * sub[0];
        const size_t                       qDimY = pDimY * sub[1];
        std::vector<std::array<size_t, 3>> qOfCell( gcells.size() );
        std::map<size_t, size_t>           qToCell; // q linear index -> index into gcells
        for ( size_t idx = 0; idx < gcells.size(); idx++ )
        {
            size_t f = gcells[idx];
            size_t ijk[3];
            parentIjk( f, ijk );
            size_t                       plocal                     = naturalIndex( ijk[0], ijk[1], ijk[2], pDimX, pDimY );
            const std::array<size_t, 3>& flatMin                    = flatOrigin[plocal];
            std::array<size_t, 3>        q                          = { ijk[0] * sub[0] + ( f % nx ) - flatMin[0],
                                                                        ijk[1] * sub[1] + ( ( f / nx ) % ny ) - flatMin[1],
                                                                        ijk[2] * sub[2] + ( f / ( nx * ny ) ) - flatMin[2] };
            qOfCell[idx]                                            = q;
            qToCell[naturalIndex( q[0], q[1], q[2], qDimX, qDimY )] = idx;
        }

        // Merge cells into connected regions (6-connectivity in q space) - one LGR per region.
        std::vector<bool> visited( gcells.size(), false );
        for ( size_t seed = 0; seed < gcells.size(); seed++ )
        {
            if ( visited[seed] ) continue;

            std::vector<size_t> regionCells;
            std::vector<size_t> stack = { seed };
            visited[seed]             = true;
            while ( !stack.empty() )
            {
                size_t cur = stack.back();
                stack.pop_back();
                regionCells.push_back( cur );

                const std::array<size_t, 3>& q = qOfCell[cur];
                const int nbr[6][3]            = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
                for ( const auto& d : nbr )
                {
                    long ni = (long)q[0] + d[0], nj = (long)q[1] + d[1], nk = (long)q[2] + d[2];
                    if ( ni < 0 || nj < 0 || nk < 0 ) continue;
                    auto it = qToCell.find( naturalIndex( (size_t)ni, (size_t)nj, (size_t)nk, qDimX, qDimY ) );
                    if ( it != qToCell.end() && !visited[it->second] )
                    {
                        visited[it->second] = true;
                        stack.push_back( it->second );
                    }
                }
            }

            // Bounding box of the region in q space.
            size_t qmin[3] = { SIZE_MAX, SIZE_MAX, SIZE_MAX }, qmax[3] = { 0, 0, 0 };
            for ( size_t idx : regionCells )
                for ( int a = 0; a < 3; a++ )
                {
                    qmin[a] = std::min( qmin[a], qOfCell[idx][a] );
                    qmax[a] = std::max( qmax[a], qOfCell[idx][a] );
                }
            const cvf::Vec3st dims( qmax[0] - qmin[0] + 1, qmax[1] - qmin[1] + 1, qmax[2] - qmin[2] + 1 );

            std::vector<size_t> boxToParent( dims.x() * dims.y() * dims.z(), cvf::UNDEFINED_SIZE_T );
            for ( size_t lk = 0; lk < dims.z(); lk++ )
                for ( size_t lj = 0; lj < dims.y(); lj++ )
                    for ( size_t li = 0; li < dims.x(); li++ )
                    {
                        size_t local = naturalIndex( li, lj, lk, dims.x(), dims.y() );
                        size_t qi = qmin[0] + li, qj = qmin[1] + lj, qk = qmin[2] + lk;
                        boxToParent[local] = naturalIndex( qi / sub[0], qj / sub[1], qk / sub[2], pDimX, pDimY );
                    }

            std::vector<size_t> boxToFlat( dims.x() * dims.y() * dims.z(), cvf::UNDEFINED_SIZE_T );
            for ( size_t idx : regionCells )
            {
                const std::array<size_t, 3>& q     = qOfCell[idx];
                size_t                       local = naturalIndex( q[0] - qmin[0], q[1] - qmin[1], q[2] - qmin[2], dims.x(), dims.y() );
                boxToFlat[local]                   = gcells[idx];
            }

            const QString gridName = QString( "LGR_NHG_L%1_%2" ).arg( level ).arg( component++ );
            buildLocalGrid( caseData, parentGrid, nextGridId++, gridName, dims, boxToFlat, boxToParent, sourceCells, true );
        }
    }
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
