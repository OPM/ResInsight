//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cvfBase.h"
#include "cvfGeometryBuilderTriangles.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::GeometryBuilderTriangles
/// \ingroup Geometry
///
/// Builds geometry represented as triangles.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
GeometryBuilderTriangles::GeometryBuilderTriangles()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint GeometryBuilderTriangles::addVertices(const Vec3fArray& vertices)
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
uint GeometryBuilderTriangles::vertexCount() const
{
    return static_cast<uint>(m_vertices.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilderTriangles::transformVertexRange(uint startIdx, uint endIdx, const Mat4f& mat)
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
void GeometryBuilderTriangles::addTriangle(uint i0, uint i1, uint i2)
{
    m_triangles.push_back(i0);
    m_triangles.push_back(i1);
    m_triangles.push_back(i2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> GeometryBuilderTriangles::vertices() const
{
    ref<Vec3fArray> verts = new Vec3fArray(m_vertices);
    return verts;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<UIntArray> GeometryBuilderTriangles::triangles() const
{
    ref<UIntArray> tris = new UIntArray(m_triangles);
    return tris;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<UShortArray> GeometryBuilderTriangles::trianglesUShort() const
{
    ref<UShortArray> tris = new UShortArray;

    tris->resize(m_triangles.size());
    size_t i;
    for (i = 0; i < m_triangles.size(); i++)
    {
        tris->set(i, static_cast<ushort>(m_triangles[i]));
    }

    return tris;
}


} // namespace cvf
