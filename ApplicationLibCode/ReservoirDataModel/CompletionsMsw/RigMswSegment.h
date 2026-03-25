/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RigMswTableRows.h"

#include <optional>
#include <string>
#include <vector>

// clang-format off


//==================================================================================================
/// Grid-cell intersection for a single MSW segment.
/// Used to populate the COMPSEGS (main grid) and COMPSEGL (LGR sub-grid) tables.
//==================================================================================================
struct RigMswCellIntersection
{
    size_t      i, j, k;        // Grid cell IJK indices (1-based)
    double      distanceStart;  // Distance from well heel to start of intersection [m or ft]
    double      distanceEnd;    // Distance from well heel to end of intersection [m or ft]
    std::string gridName;       // Empty for main grid; LGR name for sub-grids (COMPSEGL)
};

//==================================================================================================
/// Primary building block for the MSW export.
/// Each RigMswSegment corresponds to exactly one row in the WELSEGS table.
/// Cell intersections (COMPSEGS/COMPSEGL) and optional valve data are embedded.
/// The branch number lives on the containing RigMswBranch, not here.
//==================================================================================================
struct RigMswSegment
{
    // WELSEGS fields
    int    segmentNumber;        // ISEG1 / ISEG2
    int    outletSegmentNumber;  // ISEG3 (parent/outlet segment)

    double                length;    // LENGTH (incremental or absolute MD)
    double                depth;     // DEPTH  (incremental or absolute TVD)
    std::optional<double> diameter;  // ID      (liner inner diameter)
    std::optional<double> roughness; // EPSILON (roughness factor)

    std::string description;     // Comment shown in WELSEGS output
    std::string sourceWellName;  // Name of the source well path object

    // COMPSEGS / COMPSEGL: grid-cell intersections for this segment
    std::vector<RigMswCellIntersection> intersections;

    // Valve data — at most one type per segment
    std::optional<WsegvalvRow> wsegvalvData;  // WSEGVALV: ICV / ICD valves
    std::optional<WsegaicdRow> wsegaicdData;  // WSEGAICD: Autonomous ICD valves
    std::optional<WsegsicdRow> wsegsicdData;  // WSEGSICD: Spiral ICD valves
};


//==================================================================================================
/// One branch in the MSW export.
/// All segments share the same IBRANCH number, which is stored here rather than per-segment.
/// An optional tie-in valve segment (ICV) may appear at the start of lateral branches.
//==================================================================================================
struct RigMswBranch
{
    int                          branchNumber;  // IBRANCH for all segments in this branch
    std::optional<RigMswSegment> tieInValve;    // Optional ICV at the tie-in point (laterals only)
    std::vector<RigMswSegment>   segments;      // Segments of this branch
};


//==================================================================================================
/// Complete pre-computed MSW export data for one well.
/// Contains all information needed to write WELSEGS, COMPSEGS, and valve tables without
/// any further tree traversal.
//==================================================================================================
struct RigMswWellExportData
{
    WelsegsHeader               header;    // WELSEGS well-level header
    std::vector<RigMswBranch>   branches;  // One entry per branch
};

// clang-format on
