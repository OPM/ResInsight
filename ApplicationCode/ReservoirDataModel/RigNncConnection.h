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
    RigConnection( size_t                             c1GlobIdx,
                   size_t                             c2GlobIdx,
                   cvf::StructGridInterface::FaceType c1Face,
                   const std::vector<cvf::Vec3d>&     polygon );
    RigConnection( const RigConnection& rhs );

    RigConnection& operator=( RigConnection& rhs );
    bool           operator<( const RigConnection& other ) const;
    bool           hasCommonArea() const;

    size_t                             m_c1GlobIdx;
    size_t                             m_c2GlobIdx;
    cvf::StructGridInterface::FaceType m_c1Face;
    std::vector<cvf::Vec3d>            m_polygon;
};

class RigConnectionContainer
{
public:
    RigConnection operator[]( size_t i ) const;

    std::pair<size_t, size_t>&          indexPair( size_t i );
    cvf::StructGridInterface::FaceType& face( size_t i );
    std::vector<cvf::Vec3d>&            polygon( size_t i );

    void   push_back( const RigConnection& connection );
    void   insert( const RigConnectionContainer& other );
    size_t size() const;
    void   clear();
    bool   empty() const;

private:
    std::vector<std::pair<size_t, size_t>>          m_globalIndices;
    std::vector<cvf::StructGridInterface::FaceType> m_faces;
    std::vector<std::vector<cvf::Vec3d>>            m_polygons;
};
