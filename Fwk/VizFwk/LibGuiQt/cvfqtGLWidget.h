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

#include "cvfBase.h"
#include "cvfObject.h"

#include <QGLWidget>

namespace cvf {
    class OpenGLContext;
    class OpenGLContextGroup;
    class Logger;
}

namespace cvfqt {



//==================================================================================================
//
// 
//
//==================================================================================================
class GLWidget : public QGLWidget
{
public:
    GLWidget(cvf::OpenGLContextGroup* contextGroup, const QGLFormat& format, QWidget* parent, Qt::WindowFlags f = Qt::WindowFlags());
    GLWidget(GLWidget* shareWidget, QWidget* parent , Qt::WindowFlags f = Qt::WindowFlags());
    ~GLWidget();

    cvf::OpenGLContext* cvfOpenGLContext();
    void                cvfShutdownOpenGLContext();

    void                logOpenGLInfo();

protected:
    virtual void        initializeGL();
    virtual void        resizeGL(int width, int height);
    virtual void        paintGL();
    virtual bool        eventFilter(QObject* watched, QEvent* event);

private:
    void                qtOpenGLContextAboutToBeDestroyed();

private:
    cvf::ref<cvf::OpenGLContextGroup>   m_cvfOpenGLContextGroup;
    cvf::ref<cvf::OpenGLContext>        m_cvfForwardingOpenGLContext;
    cvf::ref<cvf::Logger>               m_logger;
};

}
