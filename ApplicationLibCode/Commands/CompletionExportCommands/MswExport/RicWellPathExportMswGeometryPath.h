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

#include "RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswSegment.h"
#include "CompletionsMsw/RigMswTableData.h"

#include <optional>
#include <vector>

#include <QDateTime>

class RimEclipseCase;
class RimWellPath;
class RigMainGrid;

//--------------------------------------------------------------------------------------------------
/// Free functions that implement the direct geometry path for MSW export.
/// These build WELSEGS/COMPSEGS/valve segment data directly from well-path geometry
//--------------------------------------------------------------------------------------------------
namespace RicWellPathExportMswGeometryPath
{

std::vector<RigMswBranch> buildLateralBranches( RimEclipseCase*                               eclipseCase,
                                                const RimWellPath*                            wellPath,
                                                const RigMainGrid*                            mainGrid,
                                                int                                           outletSegNum,
                                                RicWellPathExportMswTableData::CompletionType completionType,
                                                const std::optional<QDateTime>&               exportDate,
                                                int&                                          segmentNumber,
                                                int&                                          branchNumber,
                                                RiaDefines::EclipseUnitSystem                 unitSystem );

RigMswWellExportData buildMswWellExportData( RimEclipseCase*                               eclipseCase,
                                             const RimWellPath*                            wellPath,
                                             double                                        maxSegmentLength,
                                             const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                             RicWellPathExportMswTableData::CompletionType completionType,
                                             const std::optional<QDateTime>&               exportDate );

RigMswTableData collectTableData( const RigMswWellExportData& exportData, RiaDefines::EclipseUnitSystem unitSystem );

} // namespace RicWellPathExportMswGeometryPath
