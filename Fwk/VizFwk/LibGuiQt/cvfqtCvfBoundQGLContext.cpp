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
#include "cvfqtCvfBoundQGLContext.h"
#include "cvfqtOpenGLContext.h"

namespace cvfqt {



//==================================================================================================
///
/// \class cvfqt::CvfBoundQGLContext
/// \ingroup GuiQt
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CvfBoundQGLContext::CvfBoundQGLContext(cvf::OpenGLContextGroup* contextGroup, const QGLFormat & format)
:   QGLContext(format)
{
    m_cvfGLContext = new OpenGLContext(contextGroup, this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CvfBoundQGLContext::~CvfBoundQGLContext()
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
cvf::OpenGLContext* CvfBoundQGLContext::cvfOpenGLContext() const
{
    return const_cast<cvf::OpenGLContext*>(m_cvfGLContext.p());
}


} // namespace cvfqt


