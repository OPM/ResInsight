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

#include "cvfqtOpenGLWidget.h"

#include "cvfTrace.h"
#include "cvfString.h"
#include "cvfOpenGLContext.h"
#include "cvfLogManager.h"

#include <QOpenGLContext>
#include <QPointer>
#include <QEvent>
#include <QPlatformSurfaceEvent>

namespace cvfqt {



//==================================================================================================
//
// 
//
//==================================================================================================
class ForwardingOpenGLContext_OpenGLWidget : public cvf::OpenGLContext
{
public:
    ForwardingOpenGLContext_OpenGLWidget(cvf::OpenGLContextGroup* contextGroup, QOpenGLWidget* ownerQtOpenGLWidget)
    :   cvf::OpenGLContext(contextGroup),
        m_ownerQtOpenGLWidget(ownerQtOpenGLWidget)
    {
        CVF_ASSERT(contextGroup);

        // In our current usage pattern the owner widget (and its contained Qt OpenGL context)
        // must already be initialized/created
        CVF_ASSERT(m_ownerQtOpenGLWidget);
        CVF_ASSERT(m_ownerQtOpenGLWidget->isValid());
        CVF_ASSERT(m_ownerQtOpenGLWidget->context());
        CVF_ASSERT(m_ownerQtOpenGLWidget->context()->isValid());
    }

    virtual void makeCurrent()
    {
        if (m_ownerQtOpenGLWidget)
        {
            m_ownerQtOpenGLWidget->makeCurrent();
        }
    }
    
    virtual bool isCurrent() const
    {
        QOpenGLContext* ownersContext = m_ownerQtOpenGLWidget ? m_ownerQtOpenGLWidget->context() : NULL;
        if (ownersContext && QOpenGLContext::currentContext() == ownersContext)
        {
            return true;
        }

        return false;
    }

    virtual cvf::OglId defaultFramebufferObject() const
    {
        if (m_ownerQtOpenGLWidget)
        {
            return m_ownerQtOpenGLWidget->defaultFramebufferObject();
        }

        return 0;
    }

