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
#include <utility>
#include <vector>

#include <QDateTime>

class RimEclipseCase;
class RimWellPath;
class RigMainGrid;
class RimPerforationInterval;

//--------------------------------------------------------------------------------------------------
/// Free functions that build RigMswSegment lists directly from well-path geometry and Rim
/// completion objects.
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
/// One grid-cell intersection of a fishbones lateral, recorded while the lateral branches are built
/// so that effective diameters can be computed once all laterals of the well are known.
///
/// A cell intersection longer than the max segment length is split into several WELSEGS rows. They
/// all belong to the same intersection and share one effective diameter, so the segment numbers are
/// kept together here.
//--------------------------------------------------------------------------------------------------
struct FishbonesLateralSegment
{
    std::vector<int> segmentNumbers;
    size_t           globalCellIndex;
    double           equivalentDiameter;
};

//--------------------------------------------------------------------------------------------------
/// The cell intersections of one fishbones lateral, in order from the ICD sub and outwards.
//--------------------------------------------------------------------------------------------------
struct FishbonesLateral
{
    std::vector<FishbonesLateralSegment> segments;
};

//--------------------------------------------------------------------------------------------------
/// One fishbones ICD sub segment, recorded with the grid cells it connects to.
//--------------------------------------------------------------------------------------------------
struct FishbonesIcdSegment
{
    int              segmentNumber;
    std::set<size_t> globalCellIndices;
};

//--------------------------------------------------------------------------------------------------
/// Data collected across all fishbones of a well, used by the apply functions below.
//--------------------------------------------------------------------------------------------------
struct FishbonesExportContext
{
    std::vector<FishbonesLateral>    laterals;
    std::vector<FishbonesIcdSegment> icdSegments;
};

//--------------------------------------------------------------------------------------------------
/// Replace the WELSEGS diameter of fishbones lateral segments with an effective diameter.
///
/// Several laterals may run through the same grid cell. The combined flow area is represented by
/// Deff = sqrt(d1^2 + d2^2 + ..) over the lateral segments in that cell, see
/// https://github.com/OPM/ResInsight/issues/7686
///
/// The first cell intersection of a lateral shares its cell with the main bore and with the first
/// intersection of every other lateral on the same sub, which inflates Deff. It therefore inherits
/// the effective diameter of the second intersection of the same lateral, see
/// https://github.com/OPM/ResInsight/issues/7731
//--------------------------------------------------------------------------------------------------
void applyEffectiveDiameters( const FishbonesExportContext& context, RigMswWellExportData& exportData );

//--------------------------------------------------------------------------------------------------
/// Replace the WSEGVALV area of fishbones ICD subs connected to the same grid cell with the sum of
/// their areas, so that the cell sees the total flow area of the ICDs completing it.
///
/// The sums are computed from the original areas only. An ICD sub reaching into more than one cell
/// reports the largest of the sums it takes part in.
//--------------------------------------------------------------------------------------------------
void applyIcdAreaPerCell( const FishbonesExportContext& context, RigMswWellExportData& exportData );

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
RigMswBranch buildMainBoreBranch( const RimWellPath*                                wellPath,
                                  const std::vector<WellPathCellIntersectionInfo>&  filteredIntersections,
                                  const RigMainGrid*                                mainGrid,
                                  const RimEclipseCase*                             eclipseCase,
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
std::vector<RigMswBranch> buildValveBranches( const RimWellPath*                                wellPath,
                                              const std::vector<WellPathCellIntersectionInfo>&  filteredIntersections,
                                              const RigMainGrid*                                mainGrid,
                                              const RimEclipseCase*                             eclipseCase,
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
std::vector<RigMswBranch> buildFractureBranches( RimEclipseCase*                      eclipseCase,
                                                 const RimWellPath*                   wellPath,
                                                 const RigMainGrid*                   mainGrid,
                                                 const std::vector<CellSegmentEntry>& cellSegMap,
                                                 const std::string&                   infoType,
                                                 int&                                 segmentNumber,
                                                 int&                                 branchNumber );

//--------------------------------------------------------------------------------------------------
/// Build WELSEGS + COMPSEGS + WSEGVALV segments for fishbones completions.
//--------------------------------------------------------------------------------------------------
std::vector<RigMswBranch> buildFishbonesBranches( const RimEclipseCase*                            eclipseCase,
                                                  const RimWellPath*                               wellPath,
                                                  const RigMainGrid*                               mainGrid,
                                                  const std::vector<WellPathCellIntersectionInfo>& filteredIntersections,
                                                  const std::vector<CellSegmentEntry>&             cellSegMap,
                                                  const std::string&                               infoType,
                                                  const std::string&                               wellNameForExport,
                                                  int&                                             segmentNumber,
                                                  int&                                             branchNumber,
                                                  double                                           maxSegmentLength,
                                                  const std::vector<std::pair<double, double>>&    customSegmentIntervals,
                                                  RiaDefines::EclipseUnitSystem                    unitSystem,
                                                  FishbonesExportContext&                          fishbonesContext );

} // namespace RicMswBranchBuilder
