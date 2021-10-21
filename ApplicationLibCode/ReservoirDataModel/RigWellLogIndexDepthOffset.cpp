/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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
#include "RigWellLogIndexDepthOffset.h"

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogIndexDepthOffset::setIndexOffsetDepth( int kIndex, double topMd, double bottomMd, double topTvd, double bottomTvd )
{
    m_mdOffsets[kIndex]  = std::pair<double, double>( topMd, bottomMd );
    m_tvdOffsets[kIndex] = std::pair<double, double>( topTvd, bottomTvd );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellLogIndexDepthOffset::getTopMd( int kIndex ) const
{
    auto hit = m_mdOffsets.find( kIndex );
    if ( hit != m_mdOffsets.end() )
    {
        return hit->second.first;
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellLogIndexDepthOffset::getBottomMd( int kIndex ) const
{
    auto hit = m_mdOffsets.find( kIndex );
    if ( hit != m_mdOffsets.end() )
    {
        return hit->second.second;
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellLogIndexDepthOffset::getTopTvd( int kIndex ) const
{
    auto hit = m_tvdOffsets.find( kIndex );
    if ( hit != m_tvdOffsets.end() )
    {
        return hit->second.first;
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellLogIndexDepthOffset::getBottomTvd( int kIndex ) const
{
    auto hit = m_tvdOffsets.find( kIndex );
    if ( hit != m_tvdOffsets.end() )
    {
        return hit->second.second;
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RigWellLogIndexDepthOffset::sortedIndexes() const
{
    std::vector<int> indexes;
    for ( auto m : m_mdOffsets )
    {
        indexes.push_back( m.first );
    }

    std::sort( indexes.begin(), indexes.end() );
    return indexes;
}
