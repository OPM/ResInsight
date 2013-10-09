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
#include "cvfShaderProgram.h"
#include "cvfLogger.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOglRc.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Shader
/// \ingroup Render
///
/// Encapsulates a GLSL shader.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
Shader::Shader(ShaderType shaderType, const String& shaderName)
:   m_shaderType(shaderType),
    m_shaderName(shaderName),
    m_lastCompileSucceeded(false),
    m_compiledVersionTick(-1)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Shader::Shader(ShaderType shaderType, const String& shaderName, const String& source)
:   m_shaderType(shaderType),
    m_shaderName(shaderName),
    m_source(source),
    m_lastCompileSucceeded(false),
    m_compiledVersionTick(-1)
{
}


//--------------------------------------------------------------------------------------------------
/// OpenGL resources must already be deleted. \sa deleteShader()
//--------------------------------------------------------------------------------------------------
Shader::~Shader()
{
    // Just release our reference
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcShader.p()));
    m_oglRcShader = NULL;
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
Shader::ShaderType Shader::shaderType() const
{
    return m_shaderType;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Shader::shaderName() const
{
    return m_shaderName;
}


//--------------------------------------------------------------------------------------------------
/// Compiles the shader if needed
/// 
/// \return  Returns true if the shader is/was compiled. Otherwise false.
//--------------------------------------------------------------------------------------------------
bool Shader::compile(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_ASSERT(ShaderProgram::supportedOpenGL(oglContext));

    if (OglRc::safeOglId(m_oglRcShader.p()) != 0 && m_lastCompileSucceeded)
    {
#ifdef _DEBUG
        uint myOglId = OglRc::safeOglId(m_oglRcShader.p());
        GLint status;
        glGetShaderiv(myOglId, GL_COMPILE_STATUS, &status);
        CVF_ASSERT(status == GL_TRUE);
#endif

        return true;
    }

    // Start by flagging compile as failure
    m_lastCompileSucceeded = false;

    if (OglRc::safeOglId(m_oglRcShader.p()) == 0)
    {
        cvfGLenum glShaderType = GL_VERTEX_SHADER;
        switch (m_shaderType)
        {
            case VERTEX_SHADER:     glShaderType = GL_VERTEX_SHADER;        break;
            case FRAGMENT_SHADER:   glShaderType = GL_FRAGMENT_SHADER;      break;
#ifndef CVF_OPENGL_ES
            case GEOMETRY_SHADER:   glShaderType = GL_GEOMETRY_SHADER;      break;
#endif
            default:                CVF_FAIL_MSG("Unhandled shader type");  break;
        }

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcShader.p()));
        m_oglRcShader = oglContext->resourceManager()->createOglRcShader(oglContext, glShaderType);
        if (m_oglRcShader.isNull())
        {
            return false;
        }
    }

    uint myOglId = m_oglRcShader->oglId();
    CVF_ASSERT(myOglId != 0);

    if (m_source.isEmpty())
    {
        String errStr = String("Error compiling shader: '%1'\n").arg(m_shaderName);
        errStr += "Shader source is empty";
        CVF_LOG_RENDER_ERROR(oglContext, errStr);
        return false;
    }

    CharArray charArr = m_source.toAscii();

    const char* stringArray[1];
    stringArray[0] = charArr.ptr();

    glShaderSource(myOglId, 1, stringArray, NULL);
    CVF_CHECK_OGL(oglContext);

    glCompileShader(myOglId);

    GLint iCompileStatus = 0;
    glGetShaderiv(myOglId, GL_COMPILE_STATUS, &iCompileStatus);
    if (iCompileStatus != GL_TRUE)
    {
        String errStr = String("Error compiling shader: '%1'\n").arg(m_shaderName);
        errStr += "GLSL details:\n";
        errStr += shaderInfoLog(oglContext);
        CVF_LOG_RENDER_ERROR(oglContext, errStr);
        return false;
    }

    m_compiledVersionTick++;
    m_lastCompileSucceeded = true;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int Shader::compiledVersionTick() const
{
    if (m_lastCompileSucceeded && OglRc::safeOglId(m_oglRcShader.p()) != 0)
    {
        return m_compiledVersionTick;
    }
    else
    {
        return -1;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLuint Shader::shaderOglId() const
{
    return OglRc::safeOglId(m_oglRcShader.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Shader::deleteShader(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    if (m_oglRcShader.notNull())
    {
        m_oglRcShader->deleteResource(oglContext);
        CVF_CHECK_OGL(oglContext);

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcShader.p()));
        m_oglRcShader = NULL;

        m_lastCompileSucceeded = false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the information log for this shader
/// 
/// See: http://www.opengl.org/sdk/docs/man/xhtml/glGetShaderInfoLog.xml 
//--------------------------------------------------------------------------------------------------
String Shader::shaderInfoLog(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    uint myOglId = OglRc::safeOglId(m_oglRcShader.p());
    if (myOglId == 0)
    {
        return "Shader object not created.";
    }

    if (!glIsShader(myOglId))
    {
        return "Shader object identifier does not correspond to a shader object.";
    }
        
    GLint reqBufferSize = 0;    
    glGetShaderiv(myOglId, GL_INFO_LOG_LENGTH, &reqBufferSize);

    if (reqBufferSize > 0)
    {
        // TODO
        // This is a good place for using CharArray
        std::vector<GLchar> charBuffer;
        charBuffer.resize(static_cast<size_t>(reqBufferSize + 1));
        glGetShaderInfoLog(myOglId, reqBufferSize, NULL, &charBuffer[0]);

        String logString(&charBuffer[0]);
        return logString;
    }
    else
    {
        return "";
    }
}


} // namespace cvf

