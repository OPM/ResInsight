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


#include "cvfBase.h"
#include "cvfTriangleMeshEdgeExtractor.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::TriangleMeshEdgeExtractor
/// \ingroup Geometry
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TriangleMeshEdgeExtractor::TriangleMeshEdgeExtractor()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TriangleMeshEdgeExtractor::addTriangles(const UIntArray& indices, const UIntArray& triangleKeys)
{
    size_t index = 0;
    size_t numTris = indices.size()/3;
    size_t tri;
    for (tri = 0; tri < numTris; tri++)
    {
        uint key = triangleKeys[tri];

        uint v1 = indices[index++];
        uint v2 = indices[index++];
        uint v3 = indices[index++];

        addEdge(v1, v2, key);
        addEdge(v2, v3, key);
        addEdge(v3, v1, key);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TriangleMeshEdgeExtractor::addEdge(uint v1, uint v2, uint key)
{
    int64 edgeKeyVal = 0;

    if (v1 < v2)
    {
        edgeKeyVal = v2;
        edgeKeyVal <<= 32;
        edgeKeyVal += v1;
    }
    else
    {
        edgeKeyVal = v1;
        edgeKeyVal <<= 32;
        edgeKeyVal += v2;
    }

    std::map<int64, uint>::iterator it = m_edgeMap.find(edgeKeyVal);

    if (it != m_edgeMap.end())
    {
        uint foundKey = it->second;

        if (foundKey == key)
        {
            // Remove edge from map and return
            m_edgeMap.erase(it);
        }
    }
    else
    {
        m_edgeMap[edgeKeyVal] = key;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<UIntArray> TriangleMeshEdgeExtractor::lineIndices() const
{
    ref<UIntArray> indices = new UIntArray;

    size_t numEdges = m_edgeMap.size();
    if (numEdges == 0)
    {
        return indices;
    }

    indices->reserve(2*numEdges);

    std::map<cvf::int64, uint>::const_iterator it;
    for (it = m_edgeMap.begin(); it != m_edgeMap.end(); ++it)
    {
        int64 edgeKey = it->first;
        uint v1 = static_cast<uint>(edgeKey);
        uint v2 = static_cast<uint>(edgeKey>>32);

        indices->add(v1);
        indices->add(v2);
    }

    return indices;
}

} // namespace cvf
