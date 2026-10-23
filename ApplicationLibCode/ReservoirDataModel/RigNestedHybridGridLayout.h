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

#include "cafVecIjk.h"

#include "cvfVector3.h"

#include <optional>

//==================================================================================================
/// Index layout of a nested hybrid grid (see RigNestedHybridGridReconstructor).
///
/// The flat main grid embeds each original (coarse) cell (OI, OJ, OK) (0-based) at the flat position
/// (OI, OJ, OK * kFactor), where kFactor is the global K refinement factor. Flat columns outside the
/// original I/J extent hold the scattered refined cells (or padding) and have no original IJK.
///
/// This class is the single place defining that convention, used both when reconstructing the LGRs
/// and when mapping cells back to their original IJK (e.g. for IJK range filters).
//==================================================================================================
class RigNestedHybridGridLayout
{
public:
    RigNestedHybridGridLayout( const cvf::Vec3st& originalCellCounts, size_t kFactor );

    const cvf::Vec3st& originalCellCounts() const;
    size_t             kFactor() const;

    // Original IJK of a flat main grid cell, or nullopt if the flat cell lies outside the original grid.
    std::optional<caf::VecIjk0> originalIjkFromFlatIjk( const caf::VecIjk0& flatIjk ) const;

    // Flat main grid IJK of the (host) cell representing an original cell.
    caf::VecIjk0 flatIjkFromOriginalIjk( const caf::VecIjk0& originalIjk ) const;

private:
    cvf::Vec3st m_originalCellCounts;
    size_t      m_kFactor;
};
