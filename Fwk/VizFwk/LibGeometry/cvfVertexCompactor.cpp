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
#include "cvfVertexCompactor.h"
#include "cvfGeometryUtils.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::VertexCompactor
/// \ingroup Geometry
///
/// Worker class for creating compact indices based on a "global" index array
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VertexCompactor::VertexCompactor(const UIntValueArray& origIndices, const Vec3fValueArray& origVertexArray)
:   m_origIndices(origIndices),
    m_origVertexArray(origVertexArray)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexCompactor::computeCompactedIndices()
{
    m_newIndices = new UIntArray;
    m_newToOldVertexIndexMapping = new UIntArray;
    uint maxNumResultingVertices = static_cast<uint>(m_origVertexArray.size());
    GeometryUtils::removeUnusedVertices(m_origIndices, m_newIndices.p(), m_newToOldVertexIndexMapping.p(), maxNumResultingVertices);
}


//--------------------------------------------------------------------------------------------------
/// Get indices into new vertex array
///
/// \return  Newly allocated array with the new indices. An array is always returned, but may have zero entries.
//--------------------------------------------------------------------------------------------------
ref<UIntArray> VertexCompactor::indices()
{
    if (m_newIndices.isNull())
    {
        computeCompactedIndices();
    }

    CVF_ASSERT(m_newIndices.notNull());
    return m_newIndices;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> VertexCompactor::vertexArray()
{
    if (m_newIndices.isNull())
    {
        computeCompactedIndices();
    }

    CVF_ASSERT(m_newToOldVertexIndexMapping.notNull());
    ref<Vec3fArray> vertices = new Vec3fArray;
    size_t numNewVertices = m_newToOldVertexIndexMapping->size();
    if (numNewVertices > 0)
    {
        vertices->reserve(numNewVertices);
        size_t i;
        for (i = 0; i < numNewVertices; i++)
        {
            vertices->add(m_origVertexArray.val(m_newToOldVertexIndexMapping->get(i)));
        }
    }

    return vertices;
}


//--------------------------------------------------------------------------------------------------
/// Get array of original indices per new vertex. 
/// 
/// The returned array will be the same size as the array of vertices returned by vertexArray().
/// The indices are the original/source indices of the vertices
//--------------------------------------------------------------------------------------------------
ref<UIntArray> VertexCompactor::perVertexOriginalIndices()
{
    if (m_newToOldVertexIndexMapping.isNull())
    {
        computeCompactedIndices();
    }

    CVF_ASSERT(m_newToOldVertexIndexMapping.notNull());
    return m_newToOldVertexIndexMapping;
}

} // namespace cvf
