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
#include "cvfOpenGLTypes.h"

namespace cvf {

class OpenGLContext;


//==================================================================================================
//
// Class for buffer objects being managed by the OpenGLResourceManager
//
//==================================================================================================
class BufferObjectManaged : public Object
{
public:
    ~BufferObjectManaged();

    static ref<BufferObjectManaged> create(OpenGLContext* oglContext, cvfGLenum target, size_t sizeInBytes, const void* data);
    
    bool        isUploaded() const;

    bool        bindBuffer(OpenGLContext* oglContext) const;
    void        unbindBuffer(OpenGLContext* oglContext) const;
    static void unbindAllBuffers(OpenGLContext* oglContext);

    size_t      byteCount() const;
    void        deleteBuffer(OpenGLContext* oglContext);

    static bool supportedOpenGL(OpenGLContext* oglContext);

private:
    BufferObjectManaged();

private:
    cvfGLenum   m_target;       // The target to which the buffer object will be bound (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc)
    OglId       m_bufferId;     // The id (OpenGL name of the buffer object)  
    size_t      m_sizeInBytes;  // Size of buffer in bytes
};


}


