/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include <utility>
#include <vector>

class RimWellPath;

//--------------------------------------------------------------------------------------------------
/// Helper functions shared by the MSW table data export code paths.
//--------------------------------------------------------------------------------------------------
namespace RicMswTableDataTools
{

std::vector<std::pair<double, double>> createSubSegmentMDPairs( double                                        startMD,
                                                                double                                        endMD,
                                                                double                                        maxSegmentLength,
                                                                const std::vector<std::pair<double, double>>& customSegmentIntervals = {} );

double tvdFromMeasuredDepth( const RimWellPath* wellPath, double measuredDepth );

inline constexpr double valveSegmentLength = 0.1;

} // namespace RicMswTableDataTools
