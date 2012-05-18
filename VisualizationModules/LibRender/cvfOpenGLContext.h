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
#include "cvfOpenGLContextGroup.h"

namespace cvf {

class OpenGLResourceManager;
class OpenGLCapabilities;


//==================================================================================================
//
// Encapsulates an OpenGL rendering context
//
//==================================================================================================
class OpenGLContext : public Object
{
public:
    OpenGLContext(OpenGLContextGroup* contextGroup);
    virtual ~OpenGLContext();

    bool                        isContextValid() const;
    virtual bool                initializeContext();
    virtual void                shutdownContext();

    virtual void                makeCurrent() = 0;
    virtual bool                isCurrent() const = 0;

    OpenGLContextGroup*         group();
    const OpenGLContextGroup*   group() const;
    OpenGLResourceManager*      resourceManager();
    const OpenGLCapabilities*   capabilities();

private:
    OpenGLContextGroup*         m_contextGroup;         // Raw pointer (to avoid circular reference) to the context group that this context belongs to. 
    bool                        m_isValid;              // Will be set to true after successful initialization

    friend class OpenGLContextGroup;
};

#define CVF_LOG_RENDER_ERROR(OGL_CTX_PTR, THE_MESSAGE) OGL_CTX_PTR->group()->logger()->error((THE_MESSAGE), __FILE__, __LINE__ )

#define CVF_LOG_RENDER_DEBUG(OGL_CTX_PTR, THE_MESSAGE) OGL_CTX_PTR->group()->logger()->debug((THE_MESSAGE), __FILE__, __LINE__ )
#define CVF_SHOULD_LOG_RENDER_DEBUG(OGL_CTX_PTR)       OGL_CTX_PTR->group()->logger()->isDebugEnabled()


}

