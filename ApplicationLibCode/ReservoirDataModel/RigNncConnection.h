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

#pragma once

#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <deque>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigConnection
{
public:
    RigConnection();
    RigConnection( unsigned                           c1GlobIdx,
                   unsigned                           c2GlobIdx,
                   cvf::StructGridInterface::FaceType c1Face  = cvf::StructGridInterface::NO_FACE,
                   const std::vector<cvf::Vec3f>&     polygon = {} );

    RigConnection( size_t                             c1GlobIdx,
                   size_t                             c2GlobIdx,
                   cvf::StructGridInterface::FaceType c1Face  = cvf::StructGridInterface::NO_FACE,
                   const std::vector<cvf::Vec3f>&     polygon = {} );

    RigConnection( const RigConnection& rhs );

    RigConnection& operator=( const RigConnection& rhs );
    bool           operator==( const RigConnection& rhs ) const;
    bool           operator<( const RigConnection& other ) const;
    bool           hasCommonArea() const;

    inline size_t                             c1GlobIdx() const { return m_c1GlobIdx; }
    inline size_t                             c2GlobIdx() const { return m_c2GlobIdx; }
    inline cvf::StructGridInterface::FaceType face() const
    {
        return static_cast<cvf::StructGridInterface::FaceType>( m_c1Face );
    }
    inline void setFace( cvf::StructGridInterface::FaceType face ) { m_c1Face = static_cast<unsigned char>( face ); }
    inline const std::vector<cvf::Vec3f>& polygon() const { return m_polygon; }
    inline void                           setPolygon( const std::vector<cvf::Vec3f>& polygon ) { m_polygon = polygon; }

private:
    unsigned                m_c1GlobIdx;
    unsigned                m_c2GlobIdx;
    unsigned char           m_c1Face;
    std::vector<cvf::Vec3f> m_polygon;
};

class RigConnectionContainer
{
public:
    RigConnectionContainer() = default;

    const RigConnection& operator[]( size_t i ) const;
    RigConnection&       operator[]( size_t i );

    void   push_back( const RigConnection& connection );
    void   push_back( const RigConnectionContainer& connection );
    size_t size() const;
    void   clear();
    bool   empty() const;
    void   remove_duplicates();
    void   reserve( size_t requiredSize );

private:
    std::vector<RigConnection> m_connections;
};
