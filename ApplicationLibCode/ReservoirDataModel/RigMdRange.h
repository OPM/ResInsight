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

#include <optional>
#include <vector>

//==================================================================================================
/// A measured depth interval along a well path. Immutable, and start <= end is guaranteed by the constructor.
//==================================================================================================
class RigMdRange
{
public:
    RigMdRange( double startMD, double endMD );

    // True if the ranges have a part in common. Ranges that only touch at an endpoint do not overlap, except when
    // one of the ranges is a single point (zero length), which overlaps any range containing the point.
    bool overlaps( const RigMdRange& other ) const;

    // The smallest range covering all ranges, including the gaps between them. No value if ranges is empty.
    static std::optional<RigMdRange> merged( const std::vector<RigMdRange>& ranges );

    double start() const;
    double end() const;

private:
    double m_start;
    double m_end;
};
