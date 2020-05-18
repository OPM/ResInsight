/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RigNncConnection.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection::RigConnection()
    : m_c1GlobIdx( cvf::UNDEFINED_UINT )
    , m_c2GlobIdx( cvf::UNDEFINED_UINT )
    , m_c1Face( static_cast<unsigned char>( cvf::StructGridInterface::NO_FACE ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection::RigConnection( unsigned                           c1GlobIdx,
                              unsigned                           c2GlobIdx,
                              cvf::StructGridInterface::FaceType c1Face,
                              const std::vector<cvf::Vec3f>&     polygon )
    : m_c1GlobIdx( c1GlobIdx )
    , m_c2GlobIdx( c2GlobIdx )
    , m_c1Face( static_cast<unsigned char>( c1Face ) )
    , m_polygon( polygon )
{
    if ( c1GlobIdx >= c2GlobIdx )
    {
        // Ensure the smallest cell index is the first index
        // TODO : The face type is related to cell 1, so face should be changed to opposite face
        // This causes visual artifacts for some models, so this change will require more investigation
        m_c1GlobIdx = c2GlobIdx;
        m_c2GlobIdx = c1GlobIdx;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection::RigConnection( size_t                             c1GlobIdx,
                              size_t                             c2GlobIdx,
                              cvf::StructGridInterface::FaceType c1Face,
                              const std::vector<cvf::Vec3f>&     polygon )
    : m_c1GlobIdx( static_cast<unsigned>( c1GlobIdx ) )
    , m_c2GlobIdx( static_cast<unsigned>( c2GlobIdx ) )
    , m_c1Face( static_cast<unsigned char>( c1Face ) )
    , m_polygon( polygon )
{
    CVF_ASSERT( c1GlobIdx < std::numeric_limits<unsigned>::max() && c2GlobIdx < std::numeric_limits<unsigned>::max() );

    if ( c1GlobIdx >= c2GlobIdx )
    {
        // Ensure the smallest cell index is the first index
        // TODO : The face type is related to cell 1, so face should be changed to opposite face
        // This causes visual artifacts for some models, so this change will require more investigation
        m_c1GlobIdx = static_cast<unsigned>( c2GlobIdx );
        m_c2GlobIdx = static_cast<unsigned>( c1GlobIdx );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection::RigConnection( const RigConnection& rhs )
    : m_c1GlobIdx( rhs.m_c1GlobIdx )
    , m_c2GlobIdx( rhs.m_c2GlobIdx )
    , m_c1Face( rhs.m_c1Face )
    , m_polygon( rhs.m_polygon )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection& RigConnection::operator=( const RigConnection& rhs )
{
    m_c1GlobIdx = rhs.m_c1GlobIdx;
    m_c2GlobIdx = rhs.m_c2GlobIdx;
    m_c1Face    = rhs.m_c1Face;
    m_polygon   = rhs.m_polygon;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigConnection::hasCommonArea() const
{
    return m_polygon.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigConnection::operator==( const RigConnection& rhs )
{
    return m_c1GlobIdx == rhs.m_c1GlobIdx && m_c2GlobIdx == rhs.m_c2GlobIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigConnection::operator<( const RigConnection& other ) const
{
    if ( m_c1GlobIdx != other.m_c1GlobIdx )
    {
        return m_c1GlobIdx < other.m_c1GlobIdx;
    }

    return ( m_c2GlobIdx < other.m_c2GlobIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigConnection& RigConnectionContainer::operator[]( size_t i ) const
{
    return m_connections[i];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection& RigConnectionContainer::operator[]( size_t i )
{
    return m_connections[i];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::push_back( const RigConnection& connection )
{
    m_connections.push_back( connection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::push_back( const RigConnectionContainer& other )
{
    m_connections.insert( m_connections.end(), other.m_connections.begin(), other.m_connections.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigConnectionContainer::size() const
{
    return m_connections.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::clear()
{
    m_connections.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigConnectionContainer::empty() const
{
    return m_connections.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::remove_duplicates()
{
    std::sort( m_connections.begin(), m_connections.end() );
    m_connections.erase( std::unique( m_connections.begin(), m_connections.end() ), m_connections.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::reserve( size_t requiredSize )
{
    if ( m_connections.capacity() < requiredSize )
    {
        m_connections.reserve( requiredSize );
    }
}
