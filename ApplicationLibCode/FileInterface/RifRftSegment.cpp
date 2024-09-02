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

#include "cvfAssert.h"

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
int RifRftSegment::oneBasedBranchIndexForBranchId( int branchId ) const
{
    if ( m_oneBasedBranchIndexMap.count( branchId ) > 0 ) return m_oneBasedBranchIndexMap.at( branchId );

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, int> RifRftSegment::branchIdsAndOneBasedBranchIndices( RiaDefines::RftBranchType branchType ) const
{
    std::map<int, int> mapForBranchType;

    // find all branch ids for the given branch type

    for ( const auto& [branchId, branchIndex] : m_oneBasedBranchIndexMap )
    {
        if ( branchType == RiaDefines::RftBranchType::RFT_UNKNOWN || this->branchType( branchId ) == branchType )
        {
            mapForBranchType[branchId] = branchIndex;
        }
    }

    return mapForBranchType;
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
const RifRftSegmentData* RifRftSegment::segmentDataByIndex( int segmentIndex ) const
{
    CVF_ASSERT( segmentIndex < static_cast<int>( m_topology.size() ) );

    return &( m_topology[segmentIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRftSegment::createDeviceBranch( int deviceBranchFirstSegmentNumber, int oneBasedBranchIndex, const std::vector<double>& seglenstValues )
{
    double lastAssignedDeviceBranchDepth = -1.0;
    for ( auto& segData : m_topology )
    {
        if ( segData.segNo() < deviceBranchFirstSegmentNumber ) continue;

        auto branchNumber = segData.segBrno();
        if ( branchType( branchNumber ) != RiaDefines::RftBranchType::RFT_UNKNOWN ) return;

        auto segmentIndex = segmentIndexFromSegmentNumber( segData.segNo() );
        if ( segmentIndex < 0 ) continue;

        double candidateSegmentDepth = seglenstValues[segmentIndex];
        if ( lastAssignedDeviceBranchDepth > -1.0 && lastAssignedDeviceBranchDepth > candidateSegmentDepth ) return;

        lastAssignedDeviceBranchDepth = candidateSegmentDepth;
        setOneBasedBranchIndex( segData.segBrno(), oneBasedBranchIndex );

        setBranchType( segData.segBrno(), RiaDefines::RftBranchType::RFT_DEVICE );
    }
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
RiaDefines::RftBranchType RifRftSegment::branchType( int branchId ) const
{
    if ( m_branchType.count( branchId ) ) return m_branchType.at( branchId );

    return RiaDefines::RftBranchType::RFT_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifRftSegment::segmentIndicesForBranchNumber( int branchNumber ) const
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifRftSegment::segmentIndicesForBranchIndex( int branchIndex, RiaDefines::RftBranchType branchType ) const
{
    std::vector<size_t> v;
    for ( size_t i = 0; i < m_topology.size(); i++ )
    {
        if ( branchIndex <= 0 )
        {
            v.push_back( i );
            continue;
        }

        auto segment = m_topology[i];

        auto it = m_oneBasedBranchIndexMap.find( segment.segBrno() );
        if ( it != m_oneBasedBranchIndexMap.end() )
        {
            if ( it->second == branchIndex && m_branchType.at( segment.segBrno() ) == branchType )
            {
                v.push_back( i );
            }
        }
    }

    return v;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifRftSegment::packerSegmentIndicesOnAnnulus( int branchIndex ) const
{
    auto segmentIndices = segmentIndicesForBranchIndex( branchIndex, RiaDefines::RftBranchType::RFT_ANNULUS );

    std::vector<size_t> packerSegmentIndices;

    for ( auto segmentIndex : segmentIndices )
    {
        auto segment              = m_topology[segmentIndex];
        auto outflowSegmentNumber = segment.segNext();

        auto candidateSegment = segmentData( outflowSegmentNumber );
        if ( !candidateSegment ) continue;

        auto candidateBranchType = branchType( candidateSegment->segBrno() );
        if ( candidateBranchType == RiaDefines::RftBranchType::RFT_DEVICE )
        {
            packerSegmentIndices.push_back( segmentIndex );
        }
    }

    return packerSegmentIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifRftSegment::segmentNumbersForBranchIndex( int oneBasedBranchIndex, RiaDefines::RftBranchType branchType ) const
{
    std::vector<int> v;

    auto indices = segmentIndicesForBranchIndex( oneBasedBranchIndex, branchType );
    for ( auto index : indices )
    {
        v.push_back( m_topology[index].segNo() );
    }

    return v;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RifRftSegment::uniqueOneBasedBranchIndices( RiaDefines::RftBranchType branchType ) const
{
    std::set<int> indices;

    for ( const auto& [branchId, branchIndex] : m_oneBasedBranchIndexMap )
    {
        indices.insert( branchIndex );
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifRftSegment::segmentIndexFromSegmentNumber( int segmentNumber ) const
{
    for ( size_t i = 0; i < m_topology.size(); i++ )
    {
        auto segment = m_topology[i];

        if ( segment.segNo() == segmentNumber ) return static_cast<int>( i );
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifRftSegment::nonContinuousDeviceSegmentIndices( int branchIndex ) const
{
    auto deviceSegmentNumbers = segmentNumbersForBranchIndex( branchIndex, RiaDefines::RftBranchType::RFT_DEVICE );
    if ( deviceSegmentNumbers.size() < 2 ) return {};

    // Find all device segments that are connected to a tubing segment with a upstream tubing segment that is not connected to a device
    // segment.
    //
    // Device numbers :  50  51          52  53
    //                    |   |           |   |
    // Tubing numbers :   1 - 2 - 3 - 4 - 5 - 6
    //
    // Device 53 is connected to tubing 5. There are tubing segments between device 52 and 51. We mark device 52 as non-continuous
    // device segment.

    std::vector<size_t> deviceSegmentIndices;
    size_t              i = deviceSegmentNumbers.size() - 1;
    while ( i > 1 )
    {
        auto currentDeviceSegment = segmentData( deviceSegmentNumbers[i] );
        if ( !currentDeviceSegment ) continue;

        auto upstreamDeviceSegment = segmentData( deviceSegmentNumbers[i - 1] );
        if ( !upstreamDeviceSegment ) continue;

        auto tubingSegData = segmentData( currentDeviceSegment->segNext() );
        if ( !tubingSegData ) continue;

        auto upstreamTubingSegmentNumber = tubingSegData->segNext();
        if ( upstreamDeviceSegment->segNext() != upstreamTubingSegmentNumber )
        {
            deviceSegmentIndices.push_back( segmentIndexFromSegmentNumber( deviceSegmentNumbers[i] ) );
        }

        i--;
    }

    return deviceSegmentIndices;
}
