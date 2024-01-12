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

#include "cvfqtGLWidget.h"

#include "cvfOpenGLContextGroup.h"
#include "cvfOpenGLContext.h"
#include "cvfLogManager.h"
#include "cvfTrace.h"

#include <QPointer>
#include <QEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPlatformSurfaceEvent>

namespace cvfqt {



//==================================================================================================
//
// 
//
//==================================================================================================
class ForwardingOpenGLContext_GLWidget : public cvf::OpenGLContext
{
public:
    ForwardingOpenGLContext_GLWidget(cvf::OpenGLContextGroup* contextGroup, QGLWidget* ownerQtGLWidget)
    :   cvf::OpenGLContext(contextGroup),
        m_ownerQtGLWidget(ownerQtGLWidget)
    {
        CVF_ASSERT(contextGroup);

        // In our current usage pattern the owner widget (and its contained Qt OpenGL context) must already be initialized/created
        CVF_ASSERT(m_ownerQtGLWidget);
        CVF_ASSERT(m_ownerQtGLWidget->isValid());
        CVF_ASSERT(m_ownerQtGLWidget->context());
        CVF_ASSERT(m_ownerQtGLWidget->context()->isValid());
    }

    virtual void makeCurrent()
    {
        if (m_ownerQtGLWidget)
        {
            m_ownerQtGLWidget->makeCurrent();
        }
    }
    
    virtual bool isCurrent() const
    {
        const QGLContext* ownersQGLContext = m_ownerQtGLWidget ? m_ownerQtGLWidget->context() : NULL;
        if (ownersQGLContext && QGLContext::currentContext() == ownersQGLContext)
        {
            return true;
        }

        return false;
    }

