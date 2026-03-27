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

#include "RigRefinement.h"

#include <cmath>
#include <numeric>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigRefinement::widthsToCumulativeFractions( const std::vector<double>& widths )
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
/// Generate N equal cumulative fractions: {1/N, 2/N, ..., 1.0}.
//--------------------------------------------------------------------------------------------------
std::vector<double> RigRefinement::generateEqualFractions( size_t subcellCount )
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
std::vector<double> RigRefinement::generateLogarithmicWidths( size_t totalCells )
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
