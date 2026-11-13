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
    : m_start( std::min( startMD, endMD ) )
    , m_end( std::max( startMD, endMD ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMdRange::overlaps( const RigMdRange& other ) const
{
    // A zero length range is a point, and is overlapped by any range containing the point
    if ( m_start == m_end ) return other.m_start <= m_start && m_start <= other.m_end;
    if ( other.m_start == other.m_end ) return m_start <= other.m_start && other.m_start <= m_end;

    return other.m_end > m_start && other.m_start < m_end;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigMdRange> RigMdRange::merged( const std::vector<RigMdRange>& ranges )
{
    if ( ranges.empty() ) return std::nullopt;

    double minStart = ranges.front().start();
    double maxEnd   = ranges.front().end();
    for ( const auto& range : ranges )
    {
        minStart = std::min( minStart, range.start() );
        maxEnd   = std::max( maxEnd, range.end() );
    }
    return RigMdRange( minStart, maxEnd );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigMdRange::start() const
{
    return m_start;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigMdRange::end() const
{
    return m_end;
}
