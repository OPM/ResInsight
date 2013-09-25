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
#include "cvfVertexAttribute.h"
#include "cvfBufferObjectManaged.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::VertexAttribute
/// \ingroup Render
///
/// Abstract class for generic vertex attribute arrays
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// \fn virtual void VertexAttribute::setupAttribPointerBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, size_t strideInBytes, size_t bufferOffsetInBytes) const = 0;
/// 
/// Specifies the location within buffer object and data format of the array of the vertex attribute
/// 
/// \warning The buffer object to use must already be bound
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// \fn virtual void VertexAttribute::setupAttribPointerClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex) const = 0;
/// 
/// Sets up vertex attribute pointer from client memory
/// 
/// \warning The caller must ensure that no buffer object is currently bound
//--------------------------------------------------------------------------------------------------



//==================================================================================================
///
/// \class cvf::AttribSetupStrategyNormFloat
/// \ingroup Render
///
/// Strategy for specifying vertex attribute data as float.
/// 
/// Can be used with all component types, but will always expose the attributes using floating point
/// types in the shaders. Integer values will be converted to float by normalization. [0,1] for unsigned 
/// integer types, [-1,1] for signed integer types. Uses glVertexAttribPointer().
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Sets up vertex attribute pointer from bound buffer object
/// 
/// \warning The buffer object to use must already be bound
//--------------------------------------------------------------------------------------------------
void AttribSetupStrategyNormFloat::setupFromBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, size_t strideInBytes, size_t bufferOffsetInBytes) 
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsOpenGL2());
    glVertexAttribPointer(static_cast<GLuint>(vertexAttributeIndex), static_cast<GLint>(compCount), compTypeOpenGL, GL_TRUE, static_cast<GLsizei>(strideInBytes), CVF_OGL_BUFFER_OFFSET(bufferOffsetInBytes));
}


//--------------------------------------------------------------------------------------------------
/// Sets up vertex attribute pointer from client memory
/// 
/// \warning The caller must ensure that no buffer object is currently bound
//--------------------------------------------------------------------------------------------------
void AttribSetupStrategyNormFloat::setupFromClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, const void* ptr) 
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsOpenGL2());
    glVertexAttribPointer(static_cast<GLuint>(vertexAttributeIndex), static_cast<GLint>(compCount), compTypeOpenGL, GL_TRUE, 0, ptr);
}



//==================================================================================================
///
/// \class cvf::AttribSetupStrategyDirectFloat
/// \ingroup Render
///
/// Strategy for specifying vertex attribute data as float.
/// 
/// Can be used with all component types, but will always expose the attributes using floating point
/// types in the shaders. Integer values will be converted directly to float. Uses glVertexAttribPointer().
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Sets up vertex attribute pointer from bound buffer object
/// 
/// \warning The buffer object to use must already be bound
//--------------------------------------------------------------------------------------------------
void AttribSetupStrategyDirectFloat::setupFromBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, size_t strideInBytes, size_t bufferOffsetInBytes) 
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsOpenGL2());
    glVertexAttribPointer(static_cast<GLuint>(vertexAttributeIndex), static_cast<GLint>(compCount), compTypeOpenGL, GL_FALSE, static_cast<GLsizei>(strideInBytes), CVF_OGL_BUFFER_OFFSET(bufferOffsetInBytes));
}


//--------------------------------------------------------------------------------------------------
/// Sets up vertex attribute pointer from client memory
/// 
/// \warning The caller must ensure that no buffer object is currently bound
//--------------------------------------------------------------------------------------------------
void AttribSetupStrategyDirectFloat::setupFromClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, const void* ptr) 
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsOpenGL2());
    glVertexAttribPointer(static_cast<GLuint>(vertexAttributeIndex), static_cast<GLint>(compCount), compTypeOpenGL, GL_FALSE, 0, ptr);
}



//==================================================================================================
///
/// \class cvf::AttribSetupStrategyInt
/// \ingroup Render
///
/// Strategy for specifying vertex attribute data as integers.
/// 
/// Supports array of generic vertex attributes and will be exposed to shader program as integer 
/// type. Can only be used on vertex attribute arrays where the component type is an integer type.
/// Uses glVertexAttribIPointer() which requires OpenGL 3.0
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Sets up vertex attribute pointer from bound buffer object
/// 
/// \warning The buffer object to use must already be bound
//--------------------------------------------------------------------------------------------------
void AttribSetupStrategyInt::setupFromBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, size_t strideInBytes, size_t bufferOffsetInBytes) 
{
    CVF_CALLSITE_OPENGL(oglContext);
    
    if (oglContext->capabilities()->supportsOpenGLVer(3))
    {
#ifndef CVF_OPENGL_ES
        glVertexAttribIPointer(static_cast<GLuint>(vertexAttributeIndex), static_cast<GLint>(compCount), compTypeOpenGL, static_cast<GLsizei>(strideInBytes), CVF_OGL_BUFFER_OFFSET(bufferOffsetInBytes));
#endif
    }
    else
    {
        CVF_LOG_RENDER_ERROR(oglContext, "glVertexAttribIPointer not supported");
    }
}


//--------------------------------------------------------------------------------------------------
/// Sets up vertex attribute pointer from client memory
/// 
/// \warning The caller must ensure that no buffer object is currently bound
//--------------------------------------------------------------------------------------------------
void AttribSetupStrategyInt::setupFromClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, const void* ptr) 
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (oglContext->capabilities()->supportsOpenGLVer(3))
    {
#ifndef CVF_OPENGL_ES
        glVertexAttribIPointer(static_cast<GLuint>(vertexAttributeIndex), static_cast<GLint>(compCount), compTypeOpenGL, 0, ptr);
#endif
    }
    else
    {
        CVF_LOG_RENDER_ERROR(oglContext, "glVertexAttribIPointer not supported");
    }
}



} // namespace cvf

