//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfArray.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class VertexWelder
{
public:
    VertexWelder();
    
    void            initialize(double weldingDistance, double cellSize, uint numBuckets);
    void            reserveVertices(uint vertexCount);

    uint            weldVertex(const Vec3f& vertex, bool* wasWelded);

    uint            vertexCount() const;
    const Vec3f&    vertex(uint index) const;
    ref<Vec3fArray> createVertexArray() const;

private:
    uint            getGridCellBucket(int x, int y, int z) const;
    uint            locateVertexInBucket(const Vec3f& v, uint bucket) const;
    uint            addVertexToBucket(const Vec3f& v, uint bucket);

private:
    double              m_weldEpsilon;  // Welding tolerance, radius around vertex defining welding neighborhood
    double              m_cellSize;     // Grid cell size; must be at least 2*m_weldEpsilon

    uint                m_numBuckets;   // Number of hash buckets to map grid cells into
    UIntArray           m_first;        // Start of linked list for each bucket. Number of buckets long

    std::vector<uint>   m_next;         // Links each vertex to next in linked list. Always numVertices long, will grow as vertices are added
    std::vector<Vec3f>  m_vertex;       // Unique vertices within tolerance
};


}





