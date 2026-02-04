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
#include "cvfOpenGLContextGroup.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOpenGLCapabilities.h"
#include "cvfLogManager.h"
#include "cvfTrace.h"

#include <QOpenGLContext>
#include <QSurfaceFormat>


namespace cvf {


//==================================================================================================
///
/// \class cvf::OpenGLContextGroup
/// \ingroup Render
///
/// A context group associates OpenGLContext instances that share OpenGL resources such as
/// shader objects, textures and buffer objects. Contexts added to the group must be compatible from
/// OpenGL's perspective - that is they must use identical (pixel) formats.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OpenGLContextGroup::OpenGLContextGroup()
:   m_isInitialized(false)
{
    //Trace::show("OpenGLContextGroup constructor");

    m_resourceManager = new OpenGLResourceManager;
    m_logger = CVF_GET_LOGGER("cee.cvf.OpenGL");
    m_capabilities = new OpenGLCapabilities;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OpenGLContextGroup::~OpenGLContextGroup()
{
    //Trace::show("OpenGLContextGroup destructor");

    // Our resource manager must already be clean
    // There is no way of safely deleting the resources after all the contexts have been removed
    CVF_ASSERT(m_resourceManager.notNull());
    CVF_ASSERT(!m_resourceManager->hasAnyOpenGLResources());

    // We're about to die, so make sure we disentangle from the contexts
    // This is important since all the contexts have back raw pointers to the group
    size_t numContexts = m_contexts.size();
    size_t i;
    for (i = 0; i < numContexts; i++)
    {
        OpenGLContext* ctx = m_contexts.at(i);
        ctx->m_contextGroup = NULL;
    }

    m_contexts.clear();

    if (m_isInitialized)
    {
        uninitializeContextGroup();
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool OpenGLContextGroup::isContextGroupInitialized() const
{
    return m_isInitialized;
}


//--------------------------------------------------------------------------------------------------
/// Do initialization of this context group
///
/// It is safe to call this function multiple times, successive calls will do nothing if the
/// context group is already initialized.
///
/// \warning The context passed inn \a currentContext must be the current context and it must
///          be a member of this context group.
//--------------------------------------------------------------------------------------------------
bool OpenGLContextGroup::initializeContextGroup(OpenGLContext* currentContext)
{
    //Trace::show("OpenGLContextGroup::initializeContextGroup()");

    CVF_ASSERT(currentContext);
    CVF_ASSERT(currentContext->isCurrent());
    CVF_ASSERT(containsContext(currentContext));

    if (!m_isInitialized)
    {
        if (!initializeOpenGLFunctions(currentContext))
        {
            CVF_LOG_ERROR(m_logger.p(), "Failed to initialize OpenGL functions in context group");
            return false;
        }

        configureCapabilities(currentContext);

        CVF_LOG_DEBUG(m_logger.p(), "OpenGL initialized in context group");
        CVF_LOG_DEBUG(m_logger.p(), "  version:  " + m_info.version());
        CVF_LOG_DEBUG(m_logger.p(), "  vendor:   " + m_info.vendor());
        CVF_LOG_DEBUG(m_logger.p(), "  renderer: " + m_info.renderer());

        m_isInitialized = true;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OpenGLContextGroup::uninitializeContextGroup()
{
    //Trace::show("OpenGLContextGroup::uninitializeContextGroup()");

    CVF_ASSERT(m_contexts.empty());
    CVF_ASSERT(!m_resourceManager->hasAnyOpenGLResources());

    // Just replace capabilities with a new object
    m_capabilities = new OpenGLCapabilities;

    m_isInitialized = false;
}


//--------------------------------------------------------------------------------------------------
/// Prepare a context for deletion
///
/// This function will remove the context \a currentContextToShutdown from the context group.
/// If \a currentContextToShutdown is the last context in the group, all resources in the group's
/// resource manager will be deleted and the context group will be reset to uninitialized.
///
/// \warning  The passed context must be the current OpenGL context!
/// \warning  After calling this function the context is no longer usable and should be deleted.
//--------------------------------------------------------------------------------------------------
void OpenGLContextGroup::contextAboutToBeShutdown(OpenGLContext* currentContextToShutdown)
{
    //Trace::show("OpenGLContextGroup::contextAboutToBeShutdown()");

    CVF_ASSERT(currentContextToShutdown);
    CVF_ASSERT(containsContext(currentContextToShutdown));

    // If this is the last context in the group, we'll delete all the OpenGL resources in the s resource manager before we go
    bool shouldUninitializeGroup = false;
    if (contextCount() == 1)
    {
        CVF_ASSERT(m_resourceManager.notNull());
        if (m_resourceManager->hasAnyOpenGLResources() && currentContextToShutdown->isContextValid())
        {
            m_resourceManager->deleteAllOpenGLResources(currentContextToShutdown);
        }

        shouldUninitializeGroup = true;
    }

    // If the ref count on the context is down to 1, there is probably something strange going on.
    // Since one reference to the context is being held by this context group, this means that the
    // caller isn't holding a reference to the context. In this case the context object will evaporate
    // during the call below, and the caller will most likely get a nasty surprise
    CVF_ASSERT(currentContextToShutdown->refCount() > 1);

    // Make sure we set the back pointer before removing it from our collection
    // since the removal from the list may actually trigger destruction of the context (see comment above)
    currentContextToShutdown->m_contextGroup = NULL;
    m_contexts.erase(currentContextToShutdown);

    if (shouldUninitializeGroup)
    {
        // Bring context group back to virgin state
        uninitializeContextGroup();
    }
}


//--------------------------------------------------------------------------------------------------
/// Initializes OpenGL functions for this context group using Qt
///
/// \warning The context passed in \a currentContext must be current!
//--------------------------------------------------------------------------------------------------
bool OpenGLContextGroup::initializeOpenGLFunctions(OpenGLContext* currentContext)
{
    //Trace::show("OpenGLContextGroup::initializeOpenGLFunctions()");

    CVF_ASSERT(currentContext);

    QOpenGLContext* qtCtx = QOpenGLContext::currentContext();
    if (!qtCtx || !qtCtx->isValid())
    {
        CVF_LOG_ERROR(m_logger, "Error initializing OpenGL functions, no valid Qt OpenGL context is current");
        return false;
    }

    // Initialize extra functions (OpenGL 3.0+)
    QOpenGLExtraFunctions* extraFuncs = qtCtx->extraFunctions();
    if (!extraFuncs)
    {
        CVF_LOG_ERROR(m_logger, "Error initializing OpenGL extra functions");
        return false;
    }
    extraFuncs->initializeOpenGLFunctions();

    CVF_CHECK_OGL(currentContext);

    // Query OpenGL info strings
    const char* versionStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* vendorStr = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* rendererStr = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    String sVersion = versionStr ? versionStr : "";
    String sVendor = vendorStr ? vendorStr : "";
    String sRenderer = rendererStr ? rendererStr : "";

    if (sVersion.find(".") == String::npos)
    {
        CVF_LOG_ERROR(m_logger, "Error initializing OpenGL functions, probe for OpenGL version failed. No valid OpenGL context is current");
        return false;
    }

    m_info.setOpenGLStrings(sVersion, sVendor, sRenderer);

    CVF_CHECK_OGL(currentContext);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Configures the capabilities of this context group by querying Qt
///
/// \warning The passed context must be current!
//--------------------------------------------------------------------------------------------------
void OpenGLContextGroup::configureCapabilities(OpenGLContext* currentContext)
{
    CVF_UNUSED(currentContext);

    QOpenGLContext* qtCtx = QOpenGLContext::currentContext();
    if (!qtCtx)
    {
        return;
    }

    QSurfaceFormat fmt = qtCtx->format();
    int majorVer = fmt.majorVersion();

    // Simplified config for now, we only differentiate between 1, 2, and 3
    uint capMajorVer = 1;
    if (majorVer >= 2) capMajorVer = 2;
    if (majorVer >= 3) capMajorVer = 3;
    m_capabilities->configureOpenGLSupport(capMajorVer);
    m_capabilities->setSupportsFixedFunction(true);

    // Check for framebuffer object support
    if (majorVer >= 3 || qtCtx->hasExtension("GL_ARB_framebuffer_object"))
    {
        m_capabilities->addCapablity(OpenGLCapabilities::FRAMEBUFFER_OBJECT);
        m_capabilities->addCapablity(OpenGLCapabilities::GENERATE_MIPMAP_FUNC);
    }

    // Check for texture float support
    if (majorVer >= 3 || qtCtx->hasExtension("GL_ARB_texture_float"))
    {
        m_capabilities->addCapablity(OpenGLCapabilities::TEXTURE_FLOAT);
    }

    // Check for texture RG support
    if (majorVer >= 3 || qtCtx->hasExtension("GL_ARB_texture_rg"))
    {
        m_capabilities->addCapablity(OpenGLCapabilities::TEXTURE_RG);
    }

    // Check for texture rectangle support
    if (majorVer >= 3 || qtCtx->hasExtension("GL_ARB_texture_rectangle"))
    {
        m_capabilities->addCapablity(OpenGLCapabilities::TEXTURE_RECTANGLE);
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t OpenGLContextGroup::contextCount() const
{
    return m_contexts.size();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::OpenGLContext* OpenGLContextGroup::context(size_t index)
{
    return m_contexts.at(index);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool OpenGLContextGroup::containsContext(const OpenGLContext* context) const
{
    if (m_contexts.contains(context))
    {
        CVF_ASSERT(context->group() == this);
        return true;
    }
    else
    {
        CVF_ASSERT(context->group() != this);
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Add an OpenGLContext to this group
///
/// \warning It is the caller's responsibility to ensure that the context being added is actually
///          capable of sharing OpenGL resources (display lists etc) with the contexts that are
///          already in the group. This function will not do any verifications to assert whether this is true.
//--------------------------------------------------------------------------------------------------
void OpenGLContextGroup::addContext(OpenGLContext* contextToAdd)
{
    CVF_ASSERT(contextToAdd);

    // Illegal to add a context that is already in our group
    CVF_ASSERT(!m_contexts.contains(contextToAdd));

    // Guard against adding a context that is already member of a another group
    CVF_ASSERT(contextToAdd->group() == NULL || contextToAdd->group() == this);

    // Add it to our group and set ourselves as the owning group using our friend powers
    m_contexts.push_back(contextToAdd);
    contextToAdd->m_contextGroup = this;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OpenGLResourceManager* OpenGLContextGroup::resourceManager()
{
    CVF_ASSERT(m_resourceManager.notNull());
    return m_resourceManager.p();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Logger* OpenGLContextGroup::logger()
{
    return m_logger.p();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OpenGLCapabilities* OpenGLContextGroup::capabilities()
{
    CVF_ASSERT(m_capabilities.notNull());
    return m_capabilities.p();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OpenGLInfo OpenGLContextGroup::info() const
{
    return m_info;
}


} // namespace cvf
