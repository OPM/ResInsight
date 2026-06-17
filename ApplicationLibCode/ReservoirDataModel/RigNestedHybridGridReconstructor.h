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

#include <QString>

#include <vector>

class RigEclipseCaseData;
class RigMainGrid;
class RigLocalGrid;

//==================================================================================================
/// Reconstructs a true LGR (RigLocalGrid) hierarchy for an Eclipse "nested hybrid grid".
///
/// A nested hybrid grid is a single flat EGRID where refined cells are appended to the end of the
/// I (or J) axis and are connected to the coarse grid via NNCs. The refined cells are scattered in
/// the flat cell array, so they cannot directly back a RigLocalGrid (which requires a contiguous
/// cell block). This class rebuilds the refined region(s) as contiguous RigLocalGrid blocks
/// appended to the end of the global cell/node arrays, copying the real refined geometry, linking
/// each refined cell to its parent coarse cell, and hiding the original scattered cells.
///
/// The parent of each refined cell is provided explicitly by a per-cell HOSTNUM property (the
/// 1-based natural index of the parent coarse cell, 0 for non-refined cells), because the parent
/// coarse cells are collapsed to zero volume and cannot be found geometrically.
//==================================================================================================
class RigNestedHybridGridReconstructor
{
public:
    // hostNum: one value per main-grid cell. For refined cells, the 1-based natural index of the
    // parent coarse cell (i + j*nx + k*nx*ny + 1). For non-refined cells, 0.
    // Returns true if at least one refined region was reconstructed.
    static bool reconstruct( RigEclipseCaseData* caseData, const std::vector<int>& hostNum, QString* errorMessage = nullptr );

private:
    struct RegionInfo
    {
        cvf::Vec3st blockOrigin; // index origin of the refined block in the flat grid
        cvf::Vec3st blockDims; // refined block dimensions (= LGR cell counts)
        cvf::Vec3st parentOrigin; // index origin of the parent coarse region
        cvf::Vec3st refinement; // per-axis refinement factor (blockDims / parentDims)
    };

    static bool detectSingleRegion( const std::vector<int>& hostNum, const RigMainGrid* grid, RegionInfo* region, QString* errorMessage );

    static RigLocalGrid*
        buildLocalGrid( RigEclipseCaseData* caseData, const RegionInfo& region, int gridId, std::vector<size_t>& lgrToFlatCell );

    static void updateActiveCellInfo( RigEclipseCaseData* caseData, const RigLocalGrid* localGrid, const std::vector<size_t>& lgrToFlatCell );

    // Extend full-length (all-cells) result arrays so the new LGR cells get values copied from
    // their source flat cell. Active-cell-indexed results need no change (handled via result index).
    static void extendFullLengthResults( RigEclipseCaseData* caseData, size_t lgrCellStart, const std::vector<size_t>& lgrToFlatCell );

    // Re-point existing (file) NNC connections that referenced the original scattered refined cells
    // to the new LGR cells, preserving transmissibilities (connection results are indexed by position).
    static void repointNncConnections( RigEclipseCaseData* caseData, size_t lgrCellStart, const std::vector<size_t>& lgrToFlatCell );
};
