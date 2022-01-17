/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RifRftSegment.h"

#include <algorithm>
#include <unordered_set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRftSegmentData::RifRftSegmentData( int segnxt, int brno, int brnst, int brnen, int segNo )
    : m_segNext( segnxt )
    , m_segbrno( brno )
    , m_brnst( brnst )
    , m_brnen( brnen )
    , m_segmentNo( segNo )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifRftSegmentData::segNext() const
{
    return m_segNext;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifRftSegmentData::segBrno() const
{
    return m_segbrno;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifRftSegmentData::segBrnst() const
{
    return m_brnst;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifRftSegmentData::segBrnen() const
{
    return m_brnen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifRftSegmentData::segNo() const
{
    return m_segmentNo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRftSegment::setSegmentData( std::vector<RifRftSegmentData> segmentData )
{
    m_topology = segmentData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifRftSegmentData> RifRftSegment::topology() const
{
    return m_topology;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRftSegment::addResultNameAndSize( const Opm::EclIO::EclFile::EclEntry& resultNameAndSize )
{
    m_resultNameAndSize.push_back( resultNameAndSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::EclIO::EclFile::EclEntry> RifRftSegment::resultNameAndSize() const
{
    return m_resultNameAndSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifRftSegment::branchIds() const
{
    std::unordered_set<int> s;
    for ( const auto& segData : m_topology )
    {
        s.insert( segData.segBrno() );
    }

    std::vector<int> v;
    v.assign( s.begin(), s.end() );
    std::sort( v.begin(), v.end() );

    return v;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifRftSegment::indicesForBranchNumber( int branchNumber ) const
{
    std::vector<size_t> v;
    for ( size_t i = 0; i < m_topology.size(); i++ )
    {
        auto segment = m_topology[i];
        if ( branchNumber <= 0 || segment.segBrno() == branchNumber )
        {
            v.push_back( i );
        }
    }

    return v;
}
