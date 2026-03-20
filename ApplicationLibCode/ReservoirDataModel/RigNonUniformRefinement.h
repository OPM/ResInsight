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

#pragma once

#include "cvfVector3.h"

#include <cstddef>
#include <utility>
#include <vector>

//==================================================================================================
//
// Non-uniform per-dimension refinement specification for sector model export.
//
// Since refinement "crosses over" the model (all cells at a given I index share the same
// I-refinement), each dimension is independent.
//
// For each dimension: a vector-of-vectors indexed by original cell index (relative to sector),
// containing cumulative fractions in (0, 1]. Cells with no explicit refinement default to {1.0}
// (1 subcell = no refinement).
//
// A cumulative offset table per dimension enables O(1) original->refined lookup
// and O(log n) reverse lookup via binary search.
//
//==================================================================================================
class RigNonUniformRefinement
{
public:
    enum Dimension : size_t
    {
        DimI = 0,
        DimJ = 1,
        DimK = 2
    };

    RigNonUniformRefinement();
    explicit RigNonUniformRefinement( const cvf::Vec3st& sectorSize );

    // Set cumulative fractions for a specific cell index in a dimension.
    // The fractions must be strictly increasing and end at 1.0.
    void setCumulativeFractions( Dimension dim, size_t origIndex, const std::vector<double>& cumulativeFractions );

    // Set fractional widths for a specific cell index in a dimension.
    // Widths are normalized so they sum to 1.0, then converted to cumulative fractions.
    static std::vector<double> widthsToCumulativeFractions( const std::vector<double>& widths );

    // Query methods
    size_t                     subcellCount( Dimension dim, size_t origIndex ) const;
    size_t                     cumulativeOffset( Dimension dim, size_t origIndex ) const;
    size_t                     totalRefinedCount( Dimension dim ) const;
    std::pair<size_t, size_t>  mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const;
    const std::vector<double>& cumulativeFractions( Dimension dim, size_t origIndex ) const;
    size_t                     sectorSize( Dimension dim ) const;

    bool hasNonUniformRefinement() const;

    // Distribute fractional widths across a range of cells in a given dimension.
    // The widths span the combined range [startIndex, endIndex] — subdivision boundaries
    // are placed in the global fraction space and then assigned to individual cells.
    // Each cell in the range occupies an equal share (1/numCells) of the global range.
    void distributeWidthsAcrossCells( Dimension dim, size_t startIndex, size_t endIndex, const std::vector<double>& widths );

    // Create a non-uniform refinement that is equivalent to uniform refinement
    static RigNonUniformRefinement fromUniform( const cvf::Vec3st& refinement, const cvf::Vec3st& sectorSize );

private:
    void rebuildOffsets( Dimension dim );

    // Per-dimension data: m_fractions[dim][origIndex] = cumulative fractions
    std::vector<std::vector<double>> m_fractions[3];

    // Per-dimension cumulative offset table: m_offsets[dim][n] = total refined cells for cells 0..n-1
    std::vector<size_t> m_offsets[3];

    // Sector dimensions (number of original cells per dimension)
    cvf::Vec3st m_sectorSize;
};
