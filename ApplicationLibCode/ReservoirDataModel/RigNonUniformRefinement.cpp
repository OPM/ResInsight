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

#include "RigNonUniformRefinement.h"

#include <algorithm>
#include <cmath>
#include <numeric>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNonUniformRefinement::RigNonUniformRefinement()
    : m_sectorSize( 0, 0, 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNonUniformRefinement::RigNonUniformRefinement( const cvf::Vec3st& sectorSize )
    : m_sectorSize( sectorSize )
{
    // Initialize each cell with default {1.0} (no refinement)
    for ( auto dim : { DimI, DimJ, DimK } )
    {
        m_fractions[dim].resize( m_sectorSize[dim], std::vector<double>{ 1.0 } );
        rebuildOffsets( dim );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNonUniformRefinement::setCumulativeFractions( Dimension dim, size_t origIndex, const std::vector<double>& cumulativeFractions )
{
    if ( origIndex >= m_sectorSize[dim] ) return;

    m_fractions[dim][origIndex] = cumulativeFractions;
    rebuildOffsets( dim );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigNonUniformRefinement::widthsToCumulativeFractions( const std::vector<double>& widths )
{
    if ( widths.empty() ) return { 1.0 };

    double sum = std::accumulate( widths.begin(), widths.end(), 0.0 );
    if ( sum <= 0.0 ) return { 1.0 };

    std::vector<double> cumulative;
    cumulative.reserve( widths.size() );

    double runningSum = 0.0;
    for ( size_t i = 0; i < widths.size(); ++i )
    {
        runningSum += widths[i] / sum;
        cumulative.push_back( runningSum );
    }

    // Ensure the last value is exactly 1.0
    cumulative.back() = 1.0;

    return cumulative;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNonUniformRefinement::subcellCount( Dimension dim, size_t origIndex ) const
{
    if ( origIndex >= m_sectorSize[dim] ) return 1;
    return m_fractions[dim][origIndex].size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNonUniformRefinement::cumulativeOffset( Dimension dim, size_t origIndex ) const
{
    // Allow origIndex == m_sectorSize[dim] to get the total count (end sentinel)
    if ( origIndex > m_sectorSize[dim] ) return 0;
    return m_offsets[dim][origIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNonUniformRefinement::totalRefinedCount( Dimension dim ) const
{
    if ( m_sectorSize[dim] == 0 ) return 0;
    return m_offsets[dim][m_sectorSize[dim]];
}

//--------------------------------------------------------------------------------------------------
/// Maps a refined index back to {originalIndex, subIndex} using binary search on the offset table
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> RigNonUniformRefinement::mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const
{
    if ( m_sectorSize[dim] == 0 ) return { 0, 0 };

    // Binary search: find the largest origIndex such that m_offsets[dim][origIndex] <= refinedIndex
    // m_offsets[dim] has size m_sectorSize[dim]+1
    // We want upper_bound - 1
    auto   it        = std::upper_bound( m_offsets[dim].begin(), m_offsets[dim].end(), refinedIndex );
    size_t origIndex = static_cast<size_t>( std::distance( m_offsets[dim].begin(), it ) ) - 1;

    size_t subIndex = refinedIndex - m_offsets[dim][origIndex];
    return { origIndex, subIndex };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigNonUniformRefinement::cumulativeFractions( Dimension dim, size_t origIndex ) const
{
    static const std::vector<double> defaultFraction = { 1.0 };
    if ( origIndex >= m_sectorSize[dim] ) return defaultFraction;
    return m_fractions[dim][origIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNonUniformRefinement::sectorSize( Dimension dim ) const
{
    return m_sectorSize[dim];
}

//--------------------------------------------------------------------------------------------------
/// Distribute fractional widths across a range of cells.
///
/// The widths define subdivision boundaries in the combined range of cells [startIndex, endIndex].
/// Each original cell occupies 1/numCells of the global range (in index space).
/// Global subdivision boundaries that fall within a cell are converted to cell-local fractions.
///
/// Example: 3 cells, widths [0.2, 0.3, 0.5] -> cumulative [0.2, 0.5, 1.0]
///   Cell 0 [0, 0.333]: boundary 0.2 falls here -> local fracs [0.6, 1.0] -> 2 subcells
///   Cell 1 [0.333, 0.667]: boundary 0.5 falls here -> local fracs [0.5, 1.0] -> 2 subcells
///   Cell 2 [0.667, 1.0]: no interior boundaries -> local fracs [1.0] -> 1 subcell
//--------------------------------------------------------------------------------------------------
void RigNonUniformRefinement::distributeWidthsAcrossCells( Dimension dim, size_t startIndex, size_t endIndex, const std::vector<double>& widths )
{
    if ( startIndex > endIndex || endIndex >= m_sectorSize[dim] || widths.empty() ) return;

    // Convert widths to global cumulative fractions
    auto globalFractions = widthsToCumulativeFractions( widths );

    size_t numCells = endIndex - startIndex + 1;

    for ( size_t c = 0; c < numCells; ++c )
    {
        double cellStart = static_cast<double>( c ) / static_cast<double>( numCells );
        double cellEnd   = static_cast<double>( c + 1 ) / static_cast<double>( numCells );
        double cellWidth = cellEnd - cellStart;

        std::vector<double> localFractions;

        // Find global boundaries that fall strictly within this cell
        for ( double gf : globalFractions )
        {
            if ( gf > cellStart && gf < cellEnd )
            {
                // Convert to cell-local fraction
                localFractions.push_back( ( gf - cellStart ) / cellWidth );
            }
        }

        // Always add 1.0 as the final boundary (the cell end)
        localFractions.push_back( 1.0 );

        m_fractions[dim][startIndex + c] = localFractions;
    }

    rebuildOffsets( dim );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNonUniformRefinement::hasNonUniformRefinement() const
{
    for ( auto dim : { DimI, DimJ, DimK } )
    {
        if ( totalRefinedCount( dim ) != m_sectorSize[dim] ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Generate N equal cumulative fractions: {1/N, 2/N, ..., 1.0}.
//--------------------------------------------------------------------------------------------------
std::vector<double> RigNonUniformRefinement::generateEqualFractions( size_t subcellCount )
{
    if ( subcellCount == 0 ) return { 1.0 };

    std::vector<double> fractions;
    fractions.reserve( subcellCount );
    for ( size_t i = 1; i <= subcellCount; ++i )
    {
        fractions.push_back( static_cast<double>( i ) / static_cast<double>( subcellCount ) );
    }
    fractions.back() = 1.0;
    return fractions;
}

//--------------------------------------------------------------------------------------------------
/// Generate logarithmic widths with finer resolution near the center.
/// Uses a geometric series with ratio ~2.0 for one half, then mirrors for symmetry.
/// Center cells get the smallest widths, edge cells the largest.
//--------------------------------------------------------------------------------------------------
std::vector<double> RigNonUniformRefinement::generateLogarithmicWidths( size_t totalCells )
{
    if ( totalCells == 0 ) return {};
    if ( totalCells == 1 ) return { 1.0 };

    const double ratio = 2.0;

    // Build widths for the first half (edge to center: large to small)
    size_t halfCount = totalCells / 2;

    std::vector<double> halfWidths;
    halfWidths.reserve( halfCount );

    for ( size_t i = 0; i < halfCount; ++i )
    {
        // Largest at index 0 (edge), smallest at index halfCount-1 (center)
        halfWidths.push_back( std::pow( ratio, static_cast<double>( halfCount - 1 - i ) ) );
    }

    // Build full symmetric widths vector
    std::vector<double> widths;
    widths.reserve( totalCells );

    // First half: edge to center
    for ( size_t i = 0; i < halfCount; ++i )
    {
        widths.push_back( halfWidths[i] );
    }

    // If odd number of cells, add center cell with smallest width
    if ( totalCells % 2 == 1 )
    {
        widths.push_back( 1.0 );
    }

    // Second half: center to edge (mirror)
    for ( size_t i = halfCount; i > 0; --i )
    {
        widths.push_back( halfWidths[i - 1] );
    }

    return widths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNonUniformRefinement RigNonUniformRefinement::fromUniform( const cvf::Vec3st& refinement, const cvf::Vec3st& sectorSize )
{
    RigNonUniformRefinement result( sectorSize );

    size_t refCount[3] = { refinement.x(), refinement.y(), refinement.z() };

    for ( auto dim : { DimI, DimJ, DimK } )
    {
        if ( refCount[dim] <= 1 ) continue;

        auto uniformFractions = generateEqualFractions( refCount[dim] );

        for ( size_t idx = 0; idx < result.m_sectorSize[dim]; ++idx )
        {
            result.m_fractions[dim][idx] = uniformFractions;
        }
        result.rebuildOffsets( dim );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNonUniformRefinement::rebuildOffsets( Dimension dim )
{
    m_offsets[dim].resize( m_sectorSize[dim] + 1 );
    m_offsets[dim][0] = 0;

    for ( size_t i = 0; i < m_sectorSize[dim]; ++i )
    {
        m_offsets[dim][i + 1] = m_offsets[dim][i] + m_fractions[dim][i].size();
    }
}
