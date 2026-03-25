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

#include "CompletionsMsw/RigMswSegment.h"
#include "RiaDefines.h"
#include "RigActiveCellInfo.h"
#include "Well/RigWellLogExtractor.h"

#include <optional>
#include <set>
#include <string>
#include <vector>

#include <QDateTime>

class RimEclipseCase;
class RimWellPath;
class RigMainGrid;
class RimPerforationInterval;

//--------------------------------------------------------------------------------------------------
/// Free functions that build RigMswSegment lists directly from well-path geometry and Rim
/// completion objects — without creating the intermediate RicMswBranch / RicMswItem tree.
///
/// Extracted from the former `internal` anonymous namespace in RicWellPathExportMswGeometryPath.cpp
/// so that they can be called from other translation units and unit-tested in isolation.
//--------------------------------------------------------------------------------------------------
namespace RicMswBranchBuilder
{

//--------------------------------------------------------------------------------------------------
/// Mapping from a sub-segment's MD range to its main-bore segment number.
/// One entry is emitted per sub-segment (a cell split into N sub-segments produces N entries).
/// Used by valve, fracture, and lateral builders to locate the outlet segment for a given MD.
//--------------------------------------------------------------------------------------------------
struct CellSegmentEntry
{
    double cellStartMD;
    double cellEndMD;
    int    lastSubSegmentNumber;
};

//--------------------------------------------------------------------------------------------------
/// Convert a WellPathCellIntersectionInfo global-cell index to a RigMswCellIntersection (1-based i,j,k).
/// Returns std::nullopt for gap-segments (globCellIndex >= totalCellCount).|
//--------------------------------------------------------------------------------------------------
std::optional<RigMswCellIntersection>
    toMswCellIntersection( const WellPathCellIntersectionInfo& cellInfo, const RigMainGrid* mainGrid, double distanceStart, double distanceEnd );

//--------------------------------------------------------------------------------------------------
/// Find the main-bore outlet segment number for a given measured depth.
/// Returns 1 (heel) if cellSegMap is empty; returns the last entry's segment number if md is
/// beyond the end of all mapped cells.
//--------------------------------------------------------------------------------------------------
int findOutletSegmentForMD( const std::vector<CellSegmentEntry>& cellSegMap, double md );

//--------------------------------------------------------------------------------------------------
/// Build main-bore WELSEGS segments directly from well-path geometry.
/// For each grid-cell intersection overlapping a bare perforation (no active valve) a COMPSEGS
/// entry is embedded.  Optionally fills cellSegMap for later valve outlet-segment lookups.
//--------------------------------------------------------------------------------------------------
RigMswBranch buildMainBoreBranchFromGeometry( const RimWellPath*                                wellPath,
                                              const std::vector<WellPathCellIntersectionInfo>&  filteredIntersections,
                                              const RigMainGrid*                                mainGrid,
                                              const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                              const std::set<const RimPerforationInterval*>&    valvedIntervals,
                                              const std::string&                                infoType,
                                              double                                            heelMD,
                                              double                                            heelTVD,
                                              int                                               branchNumber,
                                              int&                                              segmentNumber,
                                              int                                               outletSegmentNumber,
                                              double                                            maxSegmentLength,
                                              const std::vector<std::pair<double, double>>&     customSegmentIntervals,
                                              const std::optional<QDateTime>&                   exportDate,
                                              RiaDefines::EclipseUnitSystem                     unitSystem,
                                              std::vector<CellSegmentEntry>*                    cellSegMap,
                                              const RigActiveCellInfo*                          activeCellInfo = nullptr );

//--------------------------------------------------------------------------------------------------
/// Build WELSEGS + COMPSEGS segments for ICD/ICV/AICD/SICD valve completions.
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildValveBranchesFromGeometry( const RimWellPath*                                wellPath,
                                                          const std::vector<WellPathCellIntersectionInfo>&  filteredIntersections,
                                                          const RigMainGrid*                                mainGrid,
                                                          const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                          const std::vector<CellSegmentEntry>&              cellSegMap,
                                                          const std::string&                                infoType,
                                                          const std::string&                                wellNameForExport,
                                                          int&                                              segmentNumber,
                                                          int&                                              branchNumber,
                                                          double                                            maxSegmentLength,
                                                          const std::vector<std::pair<double, double>>&     customSegmentIntervals,
                                                          const std::optional<QDateTime>&                   exportDate,
                                                          RiaDefines::EclipseUnitSystem                     unitSystem );

//--------------------------------------------------------------------------------------------------
/// Build WELSEGS + COMPSEGS segments for fracture completions.
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildFractureBranchesFromGeometry( RimEclipseCase*                      eclipseCase,
                                                             const RimWellPath*                   wellPath,
                                                             const RigMainGrid*                   mainGrid,
                                                             const std::vector<CellSegmentEntry>& cellSegMap,
                                                             const std::string&                   infoType,
                                                             int&                                 segmentNumber,
                                                             int&                                 branchNumber );

//--------------------------------------------------------------------------------------------------
/// Build WELSEGS + COMPSEGS + WSEGVALV segments for fishbones completions.
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildFishbonesBranchesFromGeometry( const RimEclipseCase*                            eclipseCase,
                                                              const RimWellPath*                               wellPath,
                                                              const RigMainGrid*                               mainGrid,
                                                              const std::vector<WellPathCellIntersectionInfo>& filteredIntersections,
                                                              const std::vector<CellSegmentEntry>&             cellSegMap,
                                                              const std::string&                               infoType,
                                                              const std::string&                               wellNameForExport,
                                                              int&                                             segmentNumber,
                                                              int&                                             branchNumber,
                                                              RiaDefines::EclipseUnitSystem                    unitSystem );

} // namespace RicMswBranchBuilder