    virtual cvf::OglId defaultFramebufferObject() const
    {
        if (m_ownerQtGLWidget)
        {
            const QGLContext* ownersQGLContext = m_ownerQtGLWidget->context();
            const QOpenGLContext* ownersQOpenGLContext = ownersQGLContext ? ownersQGLContext->contextHandle() : NULL;
            return ownersQOpenGLContext ? ownersQOpenGLContext->defaultFramebufferObject() : 0;
        }

        return 0;
    }

private:
    QPointer<QGLWidget>    m_ownerQtGLWidget;
};




//==================================================================================================
///
/// \class cvfqt::GLWidget
/// \ingroup GuiQt
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor, use this for the first or only widget in your application.
//--------------------------------------------------------------------------------------------------
GLWidget::GLWidget(cvf::OpenGLContextGroup* contextGroup, const QGLFormat& format, QWidget* parent, Qt::WindowFlags f)
:   QGLWidget(format, parent, NULL, f),
    m_cvfOpenGLContextGroup(contextGroup),
    m_logger(CVF_GET_LOGGER("cee.cvf.qt"))
{
    // Must pass a context group
    CVF_ASSERT(m_cvfOpenGLContextGroup.notNull());

    // Only the first widget in a context group can be created using this constructor
    // All following widgets must be created using the constructor overload that takes a shareWidget
    CVF_ASSERT(m_cvfOpenGLContextGroup->contextCount() == 0);

    // The isValid() call will return true if the widget has a valid GL rendering context; otherwise returns false.
    // The widget will be invalid if the system has no OpenGL support.
    if (!isValid())
    {
        CVF_LOG_ERROR(m_logger, "Widget creation failed, the system has no OpenGL support");
        return;
    }


    // The Qt docs for QGLWidget and all previous experience indicates that it is OK to do OpenGL related
    // initialization in QGLWidget's constructor (contrary to QOpenGLWidget).  
    // Note that the Qt docs still hint that initialization should be deferred to initializeGL(). If we ever
    // experience problems related to doing initialization here, we should probably move the initialization.

    // Since we're in the constructor we must ensure this widget's context is current before initialization
    makeCurrent();

    const QGLContext* myQtGLContext = context();
    CVF_ASSERT(myQtGLContext);
    CVF_ASSERT(myQtGLContext->isValid());
    CVF_ASSERT(QGLContext::currentContext() == myQtGLContext);

    m_cvfForwardingOpenGLContext = new ForwardingOpenGLContext_GLWidget(m_cvfOpenGLContextGroup.p(), this);

    if (!m_cvfOpenGLContextGroup->initializeContextGroup(m_cvfForwardingOpenGLContext.p()))
    {
        CVF_LOG_ERROR(m_logger, "Error initializing context group");
    }


    // Install our event filter
    installEventFilter(this);

    // If we're using Qt5 or above, we can get hold of a QOpenGLContext from our QGLContext
    // Connect to QOpenGLContext's aboutToBeDestroyed signal so we get notified when Qt's OpenGL context is about to be destroyed
    connect(myQtGLContext->contextHandle(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::qtOpenGLContextAboutToBeDestroyed);
}


//--------------------------------------------------------------------------------------------------
/// Constructor, creates a widget sharing the OpenGL resources with the specified widget.
//--------------------------------------------------------------------------------------------------
GLWidget::GLWidget(GLWidget* shareWidget, QWidget* parent , Qt::WindowFlags f)
:   QGLWidget(shareWidget->context()->requestedFormat(), parent, shareWidget, f),
    m_logger(CVF_GET_LOGGER("cee.cvf.qt"))
{
    // Note that when calling QGLWidget's constructor in the initializer list above, we *must* use the
    // constructor overload that takes a format as its first parameter. If we do not do this, the default QGLFormat
    // will be used, and it may not be compatible with the format of the shareWidget.
    // We grab hold of the format from the shareWidget, but note that we do that by calling requestedFormat()
    // instead of just format() to ensure that the format being requested here is the same as the one that was actually used
    // in the "first widget constructor" and not whatever format the first widget ended up with after construction.

    // Requires that a share widget is passed in as parameter
    CVF_ASSERT(shareWidget);

    // If the share widget doesn't have a CVF OpenGL context something went wrong when it was created and we cannot continue
    cvf::ref<cvf::OpenGLContext> shareContext = shareWidget->cvfOpenGLContext();
    CVF_ASSERT(shareContext.notNull());

    if (!isValid())
    {
        CVF_LOG_ERROR(m_logger, "Widget creation failed, the system has no OpenGL support");
        return;
    }

    // We need to check if we actually got a context that shares resources with the passed shareWidget.
    if (!isSharing())
    {
        CVF_LOG_ERROR(m_logger, "Widget creation failed, unable to create a shared OpenGL context");
        return;
    }


    // Since we're in the constructor we must ensure this widget's context is current before initialization
    makeCurrent();

    const QGLContext* myQtGLContext = context();
    CVF_ASSERT(myQtGLContext);
    CVF_ASSERT(myQtGLContext->isValid());
    CVF_ASSERT(QGLContext::currentContext() == myQtGLContext);

    m_cvfOpenGLContextGroup = shareContext->group();
    CVF_ASSERT(m_cvfOpenGLContextGroup.notNull());

    m_cvfForwardingOpenGLContext = new ForwardingOpenGLContext_GLWidget(m_cvfOpenGLContextGroup.p(), this);

    // Normally, the context group should already be initialized when the first widget in the group was created.
    // Still, for good measure do an initialization here. It will amount to a no-op if context group is already initialized
    if (!m_cvfOpenGLContextGroup->initializeContextGroup(m_cvfForwardingOpenGLContext.p()))
    {
        CVF_LOG_ERROR(m_logger, "Error initializing context group using sharing widget");
    }

    
    // Install our event filter
    installEventFilter(this);

    // If we're using Qt5 or above, we can get hold of a QOpenGLContext from our QGLContext
    // Connect to QOpenGLContext's aboutToBeDestroyed signal so we get notified when Qt's OpenGL context is about to be destroyed
    connect(myQtGLContext->contextHandle(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::qtOpenGLContextAboutToBeDestroyed);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
GLWidget::~GLWidget()
{
    cvf::Trace::show("GLWidget::~GLWidget()");

    // Make sure we disconnect from the aboutToBeDestroyed signal since after this destructor has been 
    // called, out object is dangling and the call to the slot will cause a crash
    const QGLContext* myQtGLContext = context();
    const QOpenGLContext* myQtOpenGLContext = myQtGLContext ? myQtGLContext->contextHandle() : NULL;
    if (myQtOpenGLContext)
    {
        disconnect(myQtOpenGLContext, &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::qtOpenGLContextAboutToBeDestroyed);
    }

    // For Qt5, our testing indicates that once we hit the widget's destructor it may be too late
    // to try and do any cleanup of OpenGL resources. Most of the time it works, but typically it will fail when
    // we're shutting down the application by closing the main window. In such cases it seems that the OpenGL 
    // context cannot be made current anymore. 
    // 
    // One solution to this is to do an explicit shutdown of the context while before the system
    // start shutting down. One traditional way of doing this is to iterate over all GLWidgets and call
    // the cvfShutdownOpenGLContext() explicitly, eg from QMainWindow::closeEvent()
    //
    // In our quest to get a notification just before the QGLContext dies we have also tried to go via 
    // QGLContext's QOpenGLContext and connect to the aboutToBeDestroyed signal. This signal does indeed trigger
    // before the QGLContext dies, but it seems that at this point we are no longer able to make the context 
    // current which is a requirement before we can do our OpenGL related cleanup.
    //
    // Another promising solution that seems to work reliably is to install an event filter and respond to
    // the QEvent::PlatformSurface event. See our eventFilter() override

    // For Qt4 it seems that doing OpenGL related cleanup in the destructor is OK
    // We're able to make the context current and delete any OpenGL resources necessary through the cvfShutdownOpenGLContext();


    // Note that calling this function is safe even if the context has already been shut down.
    // It may however fail/assert if the context hasn't already been shut down and if we're unable to make the OpenGL context current
    cvfShutdownOpenGLContext();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OpenGLContext* GLWidget::cvfOpenGLContext()
{
    return m_cvfForwardingOpenGLContext.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GLWidget::cvfShutdownOpenGLContext()
{
    if (m_cvfForwardingOpenGLContext.notNull())
    {
        makeCurrent();
        
        // If we're not able to make the context current, the eventual un-initialize that will happen
        // in the context group when we remove the last context will fail.
        const QGLContext* myContext = context();
        const QGLContext* currContext = QGLContext::currentContext();
        if (myContext != currContext)
        {
            CVF_LOG_WARNING(m_logger, "Could not make the widget's OpenGL context current for context shutdown");
        }
        
        m_cvfOpenGLContextGroup->contextAboutToBeShutdown(m_cvfForwardingOpenGLContext.p());
        m_cvfForwardingOpenGLContext = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GLWidget::initializeGL()
{
    //cvf::Trace::show("GLWidget::initializeGL()");

    // According to Qt doc this function is called once before the first call to paintGL() or resizeGL(), 
    // and then once whenever the widget has been assigned a new QGLContext. There is no need to call makeCurrent() because 
    // this has already been done when this function is called.

    if (m_cvfForwardingOpenGLContext.isNull())
    {
        CVF_LOG_ERROR(m_logger, "Unexpected error in GLWidget::initializeGL(), no forwarding OpenGL context present");
        return;
    }

    // Initialization of context group should already be done, but for good measure
    CVF_ASSERT(m_cvfOpenGLContextGroup.notNull());
    if (!m_cvfOpenGLContextGroup->isContextGroupInitialized())
    {
        CVF_LOG_DEBUG(m_logger, "Doing late initialization of context group in GLWidget::initializeGL()");
        if (!m_cvfOpenGLContextGroup->initializeContextGroup(m_cvfForwardingOpenGLContext.p()))
        {
            CVF_LOG_ERROR(m_logger, "Initialization of context group in GLWidget::initializeGL() failed");
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GLWidget::resizeGL(int /*width*/, int /*height*/)
{
    // Intentionally empty, and no implementation in QGLWidget::resizeGL() either.
    // Should normally be implemented in derived class
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GLWidget::paintGL()
{
    // No implementation here and no significant implementation in QGLWidget either.
    // In Qt4 QGLWidget::paintGL() does nothing. In Qt5 QGLWidget::paintGL() merely clears the depth and color buffer.
    // Derived classes must reimplement this function in order to do painting.
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GLWidget::qtOpenGLContextAboutToBeDestroyed()
{
    //cvf::Trace::show("GLWidget::qtOpenGLContextAboutToBeDestroyed()");

    cvfShutdownOpenGLContext();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool GLWidget::eventFilter(QObject* watched, QEvent* event)
{
    // The most reliable way we have found of detecting when an OpenGL context is about to be destroyed is
    // hooking into the QEvent::PlatformSurface event and checking for the SurfaceAboutToBeDestroyed event type.
    // From the Qt doc: 
    //   The underlying native surface will be destroyed immediately after this event.
    //   The SurfaceAboutToBeDestroyed event type is useful as a means of stopping rendering to a platform window before it is destroyed.
    if (event->type() == QEvent::PlatformSurface)
    {
        QPlatformSurfaceEvent* platformSurfaceEvent = static_cast<QPlatformSurfaceEvent*>(event);
        if (platformSurfaceEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
        {
            CVF_LOG_DEBUG(m_logger, "Shutting down OpenGL context in response to platform surface about to be destroyed");
            cvfShutdownOpenGLContext();
        }
    }

    // Reading the Qt docs it can seem like reparenting of the QGLWidget might pose a problem for us.
    // According to the doc, a change of parent will cause the widget's QGLContext to be deleted and a new one
    // to be created. In Qt4, this does indeed seem to happen, whereas for Qt5 it does not. Still, it appears that Qt4 makes
    // an effort to make the new QGLContext compatible with the old one, and does some tricks to set up temporary OpenGL
    // resource sharing so that we may not actually be affected by this since we're actually not storing any references to
    // the QGLContext, but accessing it indirectly through the widget in our ForwardingOpenGLContext. 
    // May also want to look out for 
    if (event->type() == QEvent::ParentChange)
    {
        CVF_LOG_DEBUG(m_logger, "cvfqt::GLWidget has been reparented. This may cause OpenGL issues");
    }
    else if (event->type() == QEvent::ParentAboutToChange)
    {
        CVF_LOG_DEBUG(m_logger, "cvfqt::GLWidget is about to change parent. This may cause OpenGL issues");
    }

    return QGLWidget::eventFilter(watched, event);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GLWidget::logOpenGLInfo()
{
    CVF_LOG_INFO(m_logger, "Starting logging of OpenGL info (cvfqt::GLWidget)...");

    cvf::String sQtVerInfo = cvf::String("Qt version: %1 (run-time=%2)").arg(QT_VERSION_STR).arg(qVersion());
    CVF_LOG_INFO(m_logger, sQtVerInfo);

    if (!context() || !isValid())
    {
        CVF_LOG_WARNING(m_logger, "QGLWidget does not have a valid GL rendering context, reported info will not be correct!");
    }

    // Log output from querying Qt
    CVF_LOG_INFO(m_logger, "Qt OpenGL format info:");

    const QGLFormat qglFormat = format();
    const QGLFormat::OpenGLContextProfile profile = qglFormat.profile();
    const QGLFormat::OpenGLVersionFlags qglVersionFlags = QGLFormat::openGLVersionFlags();

    cvf::String sInfo = cvf::String("  info: version %1.%2, depthSize: %3, software: %4, doubleBuffer: %5, sampleBuffers: %6 (size: %7)")
        .arg(qglFormat.majorVersion()).arg(qglFormat.minorVersion())
        .arg(qglFormat.depthBufferSize())
        .arg(qglFormat.directRendering() ? "no" : "yes")
        .arg(qglFormat.doubleBuffer() ? "yes" : "no")
        .arg(qglFormat.sampleBuffers() ? "yes" : "no")
        .arg(qglFormat.samples());
    CVF_LOG_INFO(m_logger, sInfo);

    cvf::String sProfile = "UNKNOWN";
    if      (profile == QGLFormat::NoProfile)            sProfile = "NoProfile";
    else if (profile == QGLFormat::CoreProfile)          sProfile = "CoreProfile";
    else if (profile == QGLFormat::CompatibilityProfile) sProfile = "CompatibilityProfile";
    CVF_LOG_INFO(m_logger, "  context profile: " + sProfile);

    cvf::String sVersionsPresent = cvf::String("  versions present: 1.1: %1, 2.0: %2, 2.1: %3, 3.0: %4, 3.3: %5, 4.0: %6, ES2: %7")
        .arg(qglVersionFlags & QGLFormat::OpenGL_Version_1_1 ? "yes" : "no")
        .arg(qglVersionFlags & QGLFormat::OpenGL_Version_2_0 ? "yes" : "no")
        .arg(qglVersionFlags & QGLFormat::OpenGL_Version_2_1 ? "yes" : "no")
        .arg(qglVersionFlags & QGLFormat::OpenGL_Version_3_0 ? "yes" : "no")
        .arg(qglVersionFlags & QGLFormat::OpenGL_Version_3_3 ? "yes" : "no")
        .arg(qglVersionFlags & QGLFormat::OpenGL_Version_4_0 ? "yes" : "no")
        .arg(qglVersionFlags & QGLFormat::OpenGL_ES_Version_2_0 ? "yes" : "no");
    CVF_LOG_INFO(m_logger, sVersionsPresent);

    CVF_LOG_INFO(m_logger, "  is sharing: " + cvf::String(isSharing() ? "yes" : "no"));


    // Log the information we have gathered when initializing the context group
    const cvf::OpenGLInfo oglInfo = m_cvfOpenGLContextGroup->info();
    CVF_LOG_INFO(m_logger, "OpenGL info:");
    CVF_LOG_INFO(m_logger, "  version:  " + oglInfo.version());
    CVF_LOG_INFO(m_logger, "  vendor:   " + oglInfo.vendor());
    CVF_LOG_INFO(m_logger, "  renderer: " + oglInfo.renderer());


    // Lastly, query OpenGL implementation directly if this context is current
    GLint smoothLineWidthRange[2] = { -1, -1 };
    GLint smoothPointSizeRange[2] = { -1, -1 };
    GLint aliasedLineWidthRange[2] = { -1, -1 };
    GLint aliasedPointSizeRange[2] = { -1, -1 };

    // Note that GL_LINE_WIDTH_RANGE and GL_SMOOTH_LINE_WIDTH_RANGE are synonyms (0x0B22)
    // Likewise for GL_POINT_SIZE_RANGE and GL_SMOOTH_POINT_SIZE_RANGE are synonyms (0x0B12)
#ifndef GL_ALIASED_LINE_WIDTH_RANGE
    #define GL_ALIASED_LINE_WIDTH_RANGE      0x846E
#endif
#ifndef GL_ALIASED_POINT_SIZE_RANGE
    #define GL_ALIASED_POINT_SIZE_RANGE      0x846D
#endif

    const bool thisContextIsCurrent = context() == QGLContext::currentContext();
    if (thisContextIsCurrent)
    {
        glGetIntegerv(GL_LINE_WIDTH_RANGE, smoothLineWidthRange);
        glGetIntegerv(GL_POINT_SIZE_RANGE, smoothPointSizeRange);
        if (qglVersionFlags & QGLFormat::OpenGL_Version_1_2)
        {
            glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, aliasedLineWidthRange);
            glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, aliasedPointSizeRange);
        }
    }
    else
    {
        CVF_LOG_WARNING(m_logger, "Cannot query OpenGL directly, QGLWidget's context is not current");
    }

    cvf::String sLineInfo = cvf::String("OpenGL line width range: aliased lines: %1 to %2, smooth lines: %3 to %4").arg(aliasedLineWidthRange[0]).arg(aliasedLineWidthRange[1]).arg(smoothLineWidthRange[0]).arg(smoothLineWidthRange[1]);
    cvf::String sPointInfo = cvf::String("OpenGL point size range: aliased points: %1 to %2, smooth points: %3 to %4").arg(aliasedPointSizeRange[0]).arg(aliasedPointSizeRange[1]).arg(smoothPointSizeRange[0]).arg(smoothPointSizeRange[1]);
    CVF_LOG_INFO(m_logger, sLineInfo);
    CVF_LOG_INFO(m_logger, sPointInfo);

    CVF_LOG_INFO(m_logger, "Finished logging of OpenGL info");
}


} // namespace cvfqt


