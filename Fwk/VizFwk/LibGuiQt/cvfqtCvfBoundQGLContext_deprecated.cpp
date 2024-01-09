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
#include "cvfOpenGLCapabilities.h"
#include "cvfqtCvfBoundQGLContext_deprecated.h"

namespace cvfqt {



//==================================================================================================
///
/// \class cvfqt::OpenGLContext_QGLContextAdapter_deprecated
/// \ingroup GuiQt
///
/// Derived OpenGLContext that adapts a Qt QGLContext
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLContext_QGLContextAdapter_deprecated::OpenGLContext_QGLContextAdapter_deprecated(cvf::OpenGLContextGroup* contextGroup, QGLContext* backingQGLContext)
:   cvf::OpenGLContext(contextGroup),
    m_isCoreOpenGLProfile(false),
    m_majorVersion(0),
    m_minorVersion(0)
{
    m_qtGLContext = backingQGLContext;

    CVF_ASSERT(m_qtGLContext);
    QGLFormat glFormat = m_qtGLContext->format();
    m_majorVersion = glFormat.majorVersion();
    m_minorVersion = glFormat.minorVersion();
    m_isCoreOpenGLProfile = (glFormat.profile() == QGLFormat::CoreProfile) ? true : false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLContext_QGLContextAdapter_deprecated::~OpenGLContext_QGLContextAdapter_deprecated()
{
    m_qtGLContext = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLContext_QGLContextAdapter_deprecated::initializeContext()
{
    if (!cvf::OpenGLContext::initializeContext())
    {
        return false;
    }

    // Possibly override setting for fixed function support
    if (m_isCoreOpenGLProfile)
    {
        group()->capabilities()->setSupportsFixedFunction(false);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLContext_QGLContextAdapter_deprecated::makeCurrent()
{
    CVF_ASSERT(m_qtGLContext);
    m_qtGLContext->makeCurrent();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLContext_QGLContextAdapter_deprecated::isCurrent() const
{
    if (m_qtGLContext)
    {
        if (QGLContext::currentContext() == m_qtGLContext)
        {
            return true;
        }
    }

    return false;
}




//==================================================================================================
///
/// \class cvfqt::CvfBoundQGLContext_deprecated
/// \ingroup GuiQt
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CvfBoundQGLContext_deprecated::CvfBoundQGLContext_deprecated(cvf::OpenGLContextGroup* contextGroup, const QGLFormat & format)
:   QGLContext(format)
{
    m_cvfGLContext = new OpenGLContext_QGLContextAdapter_deprecated(contextGroup, this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CvfBoundQGLContext_deprecated::~CvfBoundQGLContext_deprecated()
{
    if (m_cvfGLContext.notNull())
    {
        // TODO
        // Need to resolve the case where the Qt QGLcontext (that we're deriving from) is deleted
        // and we are still holding a reference to one or more OpenGLContext objects
        // By the time we get here we expect that we're holding the only reference
        CVF_ASSERT(m_cvfGLContext->refCount() == 1);
        m_cvfGLContext = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OpenGLContext* CvfBoundQGLContext_deprecated::cvfOpenGLContext() const
{
    return const_cast<cvf::OpenGLContext*>(m_cvfGLContext.p());
}


} // namespace cvfqt


