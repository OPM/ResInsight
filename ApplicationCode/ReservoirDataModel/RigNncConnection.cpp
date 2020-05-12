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

#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection::RigConnection()
    : m_c1GlobIdx( cvf::UNDEFINED_SIZE_T )
    , m_c2GlobIdx( cvf::UNDEFINED_SIZE_T )
    , m_c1Face( cvf::StructGridInterface::NO_FACE )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnection::RigConnection( size_t                             c1GlobIdx,
                              size_t                             c2GlobIdx,
                              cvf::StructGridInterface::FaceType c1Face,
                              const std::vector<cvf::Vec3d>&     polygon )
    : m_c1GlobIdx( c1GlobIdx )
    , m_c2GlobIdx( c2GlobIdx )
    , m_c1Face( c1Face )
    , m_polygon( polygon )
{
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
RigConnection& RigConnection::operator=( RigConnection& rhs )
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
RigConnection RigConnectionContainer::operator[]( size_t i ) const
{
    const auto& globIndices = m_globalIndices[i];
    return RigConnection( globIndices.first, globIndices.second, m_faces[i], m_polygons[i] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t>& RigConnectionContainer::indexPair( size_t i )
{
    return m_globalIndices[i];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::StructGridInterface::FaceType& RigConnectionContainer::face( size_t i )
{
    return m_faces[i];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d>& RigConnectionContainer::polygon( size_t i )
{
    return m_polygons[i];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::push_back( const RigConnection& connection )
{
    m_globalIndices.push_back( std::make_pair( connection.m_c1GlobIdx, connection.m_c2GlobIdx ) );
    m_faces.push_back( connection.m_c1Face );
    m_polygons.push_back( connection.m_polygon );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::insert( const RigConnectionContainer& other )
{
    m_globalIndices.insert( m_globalIndices.end(), other.m_globalIndices.begin(), other.m_globalIndices.end() );
    m_faces.insert( m_faces.end(), other.m_faces.begin(), other.m_faces.end() );
    m_polygons.insert( m_polygons.end(), other.m_polygons.begin(), other.m_polygons.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigConnectionContainer::size() const
{
    return m_globalIndices.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigConnectionContainer::clear()
{
    m_globalIndices.clear();
    m_faces.clear();
    m_polygons.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigConnectionContainer::empty() const
{
    return m_globalIndices.empty();
}
