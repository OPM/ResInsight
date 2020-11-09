/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuCvfOverlayItemWidget.h"

#include "RiaGuiApplication.h"

#include "cafAssert.h"
#include "cafTitledOverlayFrame.h"
#include "cafViewer.h"

#include "cvfCamera.h"
#include "cvfFramebufferObject.h"
#include "cvfRenderSequence.h"
#include "cvfRenderbufferObject.h"
#include "cvfRendering.h"
#include "cvfqtUtils.h"

#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>

#include "glew/GL/glew.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCvfOverlayItemWidget::RiuCvfOverlayItemWidget( caf::TitledOverlayFrame* overlayItem,
                                                  QWidget*                 parent,
                                                  const int                snapMargins,
                                                  const QColor& backgroundColor /*= QColor( 255, 255, 255, 100 ) */ )
    : RiuDraggableOverlayFrame( parent, snapMargins, backgroundColor )
    , m_overlayItem( overlayItem )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCvfOverlayItemWidget::~RiuCvfOverlayItemWidget()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuCvfOverlayItemWidget::sizeHint() const
{
    auto preferredSize = const_cast<caf::TitledOverlayFrame*>( m_overlayItem.p() )->preferredSize();
    return QSize( preferredSize.x(), preferredSize.y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCvfOverlayItemWidget::renderTo( QPainter* painter, const QRect& paintRect, double scaleX, double scaleY )
{
    unsigned int width  = static_cast<unsigned int>( paintRect.width() );
    unsigned int height = static_cast<unsigned int>( paintRect.height() );

    m_overlayItem->setRenderSize( cvf::Vec2ui( width, height ) );

    QGLFormat glFormat;
    glFormat.setDirectRendering( RiaGuiApplication::instance()->useShaders() );

    caf::Viewer*        viewer        = new caf::Viewer( glFormat, nullptr );
    cvf::OpenGLContext* cvfOglContext = viewer->cvfOpenGLContext();
    viewer->resize( width, height );

    // Create a rendering

    cvf::ref<cvf::Rendering> rendering = new cvf::Rendering;
    m_overlayItem->setLayoutFixedPosition( { 0, 0 } );
    rendering->addOverlayItem( m_overlayItem.p() );

    rendering->camera()->setViewport( 0, 0, width, height );
    rendering->camera()->viewport()->setClearColor( { 1, 1, 1, 0 } );

    cvf::ref<cvf::RenderSequence> renderingSequence = new cvf::RenderSequence;
    renderingSequence->addRendering( rendering.p() );

    if ( RiaGuiApplication::instance()->useShaders() )
    {
        // Set up frame and render buffers

        cvf::ref<cvf::FramebufferObject> fbo = new cvf::FramebufferObject;

        cvf::ref<cvf::RenderbufferObject> rboColor =
            new cvf::RenderbufferObject( cvf::RenderbufferObject::RGBA, width, height );
        cvf::ref<cvf::RenderbufferObject> rboDepth =
            new cvf::RenderbufferObject( cvf::RenderbufferObject::DEPTH_COMPONENT24, width, height );

        fbo->attachDepthRenderbuffer( rboDepth.p() );
        fbo->attachColorRenderbuffer( 0, rboColor.p() );

        fbo->applyOpenGL( cvfOglContext );
        rendering->setTargetFramebuffer( fbo.p() );
        fbo->bind( cvfOglContext );
    }

    renderingSequence->render( cvfOglContext );

    // Read data from frame buffer

    cvf::UByteArray arr( 4 * width * height );
    glReadPixels( 0, 0, static_cast<GLsizei>( width ), static_cast<GLsizei>( height ), GL_RGBA, GL_UNSIGNED_BYTE, arr.ptr() );

    // Create a cvf texture image

    cvf::ref<cvf::TextureImage> img = new cvf::TextureImage;
    img->setData( arr.ptr(), width, height );

    QImage image = cvfqt::Utils::toQImage( *img.p() );
    // image.save("jjsLegendImageTest.png");

    QPixmap pixmap = QPixmap::fromImage( image );

    delete viewer;

    painter->save();
    painter->drawPixmap( paintRect.topLeft(), pixmap );
    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCvfOverlayItemWidget::paintEvent( QPaintEvent* e )
{
    QRect    paintRect = e->rect();
    QPainter painter( this );
    renderTo( &painter, paintRect, 1.0, 1.0 );
}
