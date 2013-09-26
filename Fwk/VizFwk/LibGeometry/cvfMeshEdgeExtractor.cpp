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
#include "cvfMeshEdgeExtractor.h"
#include "cvfEdgeKey.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::MeshEdgeExtractor
/// \ingroup Geometry
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MeshEdgeExtractor::MeshEdgeExtractor()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MeshEdgeExtractor::addPrimitives(uint verticesPerPrimitive, const uint* indices, size_t indexCount)
{
    CVF_ASSERT(verticesPerPrimitive > 0);
    CVF_ASSERT(indices);
    CVF_ASSERT(indexCount > 0);

    // Points will never become edges
    if (verticesPerPrimitive < 2)
    {
        return;
    }

    size_t numPrimitives = indexCount/verticesPerPrimitive;

    size_t ip;
    for (ip = 0; ip < numPrimitives; ip++)
    {
        size_t firstIdxInPrimitive = ip*verticesPerPrimitive;

        uint i;
        for (i = 0; i < verticesPerPrimitive; i++)
        {
            uint vertexIdx1 = indices[firstIdxInPrimitive + i];
            uint vertexIdx2 = (i < verticesPerPrimitive - 1) ? indices[firstIdxInPrimitive + i + 1] : indices[firstIdxInPrimitive];

            // Don't add collapsed edges
            if (vertexIdx1 != vertexIdx2)
            {
                int64 edgeKeyVal = EdgeKey(vertexIdx1, vertexIdx2).toKeyVal();
                m_edgeSet.insert(edgeKeyVal);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MeshEdgeExtractor::addPrimitives(uint verticesPerPrimitive, const UIntArray& indices)
{
    CVF_ASSERT(verticesPerPrimitive > 0);

    size_t indexCount = indices.size();
    size_t numPrimitives = indexCount/verticesPerPrimitive;
    if (numPrimitives > 0)
    {
        const uint* indexPtr = indices.ptr();
        addPrimitives(verticesPerPrimitive, indexPtr, indexCount);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MeshEdgeExtractor::addFaceList(const UIntArray& faceList)
{
    size_t numFaceListEntries = faceList.size();

    size_t i = 0;
    while (i < numFaceListEntries)
    {
        uint numVerticesInFace = faceList[i++];
        CVF_ASSERT(numVerticesInFace > 0);
        CVF_ASSERT(i + numVerticesInFace <= numFaceListEntries);

        const uint* indexPtr = &faceList[i];
        addPrimitives(numVerticesInFace, indexPtr, numVerticesInFace);

        i += numVerticesInFace;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<UIntArray> MeshEdgeExtractor::lineIndices() const
{
    ref<UIntArray> indices = new UIntArray;

    size_t numEdges = m_edgeSet.size();
    if (numEdges == 0)
    {
        return indices;
    }

    indices->reserve(2*numEdges);

    std::set<cvf::int64>::const_iterator it;
    for (it = m_edgeSet.begin(); it != m_edgeSet.end(); ++it)
    {
        EdgeKey ek = EdgeKey::fromkeyVal(*it);
        indices->add(ek.index1());
        indices->add(ek.index2());
    }

    return indices;
}


} // namespace cvf

