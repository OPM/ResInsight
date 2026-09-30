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

#include "RigContourMapPeakFinder.h"

#include "cafAssert.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace RigContourMapPeakFinder
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
    /// Prominence per cell (NaN for non-peak cells, >= 0 for peaks). For each peak, also records whether
    /// its region survived the sweep (i.e. it is the highest peak of a connected defined area) and the
    /// number of cells in that surviving region.
    ///
    /// A zero prominence peak that was absorbed at a saddle is never a real peak: it is a flat shoulder
    /// or plateau cell split off by the tie-break rule. A surviving zero prominence peak is a completely
    /// flat connected area, which only counts as a peak if it is a single isolated cell.
    //--------------------------------------------------------------------------------------------------
    struct ProminenceResult
    {
        std::vector<double> prominence;
        std::vector<char>   isSurvivingRoot;
        std::vector<int>    regionSize;
    };

    ProminenceResult computeProminenceAndRegionSize( const std::vector<double>& z, int nx, int ny )
    {
        CAF_ASSERT( nx >= 0 && ny >= 0 );
        CAF_ASSERT( z.size() == static_cast<size_t>( nx ) * static_cast<size_t>( ny ) );

        const int    M   = nx * ny;
        const double nan = std::numeric_limits<double>::quiet_NaN();

        ProminenceResult result;
        result.prominence.assign( M, nan );
        result.isSurvivingRoot.assign( M, 0 );
        result.regionSize.assign( M, 0 );

        std::vector<int> order;
        order.reserve( M );

        for ( int k = 0; k < M; ++k )
        {
            if ( std::isfinite( z[k] ) ) order.push_back( k );
        }

        // Descending by value, ties broken by index -> plateaus yield exactly one peak
        std::sort( order.begin(), order.end(), [&]( int a, int b ) { return z[a] > z[b] || ( z[a] == z[b] && a < b ); } );

        std::vector<int>    parent( M, -1 ); // -1 = not yet processed
        std::vector<int>    peakOf( M, -1 ); // valid for roots: index of the region's highest cell
        std::vector<int>    liveRegionSize( M, 0 ); // valid for roots: number of cells merged into the region so far
        std::vector<double> regionMin( M, 0.0 ); // valid for roots: lowest value merged into the region so far

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
            regionMin[c]      = z[c];

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
                liveRegionSize[dominant] += liveRegionSize[r];
                parent[r] = dominant;
            }
            liveRegionSize[dominant] += liveRegionSize[c];
            regionMin[dominant] = std::min( regionMin[dominant], z[c] ); // cells are visited in descending order
            parent[c]           = dominant;
        }

        // Surviving regions (one per connected defined area): prominence relative to the region's own minimum
        for ( int k : order )
        {
            if ( parent[k] == k )
            {
                result.prominence[peakOf[k]]      = z[peakOf[k]] - regionMin[k];
                result.isSurvivingRoot[peakOf[k]] = 1;
                result.regionSize[peakOf[k]]      = liveRegionSize[k];
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
std::vector<Peak>
    findPeaks( const std::vector<double>& z, int nx, int ny, double dx, double dy, const Settings& settings, double originX, double originY )
{
    const internal::ProminenceResult prominenceResult = internal::computeProminenceAndRegionSize( z, nx, ny );
    const std::vector<double>&       prominence       = prominenceResult.prominence;
    const std::vector<char>&         isSurvivingRoot  = prominenceResult.isSurvivingRoot;
    const std::vector<int>&          regionSize       = prominenceResult.regionSize;

    const int maxPeaks = std::max( 0, settings.maxPeaks );
    if ( maxPeaks == 0 ) return {};

    std::vector<Peak> candidates;
    for ( int k = 0; k < nx * ny; ++k )
    {
        if ( std::isnan( prominence[k] ) || prominence[k] < settings.minProminence ) continue;

        // Zero prominence: flat shoulders and plateaus are not peaks. Only a single isolated cell (no other
        // data to compare against) is kept as a legitimate, if trivial, peak.
        if ( prominence[k] <= 0.0 && !( isSurvivingRoot[k] && regionSize[k] == 1 ) ) continue;

        const int i = k % nx;
        const int j = k / nx;
        if ( settings.excludeEdgePeaks && internal::touchesEdgeOrUndefined( z, nx, ny, i, j ) ) continue;

        candidates.push_back( { i, j, originX + i * dx, originY + j * dy, z[k], prominence[k] } );
    }

    std::sort( candidates.begin(),
               candidates.end(),
               [&]( const Peak& a, const Peak& b )
               {
                   const double ka = settings.rankByProminence ? a.prominence : a.z;
                   const double kb = settings.rankByProminence ? b.prominence : b.z;
                   return ka > kb;
               } );

    // Greedy non-maximum suppression on distance
    const double      minDist2 = settings.minDistance * settings.minDistance;
    std::vector<Peak> result;
    result.reserve( maxPeaks );

    for ( const Peak& c : candidates )
    {
        if ( static_cast<int>( result.size() ) >= maxPeaks ) break;

        const bool farEnough = std::none_of( result.begin(),
                                             result.end(),
                                             [&]( const Peak& t )
                                             {
                                                 const double ddx = c.x - t.x;
                                                 const double ddy = c.y - t.y;
                                                 return ddx * ddx + ddy * ddy < minDist2;
                                             } );
        if ( farEnough ) result.push_back( c );
    }

    return result;
}

} // namespace RigContourMapPeakFinder
