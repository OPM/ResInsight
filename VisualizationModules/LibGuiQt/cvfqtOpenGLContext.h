//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfOpenGLContext.h"

class QGLContext;

namespace cvfqt {


//==================================================================================================
//
// Derived OpenGLContext that adapts a Qt QGLContext
//
//==================================================================================================
class OpenGLContext : public cvf::OpenGLContext
{
public:
    OpenGLContext(cvf::OpenGLContextGroup* contextGroup, QGLContext* backingQGLContext);
    virtual ~OpenGLContext();

    virtual bool    initializeContext();

    virtual void    makeCurrent();
    virtual bool    isCurrent() const;

    static void		saveOpenGLState(cvf::OpenGLContext* oglContext);
    static void     restoreOpenGLState(cvf::OpenGLContext* oglContext);

private:
    QGLContext*     m_qtGLContext;
    bool            m_isCoreOpenGLProfile;  // This is a Core OpenGL profile. Implies OpenGL version of 3.2 or more
    int             m_majorVersion;         // OpenGL version as reported by Qt
    int             m_minorVersion;
};

}
