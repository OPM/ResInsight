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
#include "cvfVector3.h"

namespace cvf {

class BoundingBox;
class Ray;
class OpenGLResourceManager;
class ShaderProgram;
class MatrixState;
class HitDetail;
class OpenGLContext;



//==================================================================================================
//
// Drawable is the base class for drawable items in a part.
//
//==================================================================================================
class Drawable : public Object
{
public:
    Drawable();

    virtual void        render(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState& matrixState) = 0;
    virtual void        renderFixedFunction(OpenGLContext* oglContext, const MatrixState& matrixState) = 0;
    virtual void        renderImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState) = 0;
    
    virtual void        createUploadBufferObjectsGPU(OpenGLContext* oglContext) = 0;
    virtual void        releaseBufferObjectsGPU() = 0;

    virtual size_t      vertexCount() const = 0;
    virtual size_t      triangleCount() const = 0;
    virtual size_t      faceCount() const = 0;

    virtual BoundingBox boundingBox() const = 0;

    virtual bool        rayIntersectCreateDetail(const Ray& ray, Vec3d* intersectionPoint, ref<HitDetail>* hitDetail) const = 0;
};

}
