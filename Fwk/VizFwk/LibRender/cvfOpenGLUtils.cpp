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
#include "cvfOpenGLUtils.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::OpenGLUtils
/// \ingroup Render
///
/// Static class providing OpenGL helpers 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Store the current OpenGL context settings by using OpenGL's built in push methods.
/// 
/// Note: This call MUST be matched with a corresponding popOpenGLState() call.
//--------------------------------------------------------------------------------------------------
void OpenGLUtils::pushOpenGLState(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    const cvf::OpenGLCapabilities* oglCaps = oglContext->capabilities();

    // Only relevant for fixed function
    if (!oglCaps->supportsFixedFunction())
    {
        return;
    }

    CVF_CHECK_OGL(oglContext);

    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    CVF_CHECK_OGL(oglContext);

    // For now disable pushing of the vertex array related attributes as it gives a mystical
    // crash on Redhat5 under VMWare. Not a big issue, but maybe we can do without this push?
    //glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    CVF_CHECK_OGL(oglContext);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    CVF_CHECK_OGL(oglContext);

    //  Note: Only preserves matrix stack for texture unit 0
    if (oglCaps->supportsOpenGL2())
    {
        glActiveTexture(GL_TEXTURE0);
    }
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    CVF_CHECK_OGL(oglContext);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    CVF_CHECK_OGL(oglContext);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// Set back the stored OpenGL context settings by using OpenGL's built in pop methods.
/// 
/// Note: This call MUST be matched with a corresponding pushOpenGLState() call.
//--------------------------------------------------------------------------------------------------
void OpenGLUtils::popOpenGLState(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    const cvf::OpenGLCapabilities* oglCaps = oglContext->capabilities();

    // Only relevant for fixed function
    if (!oglCaps->supportsFixedFunction())
    {
        return;
    }

    CVF_CHECK_OGL(oglContext);

    //  Note: Only preserves matrix stack for texture unit 0
    if (oglCaps->supportsOpenGL2())
    {
        glActiveTexture(GL_TEXTURE0);
    }
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    CVF_CHECK_OGL(oglContext);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    CVF_CHECK_OGL(oglContext);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    CVF_CHECK_OGL(oglContext);

    glPopAttrib();
    CVF_CHECK_OGL(oglContext);

    // Currently not pushing vertex attribs, so comment out the pop
    //glPopClientAttrib();

    glPopClientAttrib();
    CVF_CHECK_OGL(oglContext);
}


} // namespace cvf