    QOpenGLWidget* ownerQtOpenGLWidget()
    {
        return m_ownerQtOpenGLWidget;
    }

private:
    QPointer<QOpenGLWidget>    m_ownerQtOpenGLWidget;
};




//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLWidget::OpenGLWidget(cvf::OpenGLContextGroup* contextGroup, QWidget* parent, Qt::WindowFlags f)
:   QOpenGLWidget(parent, f),
    m_instanceNumber(-1),
    m_initializationState(UNINITIALIZED),
    m_cvfOpenGlContextGroup(contextGroup),
    m_logger(CVF_GET_LOGGER("cee.cvf.qt"))
{
    static int sl_nextInstanceNumber = 0;
    m_instanceNumber = sl_nextInstanceNumber++;

    CVF_LOG_DEBUG(m_logger, cvf::String("OpenGLWidget[%1]::OpenGLWidget()").arg(m_instanceNumber));

    CVF_ASSERT(m_cvfOpenGlContextGroup.notNull());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLWidget::~OpenGLWidget()
{
    CVF_LOG_DEBUG(m_logger, cvf::String("OpenGLWidget[%1]::~OpenGLWidget()").arg(m_instanceNumber));

    if (m_initializationState == IS_INITIALIZED)
    {
        // Make sure we disconnect from the aboutToBeDestroyed signal since after this destructor has been 
        // called, our object is dangling and the call to the slot will cause a crash
        QOpenGLContext* myQtOpenGLContext = context();
        if (myQtOpenGLContext)
        {
            disconnect(myQtOpenGLContext, &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLWidget::qtOpenGLContextAboutToBeDestroyed);
        }
    }

    shutdownCvfOpenGLContext();
}

//--------------------------------------------------------------------------------------------------
/// This is where we do the custom initialization needed for the Qt binding to work
/// 
/// Note that if you re-implement this function in your subclass, you must make 
/// sure to call the base class.
//--------------------------------------------------------------------------------------------------
void OpenGLWidget::initializeGL()
{
    CVF_LOG_DEBUG(m_logger, cvf::String("OpenGLWidget[%1]::initializeGL()").arg(m_instanceNumber));

    if (!isValid())
    {
        CVF_LOG_WARNING(m_logger, cvf::String("OpenGLWidget[%1]: Widget is not valid for initialization").arg(m_instanceNumber));
    }

    // According to Qt doc this function is called once before the first call to paintGL() or resizeGL().
    // Further it is stated that this widget's QOpenGLContext is already current.
    // We should be able to assume that we will only be called with a create, valid and current QOpenGLContext
    //
    // Note that in some scenarios, such as when a widget get reparented, initializeGL() ends up being called
    // multiple times in the lifetime of the widget. In those cases, the widget's associated context is first destroyed 
    // and then a new one is created. This is then followed by a new call to initializeGL() where all OpenGL resources must get reinitialized.

    QOpenGLContext* myQtOpenGLContext = context();
    CVF_ASSERT(myQtOpenGLContext);
    CVF_ASSERT(myQtOpenGLContext->isValid());
    CVF_ASSERT(QOpenGLContext::currentContext() == myQtOpenGLContext);

    if (m_initializationState != IS_INITIALIZED)
    {
        CVF_LOG_DEBUG(m_logger, cvf::String("OpenGLWidget[%1]: Starting internal initialization").arg(m_instanceNumber));

        // This should either be the first call or the result of a destroy/recreate cycle
        CVF_ASSERT(m_cvfForwardingOpenGlContext.isNull());

        // Try and detect the situation where the user is trying to associate incompatible widgets/contexts in the same cvf OpenGLContextGroup
        // The assert below will fail if two incompatible OpenGL contexts end up in the same context group
        if (m_cvfOpenGlContextGroup->contextCount() > 0)
        {
            for (size_t i = 0; i < m_cvfOpenGlContextGroup->contextCount(); i++)
            {
                ForwardingOpenGLContext_OpenGLWidget* existingFwdContext = dynamic_cast<ForwardingOpenGLContext_OpenGLWidget*>(m_cvfOpenGlContextGroup->context(i));
                QOpenGLWidget* existingWidget = existingFwdContext ? existingFwdContext->ownerQtOpenGLWidget() : NULL;
                QOpenGLContext* existingQtContext = existingWidget ? existingWidget->context() : NULL;
                if (existingQtContext)
                {
                    // Assert that these two contexts are actually sharing OpenGL resources
                    CVF_ASSERT(QOpenGLContext::areSharing(existingQtContext, myQtOpenGLContext));
                }
            }
        }

        // Create the actual wrapper/forwarding OpenGL context that implements the cvf::OpenGLContext that we need
        cvf::ref<ForwardingOpenGLContext_OpenGLWidget> myCvfContext = new ForwardingOpenGLContext_OpenGLWidget(m_cvfOpenGlContextGroup.p(), this);

        // Possibly initialize the context group
        if (!m_cvfOpenGlContextGroup->isContextGroupInitialized())
        {
            if (!m_cvfOpenGlContextGroup->initializeContextGroup(myCvfContext.p()))
            {
                CVF_LOG_ERROR(m_logger, cvf::String("OpenGLWidget[%1]: OpenGL context creation failed, could not initialize context group").arg(m_instanceNumber));
                return;
            }
        }

        // All is well, so store pointer to the cvf OpenGL context
        m_cvfForwardingOpenGlContext = myCvfContext;

        const InitializationState prevInitState = m_initializationState;
        m_initializationState = IS_INITIALIZED;

        // Connect to signal so we get notified when Qt's OpenGL context is about to be destroyed
        connect(myQtOpenGLContext, &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLWidget::qtOpenGLContextAboutToBeDestroyed);

        if (prevInitState == UNINITIALIZED)
        {
            // Call overridable notification function to indicate that OpenGL has been initialized
            // Don't do this if we're being re-initialized
            onWidgetOpenGLReady();
        }

        // Trigger a repaint if we're being re-initialized
        if (prevInitState == PENDING_REINITIALIZATION)
        {
            update();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLWidget::resizeGL(int /*w*/, int /*h*/)
{
    // Empty, should normally be implemented in derived class
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLWidget::paintGL()
{
    // No implementation here (nor in QOpenGLWidget::paintGL())
    // Derived classes must re-implement this function in order to do painting.
    //
    // Typical code would go something like this:
    //   cvf::OpenGLContext* currentOglContext = cvfOpenGLContext();
    //   myRenderSequence->render(currentOglContext);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OpenGLContext* OpenGLWidget::cvfOpenGLContext()
{
    return m_cvfForwardingOpenGlContext.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int OpenGLWidget::instanceNumber() const
{
    return m_instanceNumber;
}

//--------------------------------------------------------------------------------------------------
/// Notification function that will be called after internal CVF OpenGL initialization has 
/// successfully completed. This includes both the creation of the CVF OpenGLContext and
/// successful initialization of the OpenGL context group
/// 
/// Can be re-implemented in derived classes to take any needed actions.
//--------------------------------------------------------------------------------------------------
void OpenGLWidget::onWidgetOpenGLReady()
{
    // Base implementation does nothing
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLWidget::qtOpenGLContextAboutToBeDestroyed()
{
    CVF_LOG_DEBUG(m_logger, cvf::String("OpenGLWidget[%1]::qtOpenGLContextAboutToBeDestroyed()").arg(m_instanceNumber));

    if (m_cvfForwardingOpenGlContext.notNull())
    {
        CVF_LOG_DEBUG(m_logger, cvf::String("OpenGLWidget[%1]: Shutting down CVF OpenGL context since Qt context is about to be destroyed").arg(m_instanceNumber));
        shutdownCvfOpenGLContext();
    }

    if (m_initializationState == IS_INITIALIZED)
    {
        m_initializationState = PENDING_REINITIALIZATION;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLWidget::shutdownCvfOpenGLContext()
{
    if (m_cvfForwardingOpenGlContext.notNull())
    {
        makeCurrent();

        m_cvfOpenGlContextGroup->contextAboutToBeShutdown(m_cvfForwardingOpenGlContext.p());
        m_cvfForwardingOpenGlContext = NULL;
    }
}


} // namespace cvfqt

