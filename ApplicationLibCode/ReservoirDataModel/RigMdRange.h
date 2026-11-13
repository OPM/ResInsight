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

#include <optional>
#include <vector>

//==================================================================================================
/// A measured depth interval along a well path. start <= end is guaranteed by the constructor.
//==================================================================================================
struct RigMdRange
{
    RigMdRange( double startMD, double endMD );

    // True if the interval [startMD, endMD] has a part in common with this range. Intervals that only touch the
    // range at an endpoint do not overlap, except when the range is a single point.
    bool overlaps( double startMD, double endMD ) const;

    // The smallest range covering all ranges, including the gaps between them. No value if ranges is empty.
    static std::optional<RigMdRange> merged( const std::vector<RigMdRange>& ranges );

    double start;
    double end;
};
