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
#include "cvfBufferObjectManaged.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::BufferObjectManaged
/// \ingroup Render
///
/// Class for handling OpenGL Buffer Objects managed by OpenGLResourceManager
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BufferObjectManaged::BufferObjectManaged()
:   m_target(0),
    m_bufferId(0),
    m_sizeInBytes(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BufferObjectManaged::~BufferObjectManaged()
{
    // Should already be deleted by manager through explicit call to deleteBuffer()
    CVF_ASSERT(m_bufferId == 0);
}


//--------------------------------------------------------------------------------------------------
/// Static method for creating new BufferObjectManaged object instances
/// 
/// The function will call glGenBuffers() to create the buffer object (BO) name, and then use 
/// glBufferData() to allocate memory and copy the passed data to the BO.
/// 
/// \return Returns NULL if an error occurred (out of memory)
//--------------------------------------------------------------------------------------------------
ref<BufferObjectManaged> BufferObjectManaged::create(OpenGLContext* oglContext, cvfGLenum target, size_t sizeInBytes, const void* data)
{
    CVF_CALLSITE_OPENGL(oglContext);

    OglId id = 0;
    glGenBuffers(1, &id);
    if (id == 0)
    {
        return NULL;
    }

    glBindBuffer(target, id);

    // Allocate memory for buffer object AND copy data into it
    // Currently we always use the GL_STATIC_DRAW usage hint for our buffers!
    glBufferData(target, static_cast<GLsizeiptr>(sizeInBytes), data, GL_STATIC_DRAW);

    // Do we want to unbind after each call?
    glBindBuffer(target, 0);

    // Should we always do a check here? 
    // Only way to detect out-of-memory issues, but not very performance friendly. Is there a better way of detecting errors for this case?
    cvfGLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        glDeleteBuffers(1, &id);
        return NULL;
    }

    BufferObjectManaged* bo = new BufferObjectManaged;
    bo->m_target = target;
    bo->m_bufferId = id;
    bo->m_sizeInBytes = sizeInBytes;

    return bo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool BufferObjectManaged::isUploaded() const
{
    if (m_bufferId != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool BufferObjectManaged::bindBuffer(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (m_bufferId != 0)
    {
        glBindBuffer(m_target, m_bufferId);

        // We can probably skip OpenGL error checking here.
        // The only errors resulting from glBindBuffer relate to illegal use of target and calling between glBegin()/glEnd()
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BufferObjectManaged::unbindBuffer(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);
    glBindBuffer(m_target, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BufferObjectManaged::unbindAllBuffers(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t BufferObjectManaged::byteCount() const
{
    return m_sizeInBytes;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BufferObjectManaged::deleteBuffer(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (m_bufferId != 0)
    {
        glDeleteBuffers(1, &m_bufferId);
        m_bufferId = 0;
    }

    m_sizeInBytes = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool BufferObjectManaged::supportedOpenGL(OpenGLContext* oglContext)
{
    // We'll require baseline support even if the actual requirement is only OpenGL 1.5
    return oglContext->capabilities()->supportsOpenGL2();
}

} // namespace cvf

