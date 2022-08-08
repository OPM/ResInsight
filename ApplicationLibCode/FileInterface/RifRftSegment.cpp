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
/// segnxt : Int ID for the next segment
/// brno   : Branch ID number
/// brnst  : Branch ID number for start of segment
/// brnen  : Branch ID number for end of segment
/// segNo  : Segment ID number
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
std::vector<int> RifRftSegment::tubingBranchIds() const
{
    std::vector<int> filteredBranchIds;

    for ( auto branchId : branchIds() )
    {
        if ( m_branchType.count( branchId ) )
        {
            if ( m_branchType.at( branchId ) == RiaDefines::RftBranchType::RFT_TUBING )
            {
                filteredBranchIds.push_back( branchId );
            }
        }
    }

    return filteredBranchIds;
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
int RifRftSegment::branchIdForOneBasedBranchIndex( int oneBasedBranchIndex ) const
{
    for ( auto branchIdIndex : m_oneBasedBranchIndexMap )
    {
        if ( branchIdIndex.second == oneBasedBranchIndex ) return branchIdIndex.first;
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifRftSegment::oneBasedBranchIndexForBranchId( int branchId ) const
{
    if ( m_oneBasedBranchIndexMap.count( branchId ) > 0 ) return m_oneBasedBranchIndexMap.at( branchId );

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifRftSegmentData* RifRftSegment::segmentData( int segmentNumber ) const
{
    for ( const auto& segData : m_topology )
    {
        if ( segData.segNo() == segmentNumber ) return &segData;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifRftSegmentData> RifRftSegment::deviceBranchCandidates( int segmentNumber, int oneBasedBranchIndex )
{
    std::vector<RifRftSegmentData> deviceCandidates;

    auto alreadyAssignedBranchIds = tubingBranchIds();

    for ( auto& segData : m_topology )
    {
        if ( segData.segNo() < segmentNumber ) continue;

        auto it = std::find( alreadyAssignedBranchIds.begin(), alreadyAssignedBranchIds.end(), segData.segBrno() );
        if ( it != alreadyAssignedBranchIds.end() )
        {
            return deviceCandidates;
        }

        deviceCandidates.push_back( segData );
        setOneBasedBranchIndex( segData.segBrno(), oneBasedBranchIndex );

        setBranchType( segData.segBrno(), RiaDefines::RftBranchType::RFT_DEVICE );
    }

    return deviceCandidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRftSegment::setBranchLength( int branchId, double length )
{
    m_branchLength[branchId] = length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRftSegment::setBranchType( int branchId, RiaDefines::RftBranchType branchType )
{
    m_branchType[branchId] = branchType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRftSegment::setOneBasedBranchIndex( int branchId, int oneBasedBranchIndex )
{
    m_oneBasedBranchIndexMap[branchId] = oneBasedBranchIndex;
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
