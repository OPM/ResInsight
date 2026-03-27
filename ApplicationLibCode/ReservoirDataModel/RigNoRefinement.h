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
// Identity refinement: no subdivision, all lookups are pass-through.
// Lightweight — no offset tables, no fraction vectors.
//
//==================================================================================================
class RigNoRefinement : public RigRefinement
{
public:
    explicit RigNoRefinement( const cvf::Vec3st& sectorSize );

    std::unique_ptr<RigRefinement> clone() const override;

    size_t                     subcellCount( Dimension dim, size_t origIndex ) const override;
    size_t                     cumulativeOffset( Dimension dim, size_t origIndex ) const override;
    size_t                     totalRefinedCount( Dimension dim ) const override;
    std::pair<size_t, size_t>  mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const override;
    const std::vector<double>& cumulativeFractions( Dimension dim, size_t origIndex ) const override;
    size_t                     sectorSize( Dimension dim ) const override;
    bool                       hasRefinement() const override;

private:
    cvf::Vec3st m_sectorSize;
};
