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

#include "RigNestedHybridGridReconstructor.h"

#include <QString>

#include <vector>

//==================================================================================================
/// Converts between the OLDIJK/TMP sidecar description of a nested hybrid grid and the compact
/// FIPNEST/FIPSLOT/REFINE parent-child encoding intended to travel inside the INIT file (#14510).
///
/// Encoding (all arrays full-length, one value per flat cell in natural order):
///   - FIPNEST : 0 for unrefined (level-1) cells; otherwise the 1-based flat natural index of the
///               cell's IMMEDIATE parent - the coarse host cell for a primary-level cell, or the
///               (collapsed) flat slot of the host one level shallower for a nested cell. A
///               referenced host slot that is itself refined away carries its own FIPNEST value
///               (its coarse host), so every chain terminates at a FIPNEST == 0 cell.
///   - FIPSLOT : 1 + offI + 100*offJ + 10000*offK, the cell's 0-based position within its
///               immediate parent; 0 where FIPNEST is 0.
///   - REFINE  : the per-cell nesting level, unchanged from the existing sidecar/result.
///
/// FIPNEST alone is not sufficient to rebuild the grid: the refinement level is not the parent
/// chain depth (several levels can refine the coarse grid directly), and within-parent placement
/// is ambiguous from flat coordinates when a level has hole layers. REFINE and FIPSLOT carry
/// exactly those two missing pieces.
///
/// The decoder synthesizes a NestedHybridInput that reproduces the sidecar input up to per-level
/// constant TMP shifts, which RigNestedHybridGridReconstructor::reconstruct() is invariant to -
/// reusing the reconstructor unchanged is what guarantees that a grid imported through FIPNEST
/// equals the grid imported through the sidecars.
///
/// Known prototype limitations (beyond the L5+ one above):
///   - The decoder infers the K refinement factor as the gcd of the chain roots' flat K positions.
///     If every refined host happens to sit on a common coarse-K stride (e.g. only every other
///     layer hosts a refinement), the gcd overestimates the factor and the coarse K axis is
///     compressed self-consistently; the reconstruction still places every parent at its correct
///     flat cell, but LGR K padding can differ from the sidecar import. Storing the factor
///     explicitly would remove the ambiguity.
///   - Cells the encoder cannot express (unresolvedRefinedCells) stay flat on the FIPNEST path,
///     while the sidecar path may still place them - the two imports only match exactly when the
///     encoder reports zero unresolved cells.
//==================================================================================================
class RigNestedHybridGridFipnestCodec
{
public:
    struct ParentChildArrays
    {
        std::vector<int> fipnest;
        std::vector<int> fipslot;
        size_t           unresolvedRefinedCells = 0;
    };

    // Compute the FIPNEST/FIPSLOT arrays from a sidecar input, mirroring the reconstructor's own
    // level classification and parent resolution. Refined cells whose parent cannot be encoded are
    // left at 0 and counted in unresolvedRefinedCells.
    static ParentChildArrays
        computeParentChildArrays( const RigNestedHybridGridReconstructor::NestedHybridInput& input, size_t nx, size_t ny, size_t nz );

    // Synthesize a reconstruction input from the FIPNEST/FIPSLOT/REFINE arrays. Appends any
    // non-fatal observations to warnings (one per line) if given.
    static RigNestedHybridGridReconstructor::NestedHybridInput buildInputFromParentChildArrays( const std::vector<int>& fipnest,
                                                                                                const std::vector<int>& fipslot,
                                                                                                const std::vector<int>& refine,
                                                                                                size_t                  nx,
                                                                                                size_t                  ny,
                                                                                                size_t                  nz,
                                                                                                QString* warnings = nullptr );

    static int  packSlot( int offI, int offJ, int offK );
    static void unpackSlot( int slot, int& offI, int& offJ, int& offK );
};
