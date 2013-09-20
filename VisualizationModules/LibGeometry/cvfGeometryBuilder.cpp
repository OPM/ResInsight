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
#include "cvfGeometryBuilder.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::GeometryBuilder
/// \ingroup Geometry
///
/// Abstract base class for building geometry using the Builder pattern.
/// \sa GeometryUtils
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::setTotalVertexCountHint(size_t totalVertexCountHint)
{
    CVF_UNUSED(totalVertexCountHint);
    // Nothing here, may be used in derived classes
}


//--------------------------------------------------------------------------------------------------
/// \fn virtual int GeometryBuilder::addVertices(const Vec3fArray& vertices) = 0;
/// 
/// Add vertex coordinates
/// 
/// \return  The resulting index of the first vertex in the \a vertices array.
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// \fn virtual int GeometryBuilder::addTriangle(int i0, int i1, int i2) = 0;
/// 
/// Add a single triangle by specifying the indices into the vertex array.
/// 
/// \sa addTriangles()
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// Add multiple triangles
/// 
/// \remarks There must be at least 3 entries in the \a indices array, and the total number of 
///          entries must be a multiple of 3.   
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addTriangles(const UIntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 3);
    CVF_ASSERT(numIndices % 3 == 0);

    size_t numTriangles = numIndices/3;
    CVF_ASSERT(numTriangles >= 1);
    CVF_ASSERT(3*numTriangles == numIndices);

    size_t i;
    for (i = 0; i < numTriangles; i++)
    {
        addTriangle(indices[3*i], indices[3*i + 1], indices[3*i + 2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addTriangles(const IntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 3);
    CVF_ASSERT(numIndices % 3 == 0);

    size_t numTriangles = numIndices/3;
    CVF_ASSERT(numTriangles >= 1);
    CVF_ASSERT(3*numTriangles == numIndices);

    size_t i;
    for (i = 0; i < numTriangles; i++)
    {
        CVF_ASSERT(indices[3*i] >= 0 && indices[3*i + 1] && indices[3*i + 2]);
        addTriangle(static_cast<uint>(indices[3*i]), static_cast<uint>(indices[3*i + 1]), static_cast<uint>(indices[3*i + 2]));
    }
}


//--------------------------------------------------------------------------------------------------
/// Add a triangle fan
/// 
/// Vertex ordering for triangle fans:
/// <PRE>
///   v4 *-------* v3        Resulting triangles:
///       \     / \            t1: v0, v1, v2
///        \   /   \           t2: v0, v2, v3
///         \ /     \          t3: v0, v3, v4
///       v0 *-------* v2        
///           \     /
///            \   /
///             \ /
///              * v1 </PRE>
/// 
/// \remarks The number of entries in the \a indices array must be at least 3.
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addTriangleFan(const UIntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 3);

    size_t numTriangles = numIndices - 2;
    CVF_ASSERT(numTriangles >= 1);

    size_t i;
    for (i = 0; i < numTriangles; i++)
    {
        addTriangle(indices[0], indices[i + 1], indices[i + 2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// Add a triangle strip
/// 
/// Vertex ordering for triangle strips:
/// <PRE>
///   v0      v2      v4       Resulting triangles:
///   *-------*-------*          t1: v0, v1, v2
///    \     / \     / \         t2: v2, v1, v3
///     \   /   \   /   \        t3: v2, v3, v4
///      \ /     \ /     \       t4: v4, v3, v5
///       *-------*-------*
///      v1      v3      v5 </PRE>
/// 
/// \remarks The number of entries in the \a indices array must be at least 3.
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addTriangleStrip(const UIntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 3);

    size_t numTriangles = numIndices - 2;
    CVF_ASSERT(numTriangles >= 1);

    size_t i;
    for (i = 0; i < numTriangles; i++)
    {
        if (i % 2 == 0)
        {
            addTriangle(indices[i], indices[i + 1], indices[i + 2]);
        }
        else
        {
            addTriangle(indices[i + 1], indices[i], indices[i + 2]);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addTriangleByVertices(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2)
{
    Vec3fArray verts;
    verts.resize(3);
    verts[0] = v0;
    verts[1] = v1;
    verts[2] = v2;

    uint firstVertexIdx = addVertices(verts);

    addTriangle(firstVertexIdx, firstVertexIdx + 1, firstVertexIdx + 2);
}


//--------------------------------------------------------------------------------------------------
/// Add a single quad by specifying the indices into the vertex array
/// 
/// The default implementation will split the quad into two triangles (i0,i1,i2 and i0,i2,i3) and
/// add them using addTriangle().
/// 
/// \sa addTriangle(), addQuads()
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addQuad(uint i0, uint i1, uint i2, uint i3)
{
    addTriangle(i0, i1, i2);
    addTriangle(i0, i2, i3);
}


//--------------------------------------------------------------------------------------------------
/// Add multiple quads
/// 
/// The default implementation utilizes addQuad() to add each quad separately.
/// 
/// \remarks There must be at least 4 entries in the \a indices array, and the total number of 
///          entries must be a multiple of 4.
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addQuads(const UIntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 4);
    CVF_ASSERT(numIndices % 4 == 0);

    size_t numQuads = numIndices/4;
    CVF_ASSERT(numQuads >= 1);
    CVF_ASSERT(4*numQuads == numIndices);

    size_t i;
    for (i = 0; i < numQuads; i++)
    {
        addQuad(indices[4*i], indices[4*i + 1], indices[4*i + 2], indices[4*i + 3]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addQuads(const IntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 4);
    CVF_ASSERT(numIndices % 4 == 0);

    size_t numQuads = numIndices/4;
    CVF_ASSERT(numQuads >= 1);
    CVF_ASSERT(4*numQuads == numIndices);

    size_t i;
    for (i = 0; i < numQuads; i++)
    {
        CVF_ASSERT(indices[4*i] >= 0 && indices[4*i + 1] && indices[4*i + 2] && indices[4*i + 3]);
        addQuad(static_cast<uint>(indices[4*i]), static_cast<uint>(indices[4*i + 1]), static_cast<uint>(indices[4*i + 2]), static_cast<uint>(indices[4*i + 3]));
    }
}


//--------------------------------------------------------------------------------------------------
/// Add a quad strip
/// 
/// Vertex ordering for quad strips:
/// <PRE>
///   v0    v2    v4    v6   Resulting quads:
///   *-----*-----*-----*       q1: v0, v1, v3, v2
///   |     |     |     |       q2: v2, v3, v5, v4
///   |     |     |     |       q3: v4, v5, v7, v6
///   |     |     |     |       
///   *-----*-----*-----*
///   v1    v3    v5    v7 </PRE>
/// 
/// \remarks There must be at least 4 entries in the \a indices array, and the total number of 
///          entries must be a multiple of 2.
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addQuadStrip(const UIntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 4);
    CVF_ASSERT(numIndices % 2 == 0);

    size_t numQuads = (numIndices - 2)/2;
    CVF_ASSERT(numQuads >= 1);

    size_t i;
    for (i = 0; i < numQuads; i++)
    {
        addQuad(indices[2*i], indices[2*i + 1], indices[2*i + 3], indices[2*i + 2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addQuadByVertices(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, const Vec3f& v3)
{
    Vec3fArray verts;
    verts.resize(4);
    verts[0] = v0;
    verts[1] = v1;
    verts[2] = v2;
    verts[3] = v3;

    uint firstVertexIdx = addVertices(verts);

    addQuad(firstVertexIdx, firstVertexIdx + 1, firstVertexIdx + 2, firstVertexIdx + 3);
}


//--------------------------------------------------------------------------------------------------
/// Add one face 
/// 
/// The type of primitive added will be determined from the number of indices passed in \a indices
/// 
/// \remarks Currently, points and lines are not supported. Faces with more than 4 indices will
///          be triangulated using fanning
//--------------------------------------------------------------------------------------------------
void GeometryBuilder::addFace(const UIntArray& indices)
{
    size_t numIndices = indices.size();
    CVF_ASSERT(numIndices >= 3);

    if (numIndices == 3)
    {
        addTriangle(indices[0], indices[1], indices[2]);
    }
    else if (numIndices == 4)
    {
        addQuad(indices[0], indices[1], indices[2], indices[3]);
    }
    else
    {
        size_t numTriangles = numIndices - 2;
        size_t i;
        for (i = 0; i < numTriangles; i++)
        {
            addTriangle(indices[0], indices[i + 1], indices[i + 2]);
        }
    }
}


} // namespace cvf

