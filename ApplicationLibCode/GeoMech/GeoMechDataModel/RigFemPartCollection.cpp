/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemPartCollection.h"
#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartCollection::RigFemPartCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartCollection::~RigFemPartCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartCollection::addFemPart( RigFemPart* part )
{
    size_t globalElementOffset      = 0;
    size_t globalNodeOffset         = 0;
    size_t globalConnectivityOffset = 0;
    if ( m_femParts.size() > 0 )
    {
        size_t lastIndex = m_femParts.size() - 1;
        globalElementOffset += m_femParts[lastIndex]->elementCount();
        globalElementOffset += m_partElementOffset[lastIndex];
        globalNodeOffset += m_femParts[lastIndex]->nodes().nodeIds.size();
        globalNodeOffset += m_partNodeOffset[lastIndex];
        globalConnectivityOffset += m_femParts[lastIndex]->allConnectivitiesCount();
        globalConnectivityOffset += m_partConnectivityOffset[lastIndex];
    }

    m_femParts.push_back( part );
    m_partElementOffset.push_back( globalElementOffset );
    m_partNodeOffset.push_back( globalNodeOffset );
    m_partConnectivityOffset.push_back( globalConnectivityOffset );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPart* RigFemPartCollection::part( size_t index )
{
    return m_femParts[index].p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFemPart* RigFemPartCollection::part( size_t index ) const
{
    return m_femParts[index].p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPartCollection::partCount() const
{
    return static_cast<int>( m_femParts.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartCollection::totalElementCount() const
{
    size_t elementCount = 0;

    for ( int i = 0; i < partCount(); i++ )
    {
        elementCount += part( i )->elementCount();
    }

    return elementCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RigFemPartCollection::characteristicElementSize() const
{
    if ( partCount() )
    {
        return part( 0 )->characteristicElementSize();
    }
    else
    {
        return 0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigFemPartCollection::boundingBox() const
{
    cvf::BoundingBox bBox;
    for ( int i = 0; i < partCount(); i++ )
    {
        bBox.add( part( i )->boundingBox() );
    }
    return bBox;
}

//--------------------------------------------------------------------------------------------------
/// convert from global element index to part and part-local index
//--------------------------------------------------------------------------------------------------
std::pair<int, size_t> RigFemPartCollection::partIdAndElementIndex( size_t globalIndex ) const
{
    const size_t nParts = m_partElementOffset.size();

    CVF_ASSERT( nParts > 0 );

    for ( size_t i = 1; i < nParts; i++ )
    {
        if ( globalIndex < m_partElementOffset[i] )
        {
            return std::make_pair( (int)( i - 1 ), globalIndex - m_partElementOffset[i - 1] );
        }
    }

    return std::make_pair( (int)( nParts - 1 ), globalIndex - m_partElementOffset.back() );
}

//--------------------------------------------------------------------------------------------------
/// convert from global element index to part and part-local index
//--------------------------------------------------------------------------------------------------
std::pair<const RigFemPart*, size_t> RigFemPartCollection::partAndElementIndex( size_t globalIndex ) const
{
    auto [partId, elementIdx] = partIdAndElementIndex( globalIndex );
    return std::make_pair( part( partId ), elementIdx );
}

//--------------------------------------------------------------------------------------------------
/// convert from part and part-local index to global index
//--------------------------------------------------------------------------------------------------
size_t RigFemPartCollection::globalIndex( int partId, size_t localIndex ) const
{
    return localIndex + m_partElementOffset[partId];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPartCollection::nodeIdxFromElementNodeResultIdx( size_t globalIndex ) const
{
    const size_t nParts = m_partConnectivityOffset.size();
    CVF_ASSERT( nParts > 0 );

    int    partId  = (int)( nParts - 1 );
    size_t partIdx = globalIndex - m_partConnectivityOffset.back();

    for ( size_t i = 1; i < nParts; i++ )
    {
        if ( globalIndex < m_partConnectivityOffset[i] )
        {
            partId  = (int)( i - 1 );
            partIdx = globalIndex - m_partConnectivityOffset[i - 1];
            break;
        }
    }

    const RigFemPart* part = this->part( partId );

    return (int)m_partNodeOffset[partId] + part->nodeIdxFromElementNodeResultIdx( partIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartCollection::globalElementNodeResultIdx( int partId, int elementIdx, int elmLocalNodeIdx ) const
{
    return m_partElementOffset[partId] * 8 + part( partId )->elementNodeResultIdx( elementIdx, elmLocalNodeIdx );
}
