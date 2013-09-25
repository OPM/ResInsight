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

#include "cvfObject.h"
#include "cvfArray.h"
#include "cvfMatrix4.h"

namespace cvf {



//==================================================================================================
//
// Abstract base class for building geometry using the Builder pattern
//
//==================================================================================================
class GeometryBuilder : public Object
{
public:
    virtual void    setTotalVertexCountHint(size_t totalVertexCountHint);

    virtual uint    addVertices(const Vec3fArray& vertices) = 0;
    virtual uint    vertexCount() const = 0;
    virtual void    transformVertexRange(uint startIdx, uint endIdx, const Mat4f& mat) = 0;

    virtual void    addTriangle(uint i0, uint i1, uint i2) = 0;
    virtual void    addTriangles(const UIntArray& indices);
    virtual void    addTriangles(const IntArray& indices);
    virtual void    addTriangleFan(const UIntArray& indices);
    virtual void    addTriangleStrip(const UIntArray& indices);
    virtual void    addTriangleByVertices(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2);
    
    virtual void    addQuad(uint i0, uint i1, uint i2, uint i3);
    virtual void    addQuads(const UIntArray& indices);
    virtual void    addQuads(const IntArray& indices);
    virtual void    addQuadStrip(const UIntArray& indices);
    virtual void    addQuadByVertices(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, const Vec3f& v3);

    virtual void    addFace(const UIntArray& indices);
};


}
