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

//==================================================================================================
//
// Uniform refinement: same subcell count for every cell in each dimension.
// O(1) for all operations, no per-cell storage.
//
//==================================================================================================
class RigUniformRefinement : public RigRefinement
{
public:
    RigUniformRefinement( const cvf::Vec3st& refinement, const cvf::Vec3st& sectorSize );

    std::unique_ptr<RigRefinement> clone() const override;

    size_t                     subcellCount( Dimension dim, size_t origIndex ) const override;
    size_t                     cumulativeOffset( Dimension dim, size_t origIndex ) const override;
    size_t                     totalRefinedCount( Dimension dim ) const override;
    std::pair<size_t, size_t>  mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const override;
    const std::vector<double>& cumulativeFractions( Dimension dim, size_t origIndex ) const override;
    size_t                     sectorSize( Dimension dim ) const override;
    bool                       hasRefinement() const override;

private:
    cvf::Vec3st         m_sectorSize;
    cvf::Vec3st         m_refinement;
    std::vector<double> m_cachedFractions[3];
};
