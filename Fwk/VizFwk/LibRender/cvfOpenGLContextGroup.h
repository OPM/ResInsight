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
#include "cvfCollection.h"
#include "cvfLogger.h"
#include "cvfOpenGLCapabilities.h"
#include "cvfOpenGLInfo.h"

struct GLEWContextStruct;
struct WGLEWContextStruct;

namespace cvf {

class OpenGLContext;
class OpenGLResourceManager;


//==================================================================================================
//
// 
//
//==================================================================================================
class OpenGLContextGroup : public Object
{
public:
    OpenGLContextGroup();
    virtual ~OpenGLContextGroup();

    bool                    isContextGroupInitialized() const;
    bool                    initializeContextGroup(OpenGLContext* currentContext);
    void                    contextAboutToBeShutdown(OpenGLContext* currentContextToShutdown);

    size_t                  contextCount() const;
    OpenGLContext*          context(size_t index);
    bool                    containsContext(const OpenGLContext* context) const;

    OpenGLResourceManager*  resourceManager();
    Logger*                 logger();

    OpenGLCapabilities*     capabilities();
    OpenGLInfo              info() const;
    
    GLEWContextStruct*      glewContextStruct();
    WGLEWContextStruct*     wglewContextStruct();

private:
    void                    uninitializeContextGroup();
    bool                    initializeGLEW(OpenGLContext* currentContext);
    bool                    initializeWGLEW(OpenGLContext* currentContext);
    void                    configureCapabilitiesFromGLEW(OpenGLContext* currentContext);
    void                    addContext(OpenGLContext* contextToAdd);

private:
    bool                        m_isInitialized;
    Collection<OpenGLContext>   m_contexts;             // The OpenGL contexts that belong to this group
    ref<OpenGLResourceManager>  m_resourceManager;      // Resource manager that is shared between all contexts in this group
    ref<Logger>                 m_logger;
    ref<OpenGLCapabilities>     m_capabilities;         // Capabilities of the contexts in this group context
    OpenGLInfo                  m_info;
    GLEWContextStruct*          m_glewContextStruct;    // Pointer to the GLEW context struct 
    WGLEWContextStruct*         m_wglewContextStruct;   // Pointer to the GLEW context struct 

    friend class OpenGLContext;
};


}

