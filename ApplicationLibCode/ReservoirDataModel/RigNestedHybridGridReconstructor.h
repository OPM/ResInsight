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

#include <map>
#include <vector>

class RigEclipseCaseData;
class RigMainGrid;
class RigGridBase;
class RigLocalGrid;

//==================================================================================================
/// Reconstructs a true LGR (RigLocalGrid) hierarchy for an Eclipse "nested hybrid grid".
///
/// A nested hybrid grid is a single flat EGRID where the refined cells of each coarse cell are
/// appended to the end of the I axis (in per-level I bands) at higher resolution, and connected to
/// the coarse grid via NNCs. The refined cells are scattered in the flat cell array, so they cannot
/// directly back a RigLocalGrid (which requires a contiguous cell block). This class rebuilds each
/// refined region as a contiguous RigLocalGrid appended to the end of the global cell/node arrays,
/// copying the real refined geometry, linking each region to its parent (coarse) cell, and hiding
/// the original scattered cells.
///
/// The parent of each refined cell is provided explicitly by sidecar properties (no HOSTNUM):
///   - REFINE                 : per-cell nesting level (1 = unrefined base, 2/3/4 = refined levels)
///   - OLDI/OLDJ/OLDK         : the cell's parent COARSE cell IJK (1-based)
///   - TMPI/TMPJ/TMPK         : the cell's local position in refined coordinate space
///
/// The coarse host cell is the collapsed flat cell at (OLDI-1, OLDJ-1, (OLDK-1)*KF), where the K
/// refinement factor KF = NZ / coarseNZ. Refinement is per-level and non-uniform, footprints may be
/// non-rectangular (holes), and level-(n+1) regions nest inside a single level-n cell.
//==================================================================================================
class RigNestedHybridGridReconstructor
{
public:
    // Per flat cell sidecar arrays (each of length == main grid cell count). refine may be empty.
    struct NestedHybridInput
    {
        std::vector<int> refine; // 1 = base, 2.. = refinement level
        std::vector<int> oldI; // 1-based coarse parent I
        std::vector<int> oldJ; // 1-based coarse parent J
        std::vector<int> oldK; // 1-based coarse parent K
        std::vector<int> tmpI; // local refined I
        std::vector<int> tmpJ; // local refined J
        std::vector<int> tmpK; // local refined K
    };

    // Returns true if at least one refined region was reconstructed.
    static bool reconstruct( RigEclipseCaseData* caseData, const NestedHybridInput& input, QString* errorMessage = nullptr );

private:
    // Build one RigLocalGrid of the given IJK dimensions. boxToFlat maps each local cell to its
    // source flat cell (UNDEFINED for holes); boxToParent maps each local cell to the parent (coarse)
    // cell index it subdivides in parentGrid. The LGR may span many parent cells (a CARFIN-style
    // refinement of a coarse block); each parent that owns a real cell gets its subGrid set. Appends
    // cells and nodes to the global arrays and records source cells. Returns the created grid.
    static RigLocalGrid* buildLocalGrid( RigEclipseCaseData*        caseData,
                                         RigGridBase*               parentGrid,
                                         int                        gridId,
                                         const QString&             gridName,
                                         const cvf::Vec3st&         dims,
                                         const std::vector<size_t>& boxToFlat,
                                         const std::vector<size_t>& boxToParent,
                                         std::map<size_t, size_t>&  sourceCells,
                                         bool                       synthesizeHoleParentGeometry = false );

    // Build the nested LGR(s) for a level that refines another (parent) level rather than the coarse
    // grid (e.g. level 4 inside level 3). cellToParent maps each resolved cell (flat index) to its
    // immediate parent cell (parent grid + parent-grid-local cell index). Cells are grouped by parent
    // grid and merged into connected regions; one LGR is created per region, placed inside its parent
    // grid (true LGR-in-LGR, any depth).
    static void buildNestedLevel( RigEclipseCaseData*                                      caseData,
                                  const std::vector<size_t>&                               cells,
                                  const NestedHybridInput&                                 input,
                                  const std::map<size_t, std::pair<RigGridBase*, size_t>>& cellToParent,
                                  int&                                                     nextGridId,
                                  std::map<size_t, size_t>&                                sourceCells );

    static void updateActiveCellInfo( RigEclipseCaseData* caseData, const std::map<size_t, size_t>& sourceCells );

    // Extend full-length (all-cells) result arrays so the new LGR cells get values copied from their
    // source flat cell. origCellCount is the cell count before any LGR cells were appended.
    static void extendFullLengthResults( RigEclipseCaseData* caseData, size_t origCellCount, const std::map<size_t, size_t>& sourceCells );

    // Re-point existing (file) NNC connections that referenced the original scattered refined cells
    // to the new LGR cells, preserving transmissibilities (connection results are indexed by position).
    static void repointNncConnections( RigEclipseCaseData* caseData, size_t origCellCount, const std::map<size_t, size_t>& sourceCells );
};
