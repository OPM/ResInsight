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


#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>


// Define macros that must be used wherever we're going to do OpenGL calls
// The macro provides a QOpenGLExtraFunctions pointer named cvfGL that can be used to call OpenGL functions
// Use CVF_CALLSITE_OPENGL when you're going to call OpenGL functions.
#define CVF_CALLSITE_OPENGL(CURR_OGL_CTX_PTR) \
    CVF_UNUSED(CURR_OGL_CTX_PTR); \
    QOpenGLExtraFunctions* cvfGL = QOpenGLContext::currentContext()->extraFunctions()

// Include standard OpenGL headers for constants and basic types
// Note: Qt's QOpenGLFunctions headers already include the platform GL headers,
// but we include them explicitly for clarity. GLU is not used.
#if defined(WIN32) || defined(CVF_LINUX)

// Windows and Linux includes
#include <GL/gl.h>

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

#endif


#include "cvfOpenGLTypes.h"
#include "cvfString.h"
#include "cvfCodeLocation.h"

// Include context and context group for convenience
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

