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

#include "RigNestedHybridGridFipnestCodec.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <map>
#include <numeric>
#include <set>
#include <vector>

namespace
{
size_t naturalIndex( size_t i, size_t j, size_t k, size_t nx, size_t ny )
{
    return i + j * nx + k * nx * ny;
}

std::array<int, 3> flatIjk( size_t f, size_t nx, size_t ny )
{
    return { (int)( f % nx ), (int)( ( f / nx ) % ny ), (int)( f / ( nx * ny ) ) };
}

// A primary (coarse-refining) level, recorded so the next level can be encoded/decoded against it.
struct PrimaryLevelRecord
{
    int level     = 0;
    int t0[3]     = { 0, 0, 0 }; // min TMP of the level's cells
    int c0[3]     = { 0, 0, 0 }; // min OLD of the level's cells
    int factor[3] = { 1, 1, 1 }; // per-axis refinement factor
    int dims[3]   = { 0, 0, 0 }; // TMP-box dimensions (coarseDim * factor)

    // Per-axis linear map from the level's TMP space to the flat grid: flat = s*tmp + c. Lets a
    // refined-away (hole) host slot be located as a flat cell even though no real cell occupies it.
    bool      bandValid = false;
    long long bandS[3]  = { 0, 0, 0 };
    long long bandC[3]  = { 0, 0, 0 };

