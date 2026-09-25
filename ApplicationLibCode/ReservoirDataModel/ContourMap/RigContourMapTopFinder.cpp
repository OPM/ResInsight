/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RigContourMapTopFinder.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace RigContourMapTopFinder
{
namespace internal
{
    //--------------------------------------------------------------------------------------------------
    /// Returns true if the cell at (i, j), or any of its 8 neighbors, is outside the grid or undefined.
    //--------------------------------------------------------------------------------------------------
    bool touchesEdgeOrUndefined( const std::vector<double>& z, int nx, int ny, int i, int j )
    {
        for ( int dj = -1; dj <= 1; ++dj )
        {
            for ( int di = -1; di <= 1; ++di )
            {
                const int ni = i + di;
                const int nj = j + dj;
                if ( ni < 0 || nj < 0 || ni >= nx || nj >= ny ) return true;
                if ( !std::isfinite( z[ni + nj * nx] ) ) return true;
            }
        }
        return false;
    }

    //--------------------------------------------------------------------------------------------------
    /// Prominence per cell (NaN for non-peak cells, >= 0 for peaks) together with the number of cells in
    /// the region each peak represented at the moment it was recorded (either absorbed by a higher
    /// neighboring region, or still standing once the sweep finished).
    ///
    /// The region size lets findTops() reject the single arbitrary cell that the tie-break rule in the
    /// union-find sweep turns into a "peak" for a large, perfectly flat area at the global minimum value
    /// (e.g. an undefined/background plateau) - that cell has zero prominence but represents many cells,
    /// unlike a genuine isolated one-cell peak.
    //--------------------------------------------------------------------------------------------------
    struct ProminenceResult
    {
        std::vector<double> prominence;
        std::vector<int>    regionSize;
    };

    ProminenceResult computeProminenceAndRegionSize( const std::vector<double>& z, int nx, int ny )
    {
        const int    M   = nx * ny;
        const double nan = std::numeric_limits<double>::quiet_NaN();

        ProminenceResult result;
        result.prominence.assign( M, nan );
        result.regionSize.assign( M, 0 );

        std::vector<int> order;
        order.reserve( M );

        double zMin = std::numeric_limits<double>::infinity();
        for ( int k = 0; k < M; ++k )
        {
            if ( std::isfinite( z[k] ) )
            {
                order.push_back( k );
                zMin = std::min( zMin, z[k] );
            }
        }

        // Descending by value, ties broken by index -> plateaus yield exactly one peak
        std::sort( order.begin(), order.end(), [&]( int a, int b ) { return z[a] > z[b] || ( z[a] == z[b] && a < b ); } );

        std::vector<int> parent( M, -1 ); // -1 = not yet processed
        std::vector<int> peakOf( M, -1 ); // valid for roots: index of the region's highest cell
        std::vector<int> liveRegionSize( M, 0 ); // valid for roots: number of cells merged into the region so far

        auto findRoot = [&]( int a )
        {
            while ( parent[a] != a )
            {
                parent[a] = parent[parent[a]]; // path halving
                a         = parent[a];
            }
            return a;
        };

        auto higherPeak = [&]( int pa, int pb ) { return z[pa] > z[pb] || ( z[pa] == z[pb] && pa < pb ); };

        std::vector<int> roots;
        roots.reserve( 8 );

        for ( int c : order )
        {
            parent[c]         = c;
            peakOf[c]         = c;
            liveRegionSize[c] = 1;

            const int ci = c % nx;
            const int cj = c / nx;

            roots.clear();
            for ( int dj = -1; dj <= 1; ++dj )
            {
                for ( int di = -1; di <= 1; ++di )
                {
                    if ( di == 0 && dj == 0 ) continue;
                    const int ni = ci + di;
                    const int nj = cj + dj;
                    if ( ni < 0 || nj < 0 || ni >= nx || nj >= ny ) continue;

                    const int n = ni + nj * nx;
                    if ( parent[n] == -1 ) continue;

                    const int r = findRoot( n );
                    if ( std::find( roots.begin(), roots.end(), r ) == roots.end() ) roots.push_back( r );
                }
            }

            if ( roots.empty() ) continue; // c is a new peak

            // Dominant region = the one with the highest peak
            int dominant = roots.front();
            for ( int r : roots )
            {
                if ( higherPeak( peakOf[r], peakOf[dominant] ) ) dominant = r;
            }

            // c is the saddle for all other regions: they die here
            for ( int r : roots )
            {
                if ( r == dominant ) continue;
                result.prominence[peakOf[r]] = z[peakOf[r]] - z[c];
                result.regionSize[peakOf[r]] = liveRegionSize[r];
                liveRegionSize[dominant] += liveRegionSize[r];
                parent[r] = dominant;
            }
            liveRegionSize[dominant] += liveRegionSize[c];
            parent[c] = dominant;
        }

        // Surviving regions (one per connected defined area): prominence relative to global minimum
        for ( int k : order )
        {
            if ( parent[k] == k )
            {
                result.prominence[peakOf[k]] = z[peakOf[k]] - zMin;
                result.regionSize[peakOf[k]] = liveRegionSize[k];
            }
        }

        return result;
    }
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> computeProminence( const std::vector<double>& z, int nx, int ny )
{
    return internal::computeProminenceAndRegionSize( z, nx, ny ).prominence;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Top>
    findTops( const std::vector<double>& z, int nx, int ny, double dx, double dy, const Settings& settings, double originX, double originY )
{
    const internal::ProminenceResult prominenceResult = internal::computeProminenceAndRegionSize( z, nx, ny );
    const std::vector<double>&       prominence       = prominenceResult.prominence;
    const std::vector<int>&          regionSize       = prominenceResult.regionSize;

    std::vector<Top> candidates;
    for ( int k = 0; k < nx * ny; ++k )
    {
        if ( std::isnan( prominence[k] ) || prominence[k] < settings.minProminence ) continue;

        // A cell can end up as a union-find root with zero prominence when it represents a large,
        // perfectly flat region at the global minimum value (e.g. an undefined/background plateau) -
        // that is not an actual local maximum. A single isolated cell with zero prominence (no other
        // data to compare against) is still a legitimate, if trivial, top.
        if ( prominence[k] <= 0.0 && regionSize[k] > 1 ) continue;

        const int i = k % nx;
        const int j = k / nx;
        if ( settings.excludeEdgeTops && internal::touchesEdgeOrUndefined( z, nx, ny, i, j ) ) continue;

        candidates.push_back( { i, j, originX + i * dx, originY + j * dy, z[k], prominence[k] } );
    }

    std::sort( candidates.begin(),
               candidates.end(),
               [&]( const Top& a, const Top& b )
               {
                   const double ka = settings.rankByProminence ? a.prominence : a.z;
                   const double kb = settings.rankByProminence ? b.prominence : b.z;
                   return ka > kb;
               } );

    // Greedy non-maximum suppression on distance
    const double     minDist2 = settings.minDistance * settings.minDistance;
    std::vector<Top> result;
    result.reserve( settings.maxTops );

    for ( const Top& c : candidates )
    {
        if ( static_cast<int>( result.size() ) >= settings.maxTops ) break;

        const bool farEnough = std::none_of( result.begin(),
                                             result.end(),
                                             [&]( const Top& t )
                                             {
                                                 const double ddx = c.x - t.x;
                                                 const double ddy = c.y - t.y;
                                                 return ddx * ddx + ddy * ddy < minDist2;
                                             } );
        if ( farEnough ) result.push_back( c );
    }

    return result;
}

} // namespace RigContourMapTopFinder
