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
#include "cvfOutlineEdgeExtractor.h"
#include "cvfEdgeKey.h"
#include "cvfGeometryUtils.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::OutlineEdgeExtractor
/// \ingroup Geometry
///
/// 
///
//==================================================================================================

static const size_t OEE_OUTLINE_EDGE        = cvf::UNDEFINED_SIZE_T;        // This is an outline edge
static const size_t OEE_NON_OUTLINE_EDGE    = cvf::UNDEFINED_SIZE_T - 1;    // Marked as an interior edge
static const size_t OEE_MULTIREF_EDGE       = cvf::UNDEFINED_SIZE_T - 2;    // An edge with more than two faces connected to it.

//--------------------------------------------------------------------------------------------------
/// Constructor
/// 
/// creaseAngle is specified in radians
//--------------------------------------------------------------------------------------------------
OutlineEdgeExtractor::OutlineEdgeExtractor(double creaseAngle, const Vec3fValueArray& vertexArray)
:   m_creaseAngle(creaseAngle),
    m_vertexArray(vertexArray)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OutlineEdgeExtractor::addPrimitives(uint verticesPerPrimitive, const uint* indices, size_t indexCount)
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
        size_t myFaceIndex = m_faceNormals.size();
        size_t firstIdxInPrimitive = ip*verticesPerPrimitive;

        // Normal computation accepts points and lines, but in that case returns a zero vector
        Vec3f faceNormal = computeFaceNormal(&indices[firstIdxInPrimitive], verticesPerPrimitive);
        m_faceNormals.push_back(faceNormal);

        uint i;
        for (i = 0; i < verticesPerPrimitive; i++)
        {
            const uint vertexIdx1 = indices[firstIdxInPrimitive + i];
            const uint vertexIdx2 = (i < verticesPerPrimitive - 1) ? indices[firstIdxInPrimitive + i + 1] : indices[firstIdxInPrimitive];
            
            // Never add collapsed edges
            if (vertexIdx1 == vertexIdx2)
            {
                continue;
            }

            int64 edgeKeyVal = EdgeKey(vertexIdx1, vertexIdx2).toKeyVal();
            std::map<int64, size_t>::iterator it = m_edgeToFaceIndexMap.find(edgeKeyVal);
            if (it == m_edgeToFaceIndexMap.end())
            {
                // Not found, so add and register face index
                m_edgeToFaceIndexMap[edgeKeyVal] = myFaceIndex;
            }
            else
            {
                size_t otherFaceIdx = it->second;
                if (otherFaceIdx < OEE_MULTIREF_EDGE)
                {
                    // An edge is already there, check angle
                    if (isFaceAngleAboveThreshold(myFaceIndex, otherFaceIdx))
                    {
                        m_edgeToFaceIndexMap[edgeKeyVal] = OEE_OUTLINE_EDGE;
                    }
                    else
                    {
                        m_edgeToFaceIndexMap[edgeKeyVal] = OEE_NON_OUTLINE_EDGE;
                    }
                }
                else
                {
                    // Three or more faces share an edge
                    m_edgeToFaceIndexMap[edgeKeyVal] = OEE_MULTIREF_EDGE;
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OutlineEdgeExtractor::addPrimitives(uint verticesPerPrimitive, const UIntArray& indices)
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
void OutlineEdgeExtractor::addFaceList(const UIntArray& faceList)
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
ref<UIntArray> OutlineEdgeExtractor::lineIndices() const
{
    ref<UIntArray> indices = new UIntArray;

    size_t numEdges = m_edgeToFaceIndexMap.size();
    if (numEdges == 0)
    {
        return indices;
    }

    indices->reserve(2*numEdges);

    std::map<int64, size_t>::const_iterator it;
    for (it = m_edgeToFaceIndexMap.begin(); it != m_edgeToFaceIndexMap.end(); ++it)
    {
        size_t otherFaceIdx = it->second;
        if (otherFaceIdx != OEE_NON_OUTLINE_EDGE)
        {
            EdgeKey ek = EdgeKey::fromkeyVal(it->first);
            indices->add(ek.index1());
            indices->add(ek.index2());
        }
    }

    return indices;
}


//--------------------------------------------------------------------------------------------------
/// Returns the worker array with one normal per face (primitive)
//--------------------------------------------------------------------------------------------------
const std::vector<Vec3f>& OutlineEdgeExtractor::faceNormals()
{
    return m_faceNormals;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3f OutlineEdgeExtractor::computeFaceNormal(const uint* faceVertexIndices, uint numVerticesInFace) const
{
    // Init to zero so that points and lines return a zero normal
    Vec3f normal(Vec3f::ZERO);

    if (numVerticesInFace == 3)
    {
        const Vec3f& v0 = m_vertexArray.val(faceVertexIndices[0]);
        Vec3f v1 = m_vertexArray.val(faceVertexIndices[1]) - v0;
        Vec3f v2 = m_vertexArray.val(faceVertexIndices[2]) - v0;

        normal = v1 ^ v2;
        normal.normalize();
    }
    else if (numVerticesInFace == 4)
    {
        // Quad surface normal,
        // From "Real Time Collision Detection" p 495
        const Vec3f& A = m_vertexArray.val(faceVertexIndices[0]);
        const Vec3f& B = m_vertexArray.val(faceVertexIndices[1]);
        const Vec3f& C = m_vertexArray.val(faceVertexIndices[2]);
        const Vec3f& D = m_vertexArray.val(faceVertexIndices[3]);
        normal = GeometryUtils::quadNormal(A, B, C, D);
    }
    else if (numVerticesInFace > 4)
    {
        normal = GeometryUtils::polygonNormal(m_vertexArray, faceVertexIndices, numVerticesInFace);
    }

    return normal;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OutlineEdgeExtractor::isFaceAngleAboveThreshold(size_t faceIdx1, size_t faceIdx2) const
{
    const Vec3f& n1 = m_faceNormals[faceIdx1];
    const Vec3f& n2 = m_faceNormals[faceIdx2];

    // If either vector is 0, the face is probably a line
    // Therefore just return true to flag this as an outline edge
    if (n1.isZero() || n2.isZero())
    {
        return true;
    }

    // Guard acos against out-of-domain input
    const double dotProduct = Math::clamp(static_cast<double>(n1*n2), -1.0, 1.0);

    const double angle = Math::acos(dotProduct);
    if (Math::abs(angle) > m_creaseAngle)
    {
        return true;
    }
    else
    {
        return false;
    }
}


} // namespace cvf

