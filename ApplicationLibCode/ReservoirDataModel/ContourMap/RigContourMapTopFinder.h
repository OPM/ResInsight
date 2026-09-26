/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include <vector>

//==================================================================================================
/// Find the N most significant tops in a gridded surface / contour map, with a minimum distance
/// between accepted tops.
///
/// Grid layout: row-major, index = i + j * nx. Undefined cells may be NaN or +/-inf.
/// Distances are computed in world units via cell sizes dx, dy (rotation does not affect distance).
//==================================================================================================
namespace RigContourMapTopFinder
{
struct Top
{
    int    i          = 0;
    int    j          = 0;
    double x          = 0.0;
    double y          = 0.0;
    double z          = 0.0;
    double prominence = 0.0;
};

struct Settings
{
    int    maxTops          = 10;
    double minDistance      = 0.0; // world units
    double minProminence    = 0.0; // z units, filters noise / shoulders
    bool   rankByProminence = true; // false: rank by z (after prominence filter)
    bool   excludeEdgeTops  = false; // drop tops touching grid border or undefined cells
};

// Returns prominence per cell: NaN for non-peak cells, >= 0 for peaks.
// Union-find sweep from highest to lowest value (8-connectivity). O(M log M).
std::vector<double> computeProminence( const std::vector<double>& z, int nx, int ny );

// Find the top N peaks, ranked and filtered according to settings.
std::vector<Top>
    findTops( const std::vector<double>& z, int nx, int ny, double dx, double dy, const Settings& settings, double originX = 0.0, double originY = 0.0 );

} // namespace RigContourMapTopFinder
