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
#include "cvfCollection.h"
#include "cvfLogger.h"

struct GLEWContextStruct;

namespace cvf {

class OpenGLContext;
class OpenGLResourceManager;
class OpenGLCapabilities;


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

    size_t                  contextCount() const;
    bool                    containsContext(const OpenGLContext* context) const;

    OpenGLResourceManager*  resourceManager();
    Logger*                 logger();

    OpenGLCapabilities*     capabilities();
    GLEWContextStruct*      glewContextStruct();

private:
    bool                    initializeContextGroup(OpenGLContext* currentContext);
    void                    uninitializeContextGroup();
    void                    contextAboutToBeShutdown(OpenGLContext* contextToShutdown);
    bool                    initializeGLEW(OpenGLContext* currentContext);
    void                    configureCapablititesFromGLEW(OpenGLContext* currentContext);
    void                    addContext(OpenGLContext* contextToAdd);

private:
    bool                        m_isInitialized;
    Collection<OpenGLContext>   m_contexts;             // The OpenGL contexts that belong to this group
    ref<OpenGLResourceManager>  m_resourceManager;      // Resource manager that is shared between all contexts in this group
    ref<Logger>                 m_logger;
    ref<OpenGLCapabilities>     m_capabilities;         // Capabilities of the contexts in this group context
    GLEWContextStruct*          m_glewContextStruct;    // Pointer to the GLEW context struct 

    friend class OpenGLContext;
};


}



