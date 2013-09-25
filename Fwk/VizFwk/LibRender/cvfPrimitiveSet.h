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
#include "cvfOpenGLTypes.h"

namespace cvf {

class OpenGLContext;


enum PrimitiveType
{
    PT_POINTS = 100,
    PT_LINES,
    PT_LINE_LOOP,
    PT_LINE_STRIP,
    PT_TRIANGLES,
    PT_TRIANGLE_STRIP,
    PT_TRIANGLE_FAN
};



//==================================================================================================
//
// 
//
//==================================================================================================
class PrimitiveSet : public Object
{
public:
    PrimitiveSet(PrimitiveType primitiveType);

    PrimitiveType   primitiveType() const;
    cvfGLenum       primitiveTypeOpenGL() const;

    size_t          triangleCount() const;
    size_t          faceCount() const;

    void            getFaceIndices(size_t indexOfFace, UIntArray* indices) const;

    virtual void    render(OpenGLContext* oglContext) const = 0;

    virtual void    createUploadBufferObjectsGPU(OpenGLContext* oglContext) = 0;
    virtual void    releaseBufferObjectsGPU() = 0;

    virtual size_t  indexCount() const = 0;
    virtual uint    index(size_t i) const = 0;

private:
    PrimitiveType   m_primitiveType;
};


}
