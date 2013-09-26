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
#include "cvfGeometryBuilderFaceList.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::GeometryBuilderFaceList
/// \ingroup Geometry
///
/// Builds geometry represented as a face list.
///
/// A face list is an array of integers. The first integer is the number of connectivity indices for
/// the first face, followed by an integer for each vertex int the face, which are indices into 
/// the vertex array. For example, if the face list contains (3 0 1 2), then a triangle is formed 
/// from the first three points in the vertex array. The next entry in the face list starts another 
/// face, and so on.
/// 
/// A face list representing a geometry with two triangles, one quad and finally one line would 
/// look something like this:
/// \code
///   3, 0, 1, 2,
///   3, 0, 2, 3,
///   4, 4, 5, 6, 7,
///   2, 8, 9
/// \endcode
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
GeometryBuilderFaceList::GeometryBuilderFaceList()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint GeometryBuilderFaceList::addVertices(const Vec3fArray& vertices)
{
    uint numExistingVerts = static_cast<uint>(m_vertices.size());
    uint numNewVerts = static_cast<uint>(vertices.size());

    uint i;
    for (i = 0; i < numNewVerts; i++)
    {
        m_vertices.push_back(vertices[i]);
    }

    return numExistingVerts;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint GeometryBuilderFaceList::vertexCount() const
{
    return static_cast<uint>(m_vertices.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilderFaceList::transformVertexRange(uint startIdx, uint endIdx, const Mat4f& mat)
{
    uint i;
    for (i = startIdx; i <= endIdx; i++)
    {
        m_vertices[i].transformPoint(mat);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilderFaceList::addTriangle(uint i0, uint i1, uint i2)
{
    m_faceList.push_back(3);
    m_faceList.push_back(i0);
    m_faceList.push_back(i1);
    m_faceList.push_back(i2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilderFaceList::addQuad(uint i0, uint i1, uint i2, uint i3)
{
    m_faceList.push_back(4);
    m_faceList.push_back(i0);
    m_faceList.push_back(i1);
    m_faceList.push_back(i2);
    m_faceList.push_back(i3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> GeometryBuilderFaceList::vertices() const
{
    ref<Vec3fArray> verts = new Vec3fArray(m_vertices);
    return verts;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<UIntArray> GeometryBuilderFaceList::faceList() const
{
    ref<UIntArray> fList = new UIntArray(m_faceList);
    return fList;
}


} // namespace cvf

