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

#include "cvfObject.h"
#include "cvfOpenGLTypes.h"

namespace cvf {

class OpenGLContext;
class OglRcRenderbuffer;


//==================================================================================================
//
// Wrapper for OpenGL Renderbuffer Object
//
//==================================================================================================
class RenderbufferObject : public Object
{
public:
    enum InternalFormat
    {
        RGBA,
        DEPTH_COMPONENT16,
        DEPTH_COMPONENT24,
        DEPTH_COMPONENT32,
        DEPTH24_STENCIL8
    };

public:
    RenderbufferObject(InternalFormat format, uint width, uint height);
    ~RenderbufferObject();

    void    setSize(uint width, uint height);

    bool    create(OpenGLContext* oglContext);
    void    deleteRenderbuffer(OpenGLContext* oglContext);

    OglId   renderbufferOglId() const;
    uint    versionTick() const;

private:
    cvfGLenum       intenalFormatOpenGL() const;

private:
    InternalFormat          m_interalFormat;
    uint                    m_width;
    uint                    m_height;
    uint                    m_versionTick;              // Versioning to be able to detect changes
    ref<OglRcRenderbuffer>  m_oglRcBuffer;		        // 
};

}
