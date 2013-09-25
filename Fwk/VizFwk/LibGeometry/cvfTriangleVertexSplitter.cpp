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
#include "cvfTriangleVertexSplitter.h"
#include "cvfFrustum.h"
#include "cvfOutlineEdgeExtractor.h"

namespace cvf {

//==================================================================================================
///
/// \class cvf::TriangleVertexSplitter
/// \ingroup Geometry
///
/// This class takes a triangle mesh and duplicates nodes on edges where the normal of the two triangles 
/// on the edge differ more than the given crease angle.
/// 
/// The vertices are also compacted, so the returned vertexArray() contains only the vertices
/// referenced by the triangles.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TriangleVertexSplitter::TriangleVertexSplitter(double creaseAngle, const cvf::UIntValueArray& origTriangleIndices, const cvf::Vec3fValueArray& origVertexArray)
:   m_creaseAngle(creaseAngle),
    m_origTriangleIndices(origTriangleIndices),
    m_origVertexArray(origVertexArray),
    m_isComputed(false)
{
}


//--------------------------------------------------------------------------------------------------
/// Returns the new connectivity table for the triangles. Same size as the input origTriangleIndices 
/// array
//--------------------------------------------------------------------------------------------------
ref<UIntArray> TriangleVertexSplitter::triangleIndices()
{
    if (!m_isComputed)
    {
        splitVertices();
    }

    return m_triangleIndices;
}


//--------------------------------------------------------------------------------------------------
/// Returns the new vertex array containg only the referenced nodes in the triangle mesh, but with
/// split nodes wherever needed.
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> TriangleVertexSplitter::vertexArray()
{
    if (!m_isComputed)
    {
        splitVertices();
    }

    ref<Vec3fArray> output = new Vec3fArray(m_vertexArray);

    return output;
}


//--------------------------------------------------------------------------------------------------
/// Returns an array with vertex indices into the input origVertexArray for each vertex in the 
/// output vertexArray(). Used to keep track of the original index of the compacted and split output
/// array.
//--------------------------------------------------------------------------------------------------
ref<UIntArray> TriangleVertexSplitter::perVertexOriginalIndices()
{
    if (!m_isComputed)
    {
        splitVertices();
    }

    size_t numUsedVertices = m_vertexArray.size();
    ref<UIntArray> output = new UIntArray;

    if (numUsedVertices == 0)
    {
        return output;
    }

    output->resize(numUsedVertices);
    output->setAll(UNDEFINED_UINT);

    size_t numOrigVertices = m_origVertexArray.size();

    size_t i;
    for (i = 0; i < numOrigVertices; i++)
    {
        uint usedIdx = m_origToUsedNodeMap[i];

        if (usedIdx != UNDEFINED_UINT)
        {
            output->set(usedIdx, static_cast<uint>(i));
        }
    }

    for (i = 0; i < numUsedVertices; i++)
    {
        if (output->get(i) != UNDEFINED_UINT)
        {
            uint origIndex = output->get(i);
            uint nextSplit = m_nextSplitVertexIdx[i];
            m_nextSplitVertexIdx[i] = UNDEFINED_UINT;

            while (nextSplit != UNDEFINED_UINT)
            {
                CVF_TIGHT_ASSERT(output->get(nextSplit) == UNDEFINED_UINT);

                output->set(nextSplit, origIndex);

                uint nextSplitThis = nextSplit;
                nextSplit = m_nextSplitVertexIdx[nextSplit];
                m_nextSplitVertexIdx[nextSplitThis] = UNDEFINED_UINT;
            }
        }
    }

    return output;
}


//--------------------------------------------------------------------------------------------------
/// Returns an array of smooth per vertex normals corresponding to the output vertexArray().
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> TriangleVertexSplitter::vertexNormals()
{
    if (!m_isComputed)
    {
        splitVertices();
    }

    size_t numVertices = m_normalArray.size();
    ref<Vec3fArray> output = new Vec3fArray;

    if (numVertices == 0)
    {
        return output;
    }

    output->reserve(numVertices);

    size_t i;
    for (i = 0; i < numVertices; i++)
    {
        output->add(m_normalArray[i].getNormalized());
    }

    return output;
}


//--------------------------------------------------------------------------------------------------
/// Worker. Split the vertices that needs splitting and compact the vertexArray and indices.
//--------------------------------------------------------------------------------------------------
void TriangleVertexSplitter::splitVertices()
{
    CVF_ASSERT(!m_isComputed);
    m_isComputed = true;

    // Handle empty data;
    if (m_origTriangleIndices.size() == 0)
    {
        m_triangleIndices = new UIntArray;

        return;
    }

    size_t origVertexCount = m_origVertexArray.size();
    m_triangleIndices = new UIntArray(m_origTriangleIndices);

    m_vertexArray.reserve(origVertexCount);
    m_normalArray.reserve(origVertexCount);
    m_nextSplitVertexIdx.reserve(origVertexCount);

    m_origToUsedNodeMap.resize(origVertexCount);
    m_origToUsedNodeMap.setAll(UNDEFINED_UINT);

    size_t origConnIndex = 0;
    size_t numTris = m_triangleIndices->size() / 3;
    size_t tri;
    for (tri = 0; tri < numTris; tri++)
    {
        uint c0 = m_origTriangleIndices.val(origConnIndex);
        uint c1 = m_origTriangleIndices.val(origConnIndex + 1);
        uint c2 = m_origTriangleIndices.val(origConnIndex + 2);

        // Compute normal
        Vec3f v0 = m_origVertexArray.val(c0);
        Vec3f v1 = m_origVertexArray.val(c1);
        Vec3f v2 = m_origVertexArray.val(c2);

        Vec3f normal = (v1 - v0) ^ (v2 - v0);
        normal.normalize();

        uint newConn1 = processVertex(c0, normal);
        uint newConn2 = processVertex(c1, normal);
        uint newConn3 = processVertex(c2, normal);

        m_triangleIndices->set(origConnIndex, newConn1);
        m_triangleIndices->set(origConnIndex + 1, newConn2);
        m_triangleIndices->set(origConnIndex + 2, newConn3);

        origConnIndex += 3;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the new index of the given vertex based on if the vertex will be split or not.
//--------------------------------------------------------------------------------------------------
uint TriangleVertexSplitter::processVertex(uint origVertexIndex, const Vec3f& faceNormal)
{
    uint vertexIndex = m_origToUsedNodeMap[origVertexIndex];

    if (vertexIndex == UNDEFINED_UINT)
    {
        vertexIndex = static_cast<uint>(m_vertexArray.size());
        m_origToUsedNodeMap[origVertexIndex] = vertexIndex;

        m_vertexArray.push_back(m_origVertexArray.val(origVertexIndex));
        m_normalArray.push_back(faceNormal);
        m_nextSplitVertexIdx.push_back(UNDEFINED_UINT);

        return vertexIndex;
    }
    
    uint outputIndex = vertexIndex;

    for(;;)
    {
        const Vec3f& vertexNormal = m_normalArray[outputIndex].getNormalized();
        
        if (isNormalDifferenceBelowThreshold(faceNormal, vertexNormal))
        {
            m_normalArray[outputIndex] += faceNormal;
            return outputIndex;
        }
        else
        {
            uint nextVertexIndex = m_nextSplitVertexIdx[outputIndex];

            if (nextVertexIndex == UNDEFINED_UINT)
            {
                // Cannot average with any existing normal in orig or new split vertices
                uint newVertexIndex = static_cast<uint>(m_vertexArray.size());

                m_nextSplitVertexIdx[outputIndex] = newVertexIndex;
                outputIndex = newVertexIndex;

                m_vertexArray.push_back(m_vertexArray[vertexIndex]);
                m_normalArray.push_back(faceNormal);
                m_nextSplitVertexIdx.push_back(UNDEFINED_UINT);

                return outputIndex;
            }

            outputIndex = nextVertexIndex;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Return true if the angle between the two normals is below the current crease angle.
//--------------------------------------------------------------------------------------------------
bool TriangleVertexSplitter::isNormalDifferenceBelowThreshold(const Vec3f& n1, const Vec3f& n2)
{
    // If either vector is 0, there is probably some trouble with the triangle
    // Return false so that it will be split
    if (n1.isZero() || n2.isZero())
    {
        return false;
    }

    // Guard acos against out-of-domain input
    const double dotProduct = Math::clamp(static_cast<double>(n1*n2), -1.0, 1.0);

    const double angle = Math::acos(dotProduct);
    if (Math::abs(angle) < m_creaseAngle)
    {
        return true;
    }

    return false;
}

} // namespace cvf
