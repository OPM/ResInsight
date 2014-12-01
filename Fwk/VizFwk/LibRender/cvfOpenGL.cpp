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
#include "cvfOpenGL.h"
#include "cvfSystem.h"

// Get rid of warnings from glew
#ifdef WIN32
#pragma warning (disable: 4706 4365 4191 4668)
#endif

CVF_GCC_DIAGNOSTIC_IGNORE("-Wconversion")

#ifndef CVF_OPENGL_ES

#undef glewGetContext

extern "C"
{
#include "glew/glew.c"
}

#endif  // CVF_OPENGL_ES

namespace cvf {



//==================================================================================================
///
/// \class cvf::OpenGL
/// \ingroup Render
///
/// Static class providing OpenGL wrappers and helpers 
/// 
//==================================================================================================

bool OpenGL::m_enableCheckOgl = true;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGL::hasOpenGLError(OpenGLContext* oglContext)
{
    // glGetError will end up in an endless loop if no context is current
    CVF_ASSERT(oglContext);
    CVF_ASSERT(oglContext->isCurrent());
    CVF_UNUSED(oglContext);

	cvfGLenum err = glGetError();
	if (err == GL_NO_ERROR)
	{
		return false;
	}
	else
	{
		// Empty all error flags
		while (err != GL_NO_ERROR)
		{
			err = glGetError();
		}

		return true;
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGL::clearOpenGLError(OpenGLContext* oglContext)
{
    // glGetError will end up in an endless loop if no context is current
    CVF_ASSERT(oglContext);
    CVF_ASSERT(oglContext->isCurrent());
    CVF_UNUSED(oglContext);

    cvfGLenum err = glGetError();
    
    // Empty all error flags
    while (err != GL_NO_ERROR)
    {
        err = glGetError();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String OpenGL::mapOpenGLErrorToString(cvfGLenum errorCode)
{
    String errCodeStr;

	switch (errorCode)
	{
		case GL_INVALID_ENUM:       errCodeStr = "GL_INVALID_ENUM";       break;
		case GL_INVALID_VALUE:      errCodeStr = "GL_INVALID_VALUE";      break;
		case GL_INVALID_OPERATION:  errCodeStr = "GL_INVALID_OPERATION";  break;
		case GL_OUT_OF_MEMORY:      errCodeStr = "GL_OUT_OF_MEMORY";      break;
#ifndef CVF_OPENGL_ES
		case GL_STACK_OVERFLOW:     errCodeStr = "GL_STACK_OVERFLOW";     break;
		case GL_STACK_UNDERFLOW:    errCodeStr = "GL_STACK_UNDERFLOW";    break;
#endif
        case GL_NO_ERROR:           errCodeStr = "GL_NO_ERROR";           break;

		default:
		{
			char szBuf[101];
			System::sprintf(szBuf, 100, "0x%04x", errorCode);
			errCodeStr = szBuf;
		}
	}

    return errCodeStr;
}


//--------------------------------------------------------------------------------------------------
/// Returns false if no error.
//--------------------------------------------------------------------------------------------------
bool OpenGL::testAndReportOpenGLError(OpenGLContext* oglContext, const char* operation, const CodeLocation& codeLocation)
{
    // glGetError will end up in an endless loop if no context is current
    CVF_ASSERT(oglContext);
    CVF_ASSERT(oglContext->isCurrent());

    cvfGLenum err = glGetError();
    if (err == GL_NO_ERROR)
    {
        return false;
    }

    Logger* logger = oglContext->group()->logger();

	while (err != GL_NO_ERROR)
	{
        if (logger)
        {
            String errCodeStr = mapOpenGLErrorToString(err);
            String msg = String("Operation: ") + operation;
            msg += "OGL(" + errCodeStr + "): ";
            logger->error(msg, codeLocation);
        }

		err = glGetError();
	}

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGL::cvf_check_ogl(OpenGLContext* oglContext, const CodeLocation& codeLocation)
{
    if (OpenGL::m_enableCheckOgl)
    {
        cvfGLenum err = glGetError();
        while (err != GL_NO_ERROR)
        {
            // glGetError will end up in an endless loop if no context is current
            CVF_ASSERT(oglContext);
            CVF_ASSERT(oglContext->isCurrent());

            Logger* logger = oglContext->group()->logger();
#if defined(CVF_OSX)
            if (logger && (err != GL_INVALID_FRAMEBUFFER_OPERATION))
#else
            if (logger)
#endif /* defined(CVF_OSX) */
            {
                String errCodeStr = mapOpenGLErrorToString(err);
                String msg = "OGL(" + errCodeStr + "): ";
                logger->error(msg, codeLocation);
            }

            err = glGetError();
        }
    }
}
    
    
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGL::enableCheckOgl(bool enable)
{
    m_enableCheckOgl = enable;
}

    
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGL::isCheckOglEnabled()
{
    return m_enableCheckOgl;
}


} // namespace cvf

