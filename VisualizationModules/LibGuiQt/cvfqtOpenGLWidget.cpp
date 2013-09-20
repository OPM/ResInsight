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
#include "cvfqtCvfBoundQGLContext.h"
#include "cvfqtOpenGLWidget.h"

namespace cvfqt {



//==================================================================================================
///
/// \class cvfqt::OpenGLWidget
/// \ingroup GuiQt
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLWidget::OpenGLWidget(cvf::OpenGLContextGroup* contextGroup, const QGLFormat& format, QWidget* parent, Qt::WindowFlags f)
:   QGLWidget(new CvfBoundQGLContext(contextGroup, format), parent, NULL, f)
{
    // This constructor can only be used with an empty context group!
    // We're not able to check this up front, but assert that the count is 1 by the time we get here
    CVF_ASSERT(contextGroup->contextCount() == 1);

    if (isValid())
    {
        cvf::ref<cvf::OpenGLContext> myContext = cvfOpenGLContext();
        if (myContext.notNull())
        {
            myContext->initializeContext();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Constructor
///
/// Tries to create a widget that shares OpenGL resources with \a shareWidget
/// To check if creation was actually successful, you must call isValidContext() on the context
/// of the newly created widget. For example:
/// \code
/// myNewWidget->cvfOpenGLContext()->isValidContext();
/// \endcode
///
/// If the context is not valid, sharing failed and the newly created widget/context be discarded.
//--------------------------------------------------------------------------------------------------
OpenGLWidget::OpenGLWidget(OpenGLWidget* shareWidget, QWidget* parent , Qt::WindowFlags f)
:   QGLWidget(new CvfBoundQGLContext(shareWidget->cvfOpenGLContext()->group(), shareWidget->format()), parent, shareWidget, f)
{
    CVF_ASSERT(shareWidget);
    cvf::ref<cvf::OpenGLContext> shareContext = shareWidget->cvfOpenGLContext();
    CVF_ASSERT(shareContext.notNull());

    cvf::ref<cvf::OpenGLContext> myContext = cvfOpenGLContext();
    if (myContext.notNull())
    {
        // We need to check if we actually got a context that shares resources with shareWidget.
        if (isSharing())
        {
            if (isValid())
            {
                CVF_ASSERT(myContext->group() == shareContext->group());
                myContext->initializeContext();
            }
        }
        else
        {
            // If we didn't, we need to remove the newly created context from the group it has been added to since
            // the construction process above has already optimistically added the new context to the existing group.
            // In this case, the newly context is basically defunct so we just shut it down (which will also remove it from the group)
            myContext->shutdownContext();
            CVF_ASSERT(myContext->group() == NULL);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OpenGLContext* OpenGLWidget::cvfOpenGLContext() const
{
    const QGLContext* qglContext = context();
    const CvfBoundQGLContext* contextBinding = dynamic_cast<const CvfBoundQGLContext*>(qglContext);
    CVF_ASSERT(contextBinding);

    return contextBinding->cvfOpenGLContext();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLWidget::cvfShutdownOpenGLContext()
{
    // It should be safe to call shutdown multiple times so this call should 
    // amount to a no-op if the user has already shut down the context
    cvf::ref<cvf::OpenGLContext> myContext = cvfOpenGLContext();
    if (myContext.notNull())
    {
        myContext->shutdownContext();
    }
}


} // namespace cvfqt


