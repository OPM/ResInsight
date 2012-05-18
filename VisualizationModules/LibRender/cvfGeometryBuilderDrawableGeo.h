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

#pragma once

#include "cvfGeometryBuilder.h"
#include "cvfDrawableGeo.h"

namespace cvf {


//==================================================================================================
//
// Concrete geometry builder class for building geometry to a drawable geo
//
//==================================================================================================
class GeometryBuilderDrawableGeo : public GeometryBuilder
{
public:
    GeometryBuilderDrawableGeo();
    //GeometryBuilderDrawableGeo(bool requireVerticesBeforeAdd = true);

    virtual uint        addVertices(const Vec3fArray& vertices);
    virtual uint        vertexCount() const;
    virtual void        transformVertexRange(uint startIdx, uint endIdx, const Mat4f& mat);

    virtual void        addTriangle(uint i0, uint i1, uint i2);
    virtual void        addQuad(uint i0, uint i1, uint i2, uint i3);
    //virtual void        addQuadByVertices(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, const Vec3f& v3);

    ref<DrawableGeo>    drawableGeo() const;

private:
    std::vector<Vec3f>  m_vertices;
    std::vector<uint>   m_faceList;
};

}
