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

#include "RigRefinement.h"

#include "cvfVector3.h"

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
class RigNonUniformRefinement : public RigRefinement
{
public:
    explicit RigNonUniformRefinement( const cvf::Vec3st& sectorSize );

    std::unique_ptr<RigRefinement> clone() const override;

    // Set cumulative fractions for a specific cell index in a dimension.
    // The fractions must be strictly increasing and end at 1.0.
    void setCumulativeFractions( Dimension dim, size_t origIndex, const std::vector<double>& cumulativeFractions );

    // Query methods (overrides from RigRefinement)
    size_t                     subcellCount( Dimension dim, size_t origIndex ) const override;
    size_t                     cumulativeOffset( Dimension dim, size_t origIndex ) const override;
    size_t                     totalRefinedCount( Dimension dim ) const override;
    std::pair<size_t, size_t>  mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const override;
    const std::vector<double>& cumulativeFractions( Dimension dim, size_t origIndex ) const override;
    size_t                     sectorSize( Dimension dim ) const override;
    bool                       hasRefinement() const override;

    // Distribute fractional widths across a range of cells in a given dimension.
    // The widths span the combined range [startIndex, endIndex] — subdivision boundaries
    // are placed in the global fraction space and then assigned to individual cells.
    // Each cell in the range occupies an equal share (1/numCells) of the global range.
    void distributeWidthsAcrossCells( Dimension dim, size_t startIndex, size_t endIndex, const std::vector<double>& widths );

private:
    void rebuildOffsets( Dimension dim );

    // Per-dimension data: m_fractions[dim][origIndex] = cumulative fractions
    std::vector<std::vector<double>> m_fractions[3];

    // Per-dimension cumulative offset table: m_offsets[dim][n] = total refined cells for cells 0..n-1
    std::vector<size_t> m_offsets[3];

    // Sector dimensions (number of original cells per dimension)
    cvf::Vec3st m_sectorSize;
};
