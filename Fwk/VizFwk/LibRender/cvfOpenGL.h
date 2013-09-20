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


// Currently we utilize GLEW everywhere except for OpenGLES
#ifndef CVF_OPENGL_ES
#define CVF_USE_GLEW
#endif


#ifdef CVF_USE_GLEW

// Define GLEW_MX for thread safe multi OpenGL context use in GLEW.
// On WIN32 the GLEW_MX define causes GLEW to store both the function pointers and the GLEW 
// vars (such as __GLEW_VERSION_1_1 and __GLEW_ARB_draw_buffers) inside GLEWContextStruct.
// On other platforms GLEW stores the function pointers in global variables while the GLEW vars
// are stored inside the GLEWContextStruct.
// We also need to define the glewGetContext() macro - see definition of the CVF_CALLSITE_OPENGL macro above
#define GLEW_MX
#define glewGetContext() (cvfLocGLEWCtxPtr)

// Need to define GLEW_STATIC since we're not using GLEW as a DLL
#define GLEW_STATIC
#include "glew/GL/glew.h"

#endif // CVF_USE_GLEW


// Define macros that must be used wherever we're going to do OpenGL calls (via GLEW)
// Use CVF_CALLSITE_OPENGL when you're going to call OpenGL functions. If you're going
// to query GLEW variables (and possibly do OpenGL calls), use CVF_CALLSITE_GLEW instead
// The reason for having two macros is that most of the time we don't need the pointer
// to the GLEW structure on non-WIN32 platforms since the actual function pointers
// are stored in global variables and only the GLEW vars are stored in the struct.
#define CVF_CALLSITE_GLEW(CURR_OGL_CTX_PTR) \
    GLEWContextStruct* cvfLocGLEWCtxPtr = CURR_OGL_CTX_PTR->group()->glewContextStruct()

#ifdef WIN32
#define CVF_CALLSITE_OPENGL(CURR_OGL_CTX_PTR)  CVF_CALLSITE_GLEW(CURR_OGL_CTX_PTR)
#else
#define CVF_CALLSITE_OPENGL(CURR_OGL_CTX_PTR)  CVF_UNUSED(CURR_OGL_CTX_PTR)
#endif    



#if defined(WIN32) || defined(CVF_LINUX)

// Windows and Linux includes
#include <GL/gl.h>
#include <GL/glu.h>

#elif defined(CVF_ANDROID)

// Android includes
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#elif defined(CVF_IOS)

// iOS includes
#include "OpenGLES/ES2/gl.h"
#include "OpenGLES/ES2/glext.h"

#elif defined(CVF_OSX)

// Mac OSX includes
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"

#endif 


#include "cvfOpenGLTypes.h"
#include "cvfString.h"
#include "cvfCodeLocation.h"

// As long as we're using GLEW we will almost always need the context and context group when doing OpenGL calls
#include "cvfOpenGLContext.h"
#include "cvfOpenGLContextGroup.h"



namespace cvf {

//==================================================================================================
//
// Static class providing OpenGL wrappers and helpers 
//
//==================================================================================================
class OpenGL 
{
public:
    static bool     hasOpenGLError(OpenGLContext* oglContext);
    static void     clearOpenGLError(OpenGLContext* oglContext);

	static String   mapOpenGLErrorToString(cvfGLenum errorCode);
    static bool     testAndReportOpenGLError(OpenGLContext* oglContext, const char* operation, const CodeLocation& codeLocation);

    static void     cvf_check_ogl(OpenGLContext* oglContext, const CodeLocation& codeLocation);
    
    static void	    enableCheckOgl(bool enable);
    static bool	    isCheckOglEnabled();

private:
    static bool     m_enableCheckOgl;
};

}


// Define used for specifying buffer offset for glVertexAttribPointer
#define CVF_OGL_BUFFER_OFFSET(BYTE_OFFSET) ((char*)NULL + (BYTE_OFFSET))

// Define used to log error messages with file and line
#define CVF_CHECK_OGL(OGL_CTX_PTR)  cvf::OpenGL::cvf_check_ogl(OGL_CTX_PTR, CVF_CODE_LOCATION)

#define CVF_CLEAR_OGL_ERROR(OGL_CTX_PTR) cvf::OpenGL::clearOpenGLError(OGL_CTX_PTR)
#define CVF_TEST_AND_REPORT_OPENGL_ERROR(OGL_CTX_PTR, OPERATION)  cvf::OpenGL::testAndReportOpenGLError(OGL_CTX_PTR, OPERATION, CVF_CODE_LOCATION)

