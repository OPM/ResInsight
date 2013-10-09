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
// 
//
//==================================================================================================
class OglRc : public Object
{
public:
    ~OglRc();

    OglId           oglId() const;
    virtual void    deleteResource(OpenGLContext* oglContext) = 0;

    static OglId    safeOglId(const OglRc* oglRcObj);
    static bool     isSafeToRelease(const OglRc* oglRcObj);

protected:
    OglRc();

protected:
    OglId m_openGLObjId;    // The OpenGL object identifier (object name in OGL lingo). 0 means that no handle exists
};


//==================================================================================================
//
// 
//
//==================================================================================================
class OglRcShader : public OglRc
{
public:
    static ref<OglRcShader> create(OpenGLContext* oglContext, cvfGLenum shaderType);
    virtual void            deleteResource(OpenGLContext* oglContext);
};


//==================================================================================================
//
// 
//
//==================================================================================================
class OglRcProgram : public OglRc
{
public:
    static ref<OglRcProgram> create(OpenGLContext* oglContext);
    virtual void             deleteResource(OpenGLContext* oglContext);
};



//==================================================================================================
//
// 
//
//==================================================================================================
class OglRcTexture : public OglRc
{
public:
    static ref<OglRcTexture> create(OpenGLContext* oglContext);
    virtual void             deleteResource(OpenGLContext* oglContext);
};


//==================================================================================================
//
// 
//
//==================================================================================================
class OglRcRenderbuffer : public OglRc
{
public:
    static ref<OglRcRenderbuffer> create(OpenGLContext* oglContext);
    virtual void                  deleteResource(OpenGLContext* oglContext);
};


//==================================================================================================
//
// 
//
//==================================================================================================
class OglRcFramebuffer : public OglRc
{
public:
    static ref<OglRcFramebuffer>  create(OpenGLContext* oglContext);
    virtual void                  deleteResource(OpenGLContext* oglContext);
};


}


