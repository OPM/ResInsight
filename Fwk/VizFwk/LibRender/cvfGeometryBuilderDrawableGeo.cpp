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
#include "cvfGeometryBuilderDrawableGeo.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::GeometryBuilderDrawableGeo
/// \ingroup Render
///
/// Builds geometry represented as a drawable geo.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
GeometryBuilderDrawableGeo::GeometryBuilderDrawableGeo()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint GeometryBuilderDrawableGeo::addVertices(const Vec3fArray& vertices)
{
    uint numExistingVerts = static_cast<uint>(m_vertices.size());

    size_t numNewVertices = vertices.size();
    size_t i;
    for (i = 0; i < numNewVertices; i++)
    {
        m_vertices.push_back(vertices[i]);
    }

    return numExistingVerts;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint GeometryBuilderDrawableGeo::vertexCount() const
{
    return static_cast<uint>(m_vertices.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilderDrawableGeo::transformVertexRange(uint startIdx, uint endIdx, const Mat4f& mat)
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
void GeometryBuilderDrawableGeo::addTriangle(uint i0, uint i1, uint i2)
{
    m_faceList.push_back(3);
    m_faceList.push_back(i0);
    m_faceList.push_back(i1);
    m_faceList.push_back(i2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilderDrawableGeo::addQuad(uint i0, uint i1, uint i2, uint i3)
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
/*
void GeometryBuilderDrawableGeo::addQuadByVertices(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, const Vec3f& v3)
{
    uint numExistingVerts = static_cast<uint>(m_vertices.size());

    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);

    m_faceList.push_back(3);
    m_faceList.push_back(numExistingVerts);
    m_faceList.push_back(numExistingVerts + 1);
    m_faceList.push_back(numExistingVerts + 2);

    m_faceList.push_back(3);
    m_faceList.push_back(numExistingVerts);
    m_faceList.push_back(numExistingVerts + 2);
    m_faceList.push_back(numExistingVerts + 3);
}
*/


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> GeometryBuilderDrawableGeo::drawableGeo() const
{
    ref<DrawableGeo> geo = new DrawableGeo;

    if (m_vertices.size() > 0)
    {
        ref<Vec3fArray> newVertexArray = new Vec3fArray(m_vertices);
        geo->setVertexArray(newVertexArray.p());
    }

    size_t numFaceListEntries = m_faceList.size();
    if (numFaceListEntries > 0)
    {
        // Need to cast away the const pointer, but should be safe since the UIntArray only has a short life in this scope
        const uint* dataPtr = &m_faceList[0];
        UIntArray faceList;
        faceList.setSharedPtr(const_cast<uint*>(dataPtr), numFaceListEntries);

        geo->setFromFaceList(faceList);
    }
    
    geo->computeNormals();

    return geo;
}


} // namespace cvf

