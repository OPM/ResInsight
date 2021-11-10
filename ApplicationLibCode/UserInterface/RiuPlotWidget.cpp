////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuQwtPlotWidget.h"

#include "RiaColorTools.h"
#include "RiaDefines.h"
#include "RiaFontCache.h"
#include "RiaGuiApplication.h"
#include "RiaPlotDefines.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RimPlot.h"

#include "RiuDraggableOverlayFrame.h"
#include "RiuGuiTheme.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtScalePicker.h"

#include "cafAssert.h"

#include <QDebug>
#include <QDrag>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QScrollArea>
#include <QWheelEvent>

#include <algorithm>
#include <limits>
#include <qboxlayout.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget::RiuPlotWidget( RimPlot* plotDefinition, QWidget* parent )
    : QWidget( parent )
    , m_plotDefinition( plotDefinition )
    , m_overlayMargins( 5 )
    , m_plotTitle( "" )
    , m_plotTitleEnabled( true )
{
    CAF_ASSERT( m_plotDefinition );

    // QVBoxLayout* layout = new QVBoxLayout;
    // setLayout( layout );

    // m_plot = new QwtPlot( this );
    // layout->addWidget( m_plot );

    // RiuQwtPlotTools::setCommonPlotBehaviour( m_plot );

    // m_plot->installEventFilter( m_plot );
    // m_plot->canvas()->installEventFilter( this );

    // setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget::~RiuPlotWidget()
{
    if ( m_plotDefinition )
    {
        m_plotDefinition->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RiuPlotWidget::plotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotWidget::isChecked() const
{
    if ( m_plotDefinition )
    {
        return m_plotDefinition->showWindow();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::colSpan() const
{
    if ( m_plotDefinition )
    {
        return m_plotDefinition->colSpan();
    }
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::rowSpan() const
{
    if ( m_plotDefinition )
    {
        return m_plotDefinition->rowSpan();
    }
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiuPlotWidget::plotTitle() const
{
    return m_plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::setPlotTitleEnabled( bool enabled )
{
    m_plotTitleEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotWidget::plotTitleEnabled() const
{
    return m_plotTitleEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPoint RiuPlotWidget::dragStartPosition() const
{
    return m_clickPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::scheduleReplot()
{
    RiaPlotWindowRedrawScheduler::instance()->schedulePlotWidgetReplot( this );
}

//--------------------------------------------------------------------------------------------------
/// Adds an overlay frame. The overlay frame becomes the responsibility of the plot widget
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::addOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    if ( std::find( m_overlayFrames.begin(), m_overlayFrames.end(), overlayFrame ) == m_overlayFrames.end() )
    {
        // TODO: fix overlay frame reparenting..

        // overlayFrame->setParent( m_plot->canvas() );
        // m_overlayFrames.push_back( overlayFrame );
        // updateLayout();
    }
}

//--------------------------------------------------------------------------------------------------
/// Remove the overlay widget. The frame becomes the responsibility of the caller
//--------------------------------------------------------------------------------------------------
void RiuPlotWidget::removeOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    CAF_ASSERT( overlayFrame );

    overlayFrame->hide();
    overlayFrame->setParent( nullptr );
    m_overlayFrames.removeOne( overlayFrame );
};

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RiuPlotWidget::hideEvent( QHideEvent* event )
// {
//     resetPlotItemHighlighting();
//     // TODO: remove?
//     // m_plot->hideEvent( event );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RiuPlotWidget::showEvent( QShowEvent* event )
// {
//     // TODO: remove?
//     // m_plot->showEvent( event );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RiuPlotWidget::resizeEvent( QResizeEvent* event )
// {
//     // TODO: remove???
//     // QwtPlot::resizeEvent( event );
//     updateOverlayFrameLayout();
//     event->accept();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RiuPlotWidget::keyPressEvent( QKeyEvent* event )
// {
//     emit onKeyPressEvent( event );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// QSize RiuPlotWidget::sizeHint() const
// {
//     return QSize( 0, 0 );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// QSize RiuPlotWidget::minimumSizeHint() const
// {
//     return QSize( 0, 0 );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// bool RiuPlotWidget::isZoomerActive() const
// {
//     return false;
// }

// //--------------------------------------------------------------------------------------------------
// /// Empty default implementation
// //--------------------------------------------------------------------------------------------------
// void RiuPlotWidget::endZoomOperations()
// {
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::overlayMargins() const
{
    return m_overlayMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuPlotWidget::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotWidget::defaultMinimumWidth()
{
    return 80;
}