    std::map<std::array<int, 3>, size_t> realCellByTmp; // TMP triple -> flat cell of the level's real cells
};
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigNestedHybridGridFipnestCodec::packSlot( int offI, int offJ, int offK )
{
    return 1 + offI + 100 * offJ + 10000 * offK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridFipnestCodec::unpackSlot( int slot, int& offI, int& offJ, int& offK )
{
    const int v = slot - 1;
    offI        = v % 100;
    offJ        = ( v / 100 ) % 100;
    offK        = v / 10000;
}

//--------------------------------------------------------------------------------------------------
/// Mirrors RigNestedHybridGridReconstructor::reconstruct()'s level classification: levels are
/// processed shallow to deep; a level whose cells' TMPs fall inside the previous primary level's
/// TMP box (>= 95%) is nested under that level, otherwise it is a direct refinement of the coarse
/// grid. The immediate parent written to FIPNEST is the coarse host slot for primary-level cells
/// and the previous level's (possibly refined-away) host slot for nested cells.
//--------------------------------------------------------------------------------------------------
RigNestedHybridGridFipnestCodec::ParentChildArrays
    RigNestedHybridGridFipnestCodec::computeParentChildArrays( const RigNestedHybridGridReconstructor::NestedHybridInput& input,
                                                               size_t                                                     nx,
                                                               size_t                                                     ny,
                                                               size_t                                                     nz )
{
    ParentChildArrays result;

    const size_t cellCount = nx * ny * nz;
    if ( input.refine.size() != cellCount || input.oldI.size() != cellCount || input.oldJ.size() != cellCount ||
         input.oldK.size() != cellCount || input.tmpI.size() != cellCount || input.tmpJ.size() != cellCount || input.tmpK.size() != cellCount )
    {
        return result; // empty arrays signal invalid input
    }

    result.fipnest.assign( cellCount, 0 );
    result.fipslot.assign( cellCount, 0 );

    // Same coarse-dimension and K-factor derivation as the reconstructor.
    int coarseNz = 0;
    for ( size_t f = 0; f < cellCount; f++ )
        coarseNz = std::max( coarseNz, input.oldK[f] );
    const size_t kFactor = ( coarseNz > 0 && nz % (size_t)coarseNz == 0 ) ? nz / (size_t)coarseNz : 1;

    std::map<int, std::vector<size_t>> cellsByLevel;
    for ( size_t f = 0; f < cellCount; f++ )
    {
        const int level = input.refine[f];
        if ( level <= 1 ) continue;
        if ( input.oldI[f] < 1 || input.oldJ[f] < 1 || input.oldK[f] < 1 )
        {
            result.unresolvedRefinedCells++; // padding / unmapped; skipped by the reconstructor too
            continue;
        }
        cellsByLevel[level].push_back( f );
    }

    auto coarseHostFlatIndex = [&]( int oi, int oj, int ok )
    { return naturalIndex( (size_t)( oi - 1 ), (size_t)( oj - 1 ), (size_t)( ok - 1 ) * kFactor, nx, ny ); };

    std::vector<PrimaryLevelRecord> primaries;
    std::set<int>                   nestedLevels;

    for ( const auto& [level, cells] : cellsByLevel )
    {
        if ( nestedLevels.count( level - 1 ) )
        {
            // The reconstructor would nest this level under the (nested) level below it, but the
            // encoding only supports nesting under a primary level (see class comment). Leaving the
            // cells unresolved is safer than mis-encoding them as a primary refinement.
            result.unresolvedRefinedCells += cells.size();
            continue;
        }

        const PrimaryLevelRecord* prev = nullptr;
        for ( const PrimaryLevelRecord& p : primaries )
            if ( p.level == level - 1 ) prev = &p;

        auto tmpOf     = [&]( size_t f ) { return std::array<int, 3>{ input.tmpI[f], input.tmpJ[f], input.tmpK[f] }; };
        auto inPrevBox = [&]( const std::array<int, 3>& t )
        {
            if ( !prev ) return false;
            for ( int a = 0; a < 3; a++ )
                if ( t[a] < prev->t0[a] || t[a] >= prev->t0[a] + prev->dims[a] ) return false;
            return true;
        };

        size_t inBox = 0;
        for ( size_t f : cells )
            if ( inPrevBox( tmpOf( f ) ) ) inBox++;

        if ( prev && inBox >= cells.size() * 95 / 100 && inBox > 0 )
        {
            // Nested level: each cell's immediate parent is the previous level's cell (or hole slot)
            // at the cell's own TMP - the same lookup the reconstructor performs via tmpToBuilt.
            std::map<std::array<int, 3>, std::array<int, 3>> groupFlatMin; // parent TMP -> min flat IJK of its children
            for ( size_t f : cells )
            {
                const auto t = tmpOf( f );
                if ( !inPrevBox( t ) ) continue;
                const auto fxyz = flatIjk( f, nx, ny );
                auto       it   = groupFlatMin.find( t );
                if ( it == groupFlatMin.end() )
                    groupFlatMin[t] = fxyz;
                else
                    for ( int a = 0; a < 3; a++ )
                        it->second[a] = std::min( it->second[a], fxyz[a] );
            }

            for ( size_t f : cells )
            {
                const auto t = tmpOf( f );
                if ( !inPrevBox( t ) )
                {
                    result.unresolvedRefinedCells++;
                    continue;
                }

                size_t parentFlat = 0;
                bool   parentOk   = false;
                if ( auto it = prev->realCellByTmp.find( t ); it != prev->realCellByTmp.end() )
                {
                    parentFlat = it->second;
                    parentOk   = true;
                }
                else if ( prev->bandValid )
                {
                    // The host was refined away; locate its collapsed flat slot through the band map
                    // and give that slot its own chain link back to its coarse host.
                    long long p[3];
                    parentOk = true;
                    for ( int a = 0; a < 3; a++ )
                    {
                        p[a]                = prev->bandS[a] * t[a] + prev->bandC[a];
                        const long long dim = ( a == 0 ) ? (long long)nx : ( a == 1 ) ? (long long)ny : (long long)nz;
                        if ( p[a] < 0 || p[a] >= dim ) parentOk = false;
                    }
                    if ( parentOk )
                    {
                        parentFlat = naturalIndex( (size_t)p[0], (size_t)p[1], (size_t)p[2], nx, ny );

                        int  hostOld[3], hostOff[3];
                        bool hostOffOk = true;
                        for ( int a = 0; a < 3; a++ )
                        {
                            hostOld[a] = prev->c0[a] + ( t[a] - prev->t0[a] ) / prev->factor[a];
                            hostOff[a] = ( t[a] - prev->t0[a] ) % prev->factor[a];
                            if ( hostOff[a] >= 100 ) hostOffOk = false; // beyond FIPSLOT digit capacity
                        }
                        const size_t hostFlat = coarseHostFlatIndex( hostOld[0], hostOld[1], hostOld[2] );

                        if ( input.refine[parentFlat] > 1 || !hostOffOk || hostFlat >= cellCount || input.refine[hostFlat] > 1 )
                        {
                            parentOk = false; // slot occupied by a refined cell, or host not encodable
                        }
                        else
                        {
                            const int expectedLink = (int)hostFlat + 1;
                            const int expectedSlot = packSlot( hostOff[0], hostOff[1], hostOff[2] );
                            if ( result.fipnest[parentFlat] == 0 )
                            {
                                result.fipnest[parentFlat] = expectedLink;
                                result.fipslot[parentFlat] = expectedSlot;
                            }
                            else if ( result.fipnest[parentFlat] != expectedLink || result.fipslot[parentFlat] != expectedSlot )
                            {
                                parentOk = false; // the slot already carries a different chain link
                            }
                        }
                    }
                }

                const auto  fxyz   = flatIjk( f, nx, ny );
                const auto& mn     = groupFlatMin[t];
                const int   off[3] = { fxyz[0] - mn[0], fxyz[1] - mn[1], fxyz[2] - mn[2] };
                if ( !parentOk || off[0] >= 100 || off[1] >= 100 || off[2] >= 100 )
                {
                    result.unresolvedRefinedCells++;
                    continue;
                }

                result.fipnest[f] = (int)parentFlat + 1;
                result.fipslot[f] = packSlot( off[0], off[1], off[2] );
            }
            nestedLevels.insert( level );
            continue;
        }

        // Primary level: a direct, uniform refinement of the coarse grid (same derivation and
        // uniformity gate as the reconstructor).
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

        int factor[3];
        for ( int a = 0; a < 3; a++ )
        {
            const int coarseDim = c1[a] - c0[a] + 1;
            const int tmpDim    = t1[a] - t0[a] + 1;
            factor[a]           = std::max( 1, (int)std::lround( (double)tmpDim / (double)coarseDim ) );
        }

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
            result.unresolvedRefinedCells += cells.size(); // the reconstructor defers this level
            continue;
        }

        PrimaryLevelRecord record;
        record.level = level;
        for ( int a = 0; a < 3; a++ )
        {
            record.t0[a]     = t0[a];
            record.c0[a]     = c0[a];
            record.factor[a] = factor[a];
            record.dims[a]   = ( c1[a] - c0[a] + 1 ) * factor[a];
        }

        for ( size_t f : cells )
        {
            const int t[3]   = { input.tmpI[f], input.tmpJ[f], input.tmpK[f] };
            const int old[3] = { input.oldI[f], input.oldJ[f], input.oldK[f] };
            int       off[3];
            bool      offOk = true;
            for ( int a = 0; a < 3; a++ )
            {
                off[a] = ( t[a] - t0[a] ) - ( old[a] - c0[a] ) * factor[a];
                if ( off[a] < 0 || off[a] >= factor[a] || off[a] >= 100 ) offOk = false;
            }
            // The chain must terminate at the coarse host, so the flat cell occupying that slot has
            // to be unrefined (a refined cell there would carry its own, unrelated link).
            const size_t hostFlat = coarseHostFlatIndex( old[0], old[1], old[2] );
            if ( !offOk || hostFlat >= cellCount || input.refine[hostFlat] > 1 )
            {
                result.unresolvedRefinedCells++;
                continue;
            }

            result.fipnest[f]                          = (int)hostFlat + 1;
            result.fipslot[f]                          = packSlot( off[0], off[1], off[2] );
            record.realCellByTmp[{ t[0], t[1], t[2] }] = f;
        }

        // Fit the per-axis linear TMP -> flat band map from the level's real cells.
        record.bandValid = true;
        for ( int a = 0; a < 3 && record.bandValid; a++ )
        {
            long long flatAtT0 = -1, flatAtT1 = -1;
            for ( size_t f : cells )
            {
                const int t[3] = { input.tmpI[f], input.tmpJ[f], input.tmpK[f] };
                const int fa   = flatIjk( f, nx, ny )[a];
                if ( t[a] == t0[a] ) flatAtT0 = fa;
                if ( t[a] == t1[a] ) flatAtT1 = fa;
            }
            if ( flatAtT0 < 0 || flatAtT1 < 0 )
            {
                record.bandValid = false;
                break;
            }
            if ( t1[a] == t0[a] )
            {
                record.bandS[a] = 0;
                record.bandC[a] = flatAtT0;
            }
            else
            {
                const long long dt = t1[a] - t0[a];
                const long long df = flatAtT1 - flatAtT0;
                if ( df % dt != 0 )
                {
                    record.bandValid = false;
                    break;
                }
                record.bandS[a] = df / dt;
                record.bandC[a] = flatAtT0 - record.bandS[a] * t0[a];
            }
            for ( size_t f : cells )
            {
                const int t[3] = { input.tmpI[f], input.tmpJ[f], input.tmpK[f] };
                if ( record.bandS[a] * t[a] + record.bandC[a] != flatIjk( f, nx, ny )[a] )
                {
                    record.bandValid = false;
                    break;
                }
            }
        }

        primaries.push_back( record );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Synthesizes OLDI/J/K from each cell's parent chain and TMPI/J/K from the recovered per-level
/// refinement factors and the FIPSLOT offsets. The synthesized TMPs equal the original sidecar TMPs
/// up to a constant per-level per-axis shift, which reconstruct() is invariant to; disjoint shifts
/// are chosen so no TMP triple of one level collides with another level's slot registration.
//--------------------------------------------------------------------------------------------------
RigNestedHybridGridReconstructor::NestedHybridInput
    RigNestedHybridGridFipnestCodec::buildInputFromParentChildArrays( const std::vector<int>& fipnest,
                                                                      const std::vector<int>& fipslot,
                                                                      const std::vector<int>& refine,
                                                                      size_t                  nx,
                                                                      size_t                  ny,
                                                                      size_t                  nz,
                                                                      QString*                warnings )
{
    RigNestedHybridGridReconstructor::NestedHybridInput input;

    auto warn = [&]( const QString& msg )
    {
        if ( warnings ) *warnings += ( warnings->isEmpty() ? "" : "\n" ) + msg;
    };

    const size_t cellCount = nx * ny * nz;
    if ( fipnest.size() != cellCount || fipslot.size() != cellCount || refine.size() != cellCount )
    {
        warn( "FIPNEST/FIPSLOT/REFINE size does not match the grid cell count." );
        return input;
    }

    input.refine = refine;
    input.oldI.assign( cellCount, 0 );
    input.oldJ.assign( cellCount, 0 );
    input.oldK.assign( cellCount, 0 );
    input.tmpI.assign( cellCount, 0 );
    input.tmpJ.assign( cellCount, 0 );
    input.tmpK.assign( cellCount, 0 );

    // Walk each chain to its FIPNEST == 0 root (the coarse host). Memoized; cycles yield no root.
    std::vector<long long> rootOfCell( cellCount, -2 ); // -2 unvisited, -1 invalid, else root flat index
    auto                   rootOf = [&]( size_t f ) -> long long
    {
        std::vector<size_t> path;
        size_t              cur = f;
        while ( rootOfCell[cur] == -2 )
        {
            const int link = fipnest[cur];
            if ( link == 0 )
            {
                rootOfCell[cur] = (long long)cur;
                break;
            }
            if ( link < 1 || (size_t)link > cellCount || path.size() > 16 )
            {
                rootOfCell[cur] = -1; // out of range or unreasonably deep (cycle)
                break;
            }
            path.push_back( cur );
            cur = (size_t)link - 1;
        }
        const long long root = rootOfCell[cur];
        for ( size_t c : path )
            rootOfCell[c] = root;
        return rootOfCell[f];
    };

    // K refinement factor: coarse hosts occupy every kFactor'th flat K layer.
    long long kFactor = 0;
    for ( size_t f = 0; f < cellCount; f++ )
    {
        if ( refine[f] <= 1 || fipnest[f] == 0 ) continue;
        const long long root = rootOf( f );
        if ( root < 0 ) continue;
        kFactor = std::gcd( kFactor, (long long)( (size_t)root / ( nx * ny ) ) );
    }
    // gcd with NZ guarantees a divisor of NZ; with no refined K layers above 0 this yields NZ itself,
    // which maps every host to coarse K = 1 - consistent with how the reconstructor re-derives it.
    kFactor = std::gcd( kFactor, (long long)nz );

    std::map<int, std::vector<size_t>> cellsByLevel;
    size_t                             unresolved = 0;
    for ( size_t f = 0; f < cellCount; f++ )
    {
        if ( refine[f] <= 1 ) continue;
        const long long root = fipnest[f] != 0 ? rootOf( f ) : -1;
        if ( root < 0 || refine[(size_t)root] > 1 )
        {
            unresolved++; // no parent chain; the cell is left out (stays a flat cell)
            continue;
        }
        const auto r  = flatIjk( (size_t)root, nx, ny );
        input.oldI[f] = r[0] + 1;
        input.oldJ[f] = r[1] + 1;
        input.oldK[f] = r[2] / (int)kFactor + 1;
        cellsByLevel[refine[f]].push_back( f );
    }
    if ( unresolved > 0 )
    {
        warn( QString( "%1 refined cells have no usable FIPNEST chain and are left un-nested." ).arg( unresolved ) );
    }

    // Give every unrefined cell its own coarse position as OLD. The reconstructor re-derives the
    // K refinement factor from the maximum OLDK, so covering all NZ layers here pins that factor to
    // the one used for the chain roots above (the refined hosts alone may stop below the top layer,
    // which would silently shift every parent's K slot).
    for ( size_t f = 0; f < cellCount; f++ )
    {
        if ( refine[f] > 1 ) continue;
        const auto r  = flatIjk( f, nx, ny );
        input.oldI[f] = r[0] + 1;
        input.oldJ[f] = r[1] + 1;
        input.oldK[f] = r[2] / (int)kFactor + 1;
    }

    // Per-axis refinement factor and TMP shift of each synthesized primary level.
    struct LevelParams
    {
        int factor[3] = { 1, 1, 1 };
        int shift[3]  = { 0, 0, 0 };
    };
    std::map<int, LevelParams> primaryParams;

    int nextShift[3] = { 0, 0, 0 };

    // Offsets above the FIPSLOT digit capacity cannot come from the encoder; treat them as corrupt
    // input rather than letting huge decoded offsets poison the factor recovery (or overflow).
    const int maxValidSlot = packSlot( 99, 99, 99 );
    auto      slotValid    = [&]( size_t f ) { return fipslot[f] > 0 && fipslot[f] <= maxValidSlot; };

    auto offsetsOf = [&]( size_t f )
    {
        std::array<int, 3> off = { 0, 0, 0 };
        if ( slotValid( f ) ) unpackSlot( fipslot[f], off[0], off[1], off[2] );
        return off;
    };

    size_t droppedCells = 0;

    for ( const auto& [level, cells] : cellsByLevel )
    {
        // A level is nested when its cells' immediate parents are themselves refined-slot cells
        // (they carry a FIPNEST link of their own); primary-level parents are coarse hosts.
        size_t nestedVotes = 0;
        for ( size_t f : cells )
        {
            const size_t parent = (size_t)fipnest[f] - 1;
            if ( fipnest[parent] != 0 ) nestedVotes++;
        }
        const bool isNested = nestedVotes >= cells.size() * 95 / 100 && nestedVotes > 0;

        if ( isNested )
        {
            const auto paramsIt = primaryParams.find( level - 1 );
            if ( paramsIt == primaryParams.end() )
            {
                warn( QString( "Level %1 nests under a level that was not built as a primary refinement; its cells are left un-nested." )
                          .arg( level ) );
                for ( size_t f : cells )
                    input.oldI[f] = input.oldJ[f] = input.oldK[f] = 0;
                continue;
            }
            const LevelParams& pp = paramsIt->second;

            for ( size_t f : cells )
            {
                // The cell carries its immediate parent's TMP: recompute that slot's TMP from the
                // parent's own coarse host and FIPSLOT offsets.
                const size_t parent = (size_t)fipnest[f] - 1;
                if ( fipnest[parent] == 0 || !slotValid( parent ) || rootOf( parent ) < 0 )
                {
                    input.oldI[f] = input.oldJ[f] = input.oldK[f] = 0;
                    droppedCells++;
                    continue;
                }
                const auto pr      = flatIjk( (size_t)rootOf( parent ), nx, ny );
                const int  pOld[3] = { pr[0] + 1, pr[1] + 1, pr[2] / (int)kFactor + 1 };
                const auto pOff    = offsetsOf( parent );

                input.tmpI[f] = ( pOld[0] - 1 ) * pp.factor[0] + pOff[0] + pp.shift[0];
                input.tmpJ[f] = ( pOld[1] - 1 ) * pp.factor[1] + pOff[1] + pp.shift[1];
                input.tmpK[f] = ( pOld[2] - 1 ) * pp.factor[2] + pOff[2] + pp.shift[2];
            }
            continue;
        }

        // Primary level: recover the per-axis refinement factor. Within one coarse column the flat
        // coordinate advances by the band stride s per offset step, and between adjacent coarse
        // columns the column base advances by s*factor - so factor = (base stride) / s. This is what
        // distinguishes e.g. offsets {0,2} of factor 4 (hole layers) from {0,1} of factor 2.
        LevelParams params;
        for ( int a = 0; a < 3; a++ )
        {
            std::map<int, std::map<int, long long>> columns; // old -> off -> flat coordinate
            const int* oldArr = ( a == 0 ) ? input.oldI.data() : ( a == 1 ) ? input.oldJ.data() : input.oldK.data();
            int        maxOff = 0;
            for ( size_t f : cells )
            {
                if ( oldArr[f] < 1 || !slotValid( f ) ) continue;
                const auto off             = offsetsOf( f );
                columns[oldArr[f]][off[a]] = flatIjk( f, nx, ny )[a];
                maxOff                     = std::max( maxOff, off[a] );
            }

            long long s = 0;
            for ( const auto& [old, byOff] : columns )
            {
                if ( byOff.size() < 2 ) continue;
                const auto      first = byOff.begin();
                const auto      last  = std::prev( byOff.end() );
                const long long dOff  = last->first - first->first;
                const long long dFlat = last->second - first->second;
                if ( dOff > 0 && dFlat % dOff == 0 )
                {
                    s = dFlat / dOff;
                    break;
                }
            }

            long long baseStride = 0;
            if ( s > 0 )
            {
                std::map<int, long long> baseByOld;
                for ( const auto& [old, byOff] : columns )
                    baseByOld[old] = byOff.begin()->second - s * byOff.begin()->first;
                for ( auto it = std::next( baseByOld.begin() ); it != baseByOld.end(); ++it )
                {
                    const auto      prevIt = std::prev( it );
                    const long long dOld   = it->first - prevIt->first;
                    const long long dBase  = it->second - prevIt->second;
                    if ( dOld > 0 && dBase % dOld == 0 )
                    {
                        const long long candidate = dBase / dOld;
                        if ( baseStride == 0 )
                            baseStride = candidate;
                        else if ( baseStride != candidate )
                        {
                            warn( QString( "Level %1 axis %2: inconsistent column stride; using the first one." ).arg( level ).arg( a ) );
                        }
                    }
                }
            }

            if ( s > 0 && baseStride > 0 && baseStride % s == 0 && baseStride / s > maxOff )
            {
                params.factor[a] = (int)( baseStride / s );
            }
            else
            {
                params.factor[a] = maxOff + 1;
                if ( columns.size() > 1 )
                {
                    warn( QString( "Level %1 axis %2: refinement factor not recoverable from column strides; assuming %3." )
                              .arg( level )
                              .arg( a )
                              .arg( params.factor[a] ) );
                }
            }
            params.shift[a] = nextShift[a];
        }

        int maxTmp[3] = { 0, 0, 0 };
        for ( size_t f : cells )
        {
            if ( !slotValid( f ) )
            {
                input.oldI[f] = input.oldJ[f] = input.oldK[f] = 0;
                droppedCells++;
                continue;
            }
            const auto off    = offsetsOf( f );
            const int  old[3] = { input.oldI[f], input.oldJ[f], input.oldK[f] };
            const int  tmp[3] = { ( old[0] - 1 ) * params.factor[0] + off[0] + params.shift[0],
                                  ( old[1] - 1 ) * params.factor[1] + off[1] + params.shift[1],
                                  ( old[2] - 1 ) * params.factor[2] + off[2] + params.shift[2] };
            input.tmpI[f]     = tmp[0];
            input.tmpJ[f]     = tmp[1];
            input.tmpK[f]     = tmp[2];
            for ( int a = 0; a < 3; a++ )
                maxTmp[a] = std::max( maxTmp[a], tmp[a] );
        }

        for ( int a = 0; a < 3; a++ )
            nextShift[a] = maxTmp[a] + 1;

        primaryParams[level] = params;
    }

    if ( droppedCells > 0 )
    {
        warn( QString( "%1 refined cells have an unusable parent link or slot and are left un-nested." ).arg( droppedCells ) );
    }

    return input;
}
