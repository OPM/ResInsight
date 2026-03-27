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

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

//==================================================================================================
//
// Abstract base class for grid refinement specifications.
//
// Provides a uniform interface for querying refinement across three dimensions (I, J, K).
// Concrete subclasses:
//   - RigNoRefinement        : identity (1:1 mapping, no refinement)
//   - RigUniformRefinement   : same subcell count per cell in each dimension
//   - RigNonUniformRefinement: per-cell custom cumulative fractions
//
//==================================================================================================
class RigRefinement
{
public:
    enum Dimension : size_t
    {
        DimI = 0,
        DimJ = 1,
        DimK = 2
    };

    virtual ~RigRefinement() = default;

    virtual std::unique_ptr<RigRefinement> clone() const = 0;

    // Query methods
    virtual size_t                     subcellCount( Dimension dim, size_t origIndex ) const            = 0;
    virtual size_t                     cumulativeOffset( Dimension dim, size_t origIndex ) const        = 0;
    virtual size_t                     totalRefinedCount( Dimension dim ) const                         = 0;
    virtual std::pair<size_t, size_t>  mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const = 0;
    virtual const std::vector<double>& cumulativeFractions( Dimension dim, size_t origIndex ) const     = 0;
    virtual size_t                     sectorSize( Dimension dim ) const                                = 0;
    virtual bool                       hasRefinement() const                                            = 0;

    // Static utilities shared by subclasses
    static std::vector<double> widthsToCumulativeFractions( const std::vector<double>& widths );
    static std::vector<double> generateEqualFractions( size_t subcellCount );
    static std::vector<double> generateLogarithmicWidths( size_t totalCells );
};
