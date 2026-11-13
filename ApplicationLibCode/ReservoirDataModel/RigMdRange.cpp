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

#include "RigMdRange.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMdRange::RigMdRange( double startMD, double endMD )
    : start( std::min( startMD, endMD ) )
    , end( std::max( startMD, endMD ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMdRange::overlaps( double startMD, double endMD ) const
{
    const double lower = std::min( startMD, endMD );
    const double upper = std::max( startMD, endMD );

    // A zero length range is a point, and is overlapped by any interval containing the point
    if ( start == end ) return lower <= start && start <= upper;

    return upper > start && lower < end;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigMdRange> RigMdRange::merged( const std::vector<RigMdRange>& ranges )
{
    if ( ranges.empty() ) return std::nullopt;

    RigMdRange result = ranges.front();
    for ( const auto& range : ranges )
    {
        result.start = std::min( result.start, range.start );
        result.end   = std::max( result.end, range.end );
    }
    return result;
}
