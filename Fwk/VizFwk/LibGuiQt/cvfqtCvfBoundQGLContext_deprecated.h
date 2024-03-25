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

#include "cvfOpenGLContext.h"

#include <QGLContext>

namespace cvfqt {


//==================================================================================================
//
// Derived OpenGLContext that adapts a Qt QGLContext
//
//==================================================================================================
class OpenGLContext_QGLContextAdapter_deprecated : public cvf::OpenGLContext
{
public:
    OpenGLContext_QGLContextAdapter_deprecated(cvf::OpenGLContextGroup* contextGroup, QGLContext* backingQGLContext);
    virtual ~OpenGLContext_QGLContextAdapter_deprecated();

    virtual void    makeCurrent();
    virtual bool    isCurrent() const;

private:
    QGLContext*     m_qtGLContext;
};



//==================================================================================================
//
// Utility class used to piggyback OpenGLContext onto Qt's QGLContext
//
//==================================================================================================
class CvfBoundQGLContext_deprecated : public QGLContext
{
public:
    CvfBoundQGLContext_deprecated(cvf::OpenGLContextGroup* contextGroup, const QGLFormat & format);
    virtual ~CvfBoundQGLContext_deprecated();

    cvf::OpenGLContext* cvfOpenGLContext() const;

private:
    cvf::ref<cvf::OpenGLContext>  m_cvfGLContext;
};

}
