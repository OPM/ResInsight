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
#include "cvfOpenGLContext.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfTrace.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::OpenGLContext
/// \ingroup Render
///
/// Encapsulates an OpenGL rendering context
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
///
/// The context will be added unconditionally to the \a contextGroup group
//--------------------------------------------------------------------------------------------------
OpenGLContext::OpenGLContext(OpenGLContextGroup* contextGroup)
:   m_contextGroup(contextGroup),
    m_isValid(false)
{
    CVF_ASSERT(m_contextGroup);
    m_contextGroup->addContext(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLContext::~OpenGLContext()
{
    //Trace::show("OpenGLContext destructor");

    m_isValid = false;

    // Context group is holding references to contexts, so by the time we get to this
    // destructor the link to the context group must already be broken
    CVF_ASSERT(m_contextGroup == NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLContext::isContextValid() const
{
    // Also check on context group, since it may have been set to NULL after valid flag was set
    if (m_contextGroup && m_isValid)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLContext::initializeContext()
{
    makeCurrent();

    if (!m_contextGroup)
    {
        return false;
    }

    if (!m_contextGroup->isContextGroupInitialized())
    {
        if (!m_contextGroup->initializeContextGroup(this))
        {
            return false;
        }
    }

    m_isValid = true;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Prepare the context for deletion
///
/// Prepares the context for deletion by removing the context from its context group. If this is the
/// last context in the context group, this function will also try and delete the context group's
/// OpenGL resources.
/// 
/// \warning  After calling this function the context is no longer usable and should be deleted.
/// \warning  May try and make the context current in order to free resources if this context
///           is the last context in the context group.
//--------------------------------------------------------------------------------------------------
void OpenGLContext::shutdownContext()
{
    if (m_contextGroup)
    {
        CVF_ASSERT(m_contextGroup->containsContext(this));

        // If our ref count is down to 1, there is probably something strange going on.
        // Since one reference to us is being held by the context group, this means that the
        // caller ISN'T holding a reference which may give him a nast surprise when this function returns!
        CVF_ASSERT(refCount() > 1);

        // This call will remove the context from the group AND clean up resources in the
        // group if we are the last context in the group
        m_contextGroup->contextAboutToBeShutdown(this);
    }

    CVF_ASSERT(m_contextGroup == NULL);

    m_isValid = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLContextGroup* OpenGLContext::group()
{
    return m_contextGroup;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const OpenGLContextGroup* OpenGLContext::group() const
{
    return m_contextGroup;
}


//--------------------------------------------------------------------------------------------------
/// Get the resource manager to use with this context
/// 
/// This is a convenience function for getting the resource manager from the owner context group.
//--------------------------------------------------------------------------------------------------
OpenGLResourceManager* OpenGLContext::resourceManager()
{
    CVF_ASSERT(m_contextGroup);
    return m_contextGroup->resourceManager();
}



//--------------------------------------------------------------------------------------------------
/// Get the capabilities for this context
/// 
/// This is a convenience function for getting the capabilities from the owner context group.
//--------------------------------------------------------------------------------------------------
const OpenGLCapabilities* OpenGLContext::capabilities()
{
    CVF_ASSERT(m_contextGroup);
    return m_contextGroup->capabilities();
}



} // namespace cvf

