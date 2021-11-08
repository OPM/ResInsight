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
#include "RiuQwtPlotTools.h"
#include "RiuQwtScalePicker.h"

#include "cafAssert.h"

#include "qwt_date_scale_engine.h"
#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot_barchart.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_shapeitem.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"
#include "qwt_symbol.h"
#include "qwt_text.h"
#include "qwt_text_label.h"

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
RiuQwtPlotWidget::RiuQwtPlotWidget( RimPlot* plotDefinition, QWidget* parent )
    : QWidget( parent )
    , m_plotDefinition( plotDefinition )
    , m_overlayMargins( 5 )
    , m_plotTitle( "" )
    , m_plotTitleEnabled( true )
{
    CAF_ASSERT( m_plotDefinition );

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    m_plot = new QwtPlot( this );
    layout->addWidget( m_plot );

    RiuQwtPlotTools::setCommonPlotBehaviour( m_plot );

    m_plot->installEventFilter( m_plot );
    m_plot->canvas()->installEventFilter( this );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget::~RiuQwtPlotWidget()
{
    if ( m_plotDefinition )
    {
        m_plotDefinition->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RiuQwtPlotWidget::plotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::isChecked() const
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
int RiuQwtPlotWidget::colSpan() const
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
int RiuQwtPlotWidget::rowSpan() const
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
int RiuQwtPlotWidget::axisTitleFontSize( RiaDefines::PlotAxis axis ) const
{
    if ( axisEnabled( axis ) )
    {
        return m_plot->axisFont( RiaDefines::toQwtPlotAxis( axis ) ).pointSize();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::axisValueFontSize( RiaDefines::PlotAxis axis ) const
{
    if ( axisEnabled( axis ) )
    {
        return m_plot->axisTitle( RiaDefines::toQwtPlotAxis( axis ) ).font().pointSize();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisFontsAndAlignment( RiaDefines::PlotAxis axis,
                                                 int                  titleFontSize,
                                                 int                  valueFontSize,
                                                 bool                 titleBold,
                                                 int                  alignment )
{
    int titleFontPixelSize = caf::FontTools::pointSizeToPixelSize( titleFontSize );
    int valueFontPixelSize = caf::FontTools::pointSizeToPixelSize( valueFontSize );

    // Axis number font

    int   qwtAxis  = RiaDefines::toQwtPlotAxis( axis );
    QFont axisFont = m_plot->axisFont( qwtAxis );
    axisFont.setPixelSize( valueFontPixelSize );
    axisFont.setBold( false );
    m_plot->setAxisFont( qwtAxis, axisFont );

    // Axis title font
    QwtText axisTitle     = m_plot->axisTitle( qwtAxis );
    QFont   axisTitleFont = axisTitle.font();
    axisTitleFont.setPixelSize( titleFontPixelSize );
    axisTitleFont.setBold( titleBold );
    axisTitle.setFont( axisTitleFont );
    axisTitle.setRenderFlags( alignment | Qt::TextWordWrap );

    m_plot->setAxisTitle( qwtAxis, axisTitle );
    applyAxisTitleToQwt( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxesFontsAndAlignment( int titleFontSize, int valueFontSize, bool titleBold, int alignment )
{
    for ( auto axisTitlePair : m_axisTitles )
    {
        setAxisFontsAndAlignment( axisTitlePair.first, titleFontSize, valueFontSize, titleBold, alignment );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisTitleText( RiaDefines::PlotAxis axis, const QString& title )
{
    m_axisTitles[axis] = title;
    applyAxisTitleToQwt( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisTitleEnabled( RiaDefines::PlotAxis axis, bool enable )
{
    m_axisTitlesEnabled[axis] = enable;
    applyAxisTitleToQwt( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle = plotTitle;
    applyPlotTitleToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiuQwtPlotWidget::plotTitle() const
{
    return m_plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setPlotTitleEnabled( bool enabled )
{
    m_plotTitleEnabled = enabled;
    applyPlotTitleToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::plotTitleEnabled() const
{
    return m_plotTitleEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setPlotTitleFontSize( int titleFontSize )
{
    auto  title = m_plot->title();
    QFont font  = title.font();
    font.setPixelSize( caf::FontTools::pointSizeToPixelSize( titleFontSize ) );
    title.setFont( font );
    m_plot->setTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setLegendFontSize( int fontSize )
{
    if ( m_plot->legend() )
    {
        QFont font = m_plot->legend()->font();
        font.setPixelSize( caf::FontTools::pointSizeToPixelSize( fontSize ) );
        m_plot->legend()->setFont( font );
        // Set font size for all existing labels
        QList<QwtLegendLabel*> labels = m_plot->legend()->findChildren<QwtLegendLabel*>();
        for ( QwtLegendLabel* label : labels )
        {
            label->setFont( font );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setInternalLegendVisible( bool visible )
{
    if ( visible )
    {
        QwtLegend* legend = new QwtLegend( this );
        m_plot->insertLegend( legend, QwtPlot::BottomLegend );
    }
    else
    {
        m_plot->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtInterval RiuQwtPlotWidget::axisRange( RiaDefines::PlotAxis axis ) const
{
    return m_plot->axisScaleDiv( RiaDefines::toQwtPlotAxis( axis ) ).interval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisRange( RiaDefines::PlotAxis axis, double min, double max )
{
    // Note: Especially the Y-axis may be inverted
    if ( m_plot->axisScaleEngine( RiaDefines::toQwtPlotAxis( axis ) )->testAttribute( QwtScaleEngine::Inverted ) )
    {
        setAxisScale( axis, max, min );
    }
    else
    {
        setAxisScale( axis, min, max );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisInverted( RiaDefines::PlotAxis axis, bool isInverted )
{
    m_plot->axisScaleEngine( RiaDefines::toQwtPlotAxis( axis ) )->setAttribute( QwtScaleEngine::Inverted, isInverted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisLabelsAndTicksEnabled( RiaDefines::PlotAxis axis, bool enableLabels, bool enableTicks )
{
    m_plot->axisScaleDraw( RiaDefines::toQwtPlotAxis( axis ) )->enableComponent( QwtAbstractScaleDraw::Ticks, enableTicks );
    m_plot->axisScaleDraw( RiaDefines::toQwtPlotAxis( axis ) )->enableComponent( QwtAbstractScaleDraw::Labels, enableLabels );
    recalculateAxisExtents( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::enableGridLines( RiaDefines::PlotAxis axis, bool majorGridLines, bool minorGridLines )
{
    QwtPlotItemList plotItems = m_plot->itemList( QwtPlotItem::Rtti_PlotGrid );
    QwtPlot::Axis   qwtAxis   = RiaDefines::toQwtPlotAxis( axis );
    for ( QwtPlotItem* plotItem : plotItems )
    {
        QwtPlotGrid* grid = static_cast<QwtPlotGrid*>( plotItem );
        if ( qwtAxis == QwtPlot::xTop || qwtAxis == QwtPlot::xBottom )
        {
            grid->setXAxis( qwtAxis );
            grid->enableX( majorGridLines );
            grid->enableXMin( minorGridLines );
        }
        else
        {
            grid->setYAxis( qwtAxis );
            grid->enableY( majorGridLines );
            grid->enableYMin( minorGridLines );
        }
        grid->setMajorPen( Qt::lightGray, 1.0, Qt::SolidLine );
        grid->setMinorPen( Qt::lightGray, 1.0, Qt::DashLine );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setMajorAndMinorTickIntervals( RiaDefines::PlotAxis axis,
                                                      double               majorTickInterval,
                                                      double               minorTickInterval,
                                                      double               minValue,
                                                      double               maxValue )
{
    QwtPlot::Axis            qwtAxis     = RiaDefines::toQwtPlotAxis( axis );
    RiuQwtLinearScaleEngine* scaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    if ( scaleEngine )
    {
        QwtScaleDiv scaleDiv =
            scaleEngine->divideScaleWithExplicitIntervals( minValue, maxValue, majorTickInterval, minorTickInterval );

        m_plot->setAxisScaleDiv( qwtAxis, scaleDiv );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setMajorAndMinorTickIntervalsAndRange( RiaDefines::PlotAxis axis,
                                                              double               majorTickInterval,
                                                              double               minorTickInterval,
                                                              double               minTickValue,
                                                              double               maxTickValue,
                                                              double               rangeMin,
                                                              double               rangeMax )
{
    QwtPlot::Axis            qwtAxis     = RiaDefines::toQwtPlotAxis( axis );
    RiuQwtLinearScaleEngine* scaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    if ( scaleEngine )
    {
        QwtScaleDiv scaleDiv = scaleEngine->divideScaleWithExplicitIntervalsAndRange( minTickValue,
                                                                                      maxTickValue,
                                                                                      majorTickInterval,
                                                                                      minorTickInterval,
                                                                                      rangeMin,
                                                                                      rangeMax );

        m_plot->setAxisScaleDiv( qwtAxis, scaleDiv );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAutoTickIntervalCounts( RiaDefines::PlotAxis axis,
                                                  int                  maxMajorTickIntervalCount,
                                                  int                  maxMinorTickIntervalCount )
{
    m_plot->setAxisMaxMajor( RiaDefines::toQwtPlotAxis( axis ), maxMajorTickIntervalCount );
    m_plot->setAxisMaxMinor( RiaDefines::toQwtPlotAxis( axis ), maxMinorTickIntervalCount );
    // Reapply axis limits to force Qwt to use the tick settings.
    QwtInterval currentRange = m_plot->axisInterval( RiaDefines::toQwtPlotAxis( axis ) );
    setAxisScale( axis, currentRange.minValue(), currentRange.maxValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQwtPlotWidget::majorTickInterval( RiaDefines::PlotAxis axis ) const
{
    QwtScaleDiv   scaleDiv   = m_plot->axisScaleDiv( RiaDefines::toQwtPlotAxis( axis ) );
    QList<double> majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    if ( majorTicks.size() < 2 ) return 0.0;

    return majorTicks.at( 1 ) - majorTicks.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQwtPlotWidget::minorTickInterval( RiaDefines::PlotAxis axis ) const
{
    QwtScaleDiv   scaleDiv   = m_plot->axisScaleDiv( QwtPlot::xTop );
    QList<double> minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    if ( minorTicks.size() < 2 ) return 0.0;

    return minorTicks.at( 1 ) - minorTicks.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::axisExtent( RiaDefines::PlotAxis axis ) const
{
    if ( std::abs( axisRange( axis ).maxValue() - axisRange( axis ).minValue() ) < 1.0e-14 ) return 0;

    int           lineExtent = 0;
    QwtPlot::Axis qwtAxis    = RiaDefines::toQwtPlotAxis( axis );

    if ( m_plot->axisScaleDraw( qwtAxis )->hasComponent( QwtAbstractScaleDraw::Ticks ) )
    {
        lineExtent += m_plot->axisScaleDraw( qwtAxis )->maxTickLength();
    }

    if ( m_plot->axisScaleDraw( qwtAxis )->hasComponent( QwtAbstractScaleDraw::Labels ) )
    {
        QFont tickLabelFont = m_plot->axisFont( RiaDefines::toQwtPlotAxis( axis ) );
        // Make space for a fairly long value label
        QSize labelSize = QFontMetrics( tickLabelFont ).boundingRect( QString( "9.9e-9" ) ).size();

        if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
        {
            lineExtent = labelSize.width();
        }
        else
        {
            lineExtent = labelSize.height();
        }
    }

    if ( !m_plot->axisTitle( qwtAxis ).text().isEmpty() )
    {
        auto it = m_axisTitlesEnabled.find( axis );
        if ( it != m_axisTitlesEnabled.end() && it->second )
        {
            QFont titleFont = m_plot->axisTitle( qwtAxis ).font();
            // Label is aligned vertically on vertical axes
            // So height is sufficient in both cases.
            lineExtent += QFontMetrics( titleFont ).height();
        }
    }

    return lineExtent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::frameIsInFrontOfThis( const QRect& frameGeometry )
{
    QRect ownGeometry = m_plot->canvas()->geometry();
    ownGeometry.translate( m_plot->geometry().topLeft() );

    if ( frameGeometry.bottom() < ownGeometry.center().y() )
    {
        return true;
    }
    else if ( frameGeometry.left() < ownGeometry.left() && frameGeometry.top() < ownGeometry.center().y() )
    {
        return true;
    }
    else
    {
        QRect intersection = ownGeometry.intersected( frameGeometry );

        double ownArea          = double( ownGeometry.height() ) * double( ownGeometry.width() );
        double frameArea        = double( frameGeometry.height() ) * double( frameGeometry.width() );
        double intersectionArea = double( intersection.height() ) * double( intersection.width() );
        if ( intersectionArea > 0.8 * std::min( ownArea, frameArea ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPoint RiuQwtPlotWidget::dragStartPosition() const
{
    return m_clickPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::scheduleReplot()
{
    RiaPlotWindowRedrawScheduler::instance()->schedulePlotWidgetReplot( this );
}

//--------------------------------------------------------------------------------------------------
/// Adds an overlay frame. The overlay frame becomes the responsibility of the plot widget
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::addOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    if ( std::find( m_overlayFrames.begin(), m_overlayFrames.end(), overlayFrame ) == m_overlayFrames.end() )
    {
        overlayFrame->setParent( m_plot->canvas() );
        m_overlayFrames.push_back( overlayFrame );
        updateLayout();
    }
}

//--------------------------------------------------------------------------------------------------
/// Remove the overlay widget. The frame becomes the responsibility of the caller
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::removeOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    CAF_ASSERT( overlayFrame );

    overlayFrame->hide();
    overlayFrame->setParent( nullptr );
    m_overlayFrames.removeOne( overlayFrame );
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::updateLayout()
{
    m_plot->updateLayout();
    updateOverlayFrameLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::updateLegend()
{
    m_plot->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::eventFilter( QObject* watched, QEvent* event )
{
    QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>( event );
    if ( wheelEvent && watched == m_plot->canvas() )
    {
        event->accept();

        emit onWheelEvent( wheelEvent );
        return true;
    }

    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>( event );
    if ( mouseEvent )
    {
        if ( isZoomerActive() ) return false;

        bool toggleItemInSelection = ( mouseEvent->modifiers() & Qt::ControlModifier ) != 0;

        if ( mouseEvent->type() == QMouseEvent::MouseButtonPress && mouseEvent->button() == Qt::LeftButton )
        {
            m_clickPosition = mouseEvent->pos();
        }

        if ( watched == this && !m_plot->canvas()->geometry().contains( mouseEvent->pos() ) )
        {
            if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease && ( mouseEvent->button() == Qt::LeftButton ) &&
                 !m_clickPosition.isNull() )
            {
                QWidget* childClicked = m_plot->childAt( m_clickPosition );
                if ( childClicked )
                {
                    QwtScaleWidget* scaleWidget = qobject_cast<QwtScaleWidget*>( childClicked );
                    if ( scaleWidget )
                    {
                        onAxisSelected( scaleWidget, toggleItemInSelection );
                        m_clickPosition = QPoint();
                        return true;
                    }
                }
                else
                {
                    endZoomOperations();
                    emit plotSelected( toggleItemInSelection );
                    m_clickPosition = QPoint();
                    return true;
                }
            }
        }
        else if ( watched == m_plot->canvas() )
        {
            if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease && mouseEvent->button() == Qt::LeftButton &&
                 !m_clickPosition.isNull() )
            {
                endZoomOperations();
                selectClosestPlotItem( mouseEvent->pos(), toggleItemInSelection );
                m_clickPosition = QPoint();
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::hideEvent( QHideEvent* event )
{
    resetPlotItemHighlighting();
    // TODO: remove?
    // m_plot->hideEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::showEvent( QShowEvent* event )
{
    // TODO: remove?
    // m_plot->showEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resizeEvent( QResizeEvent* event )
{
    // TODO: remove???
    // QwtPlot::resizeEvent( event );
    updateOverlayFrameLayout();
    event->accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::keyPressEvent( QKeyEvent* event )
{
    emit onKeyPressEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::applyPlotTitleToQwt()
{
    QString plotTitleToApply = m_plotTitleEnabled ? m_plotTitle : QString( "" );
    QwtText plotTitle        = m_plot->title();
    plotTitle.setRenderFlags( Qt::AlignHCenter | Qt::TextSingleLine );
    if ( plotTitleToApply != plotTitle.text() )
    {
        plotTitle.setText( plotTitleToApply );
        m_plot->setTitle( plotTitle );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::applyAxisTitleToQwt( RiaDefines::PlotAxis axis )
{
    QString       titleToApply = m_axisTitlesEnabled[axis] ? m_axisTitles[axis] : QString( "" );
    QwtPlot::Axis qwtAxis      = RiaDefines::toQwtPlotAxis( axis );
    QwtText       axisTitle    = m_plot->axisTitle( qwtAxis );
    if ( titleToApply != axisTitle.text() )
    {
        axisTitle.setText( titleToApply );

        m_plot->setAxisTitle( qwtAxis, axisTitle );
    }
    recalculateAxisExtents( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlotWidget::sizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlotWidget::minimumSizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::isZoomerActive() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::endZoomOperations()
{
}

//--------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::renderTo( QPainter* painter, const QRect& targetRect, double scaling )
{
    static_cast<QwtPlotCanvas*>( m_plot->canvas() )->setPaintAttribute( QwtPlotCanvas::BackingStore, false );

    QPoint plotTopLeftInWindowCoords = targetRect.topLeft();

    QRectF canvasRect = m_plot->plotLayout()->canvasRect();
    QPoint canvasTopLeftInPlotCoords( canvasRect.topLeft().x() * scaling, canvasRect.topLeft().y() * scaling );
    QPoint canvasBottomRightInPlotCoords( canvasRect.bottomRight().x(), canvasRect.bottomRight().y() );

    QPoint canvasTopLeftInWindowCoords     = canvasTopLeftInPlotCoords + plotTopLeftInWindowCoords;
    QPoint canvasBottomRightInWindowCoords = canvasBottomRightInPlotCoords + plotTopLeftInWindowCoords;

    QwtPlotRenderer renderer( this );
    renderer.render( m_plot, painter, targetRect );
    static_cast<QwtPlotCanvas*>( m_plot->canvas() )->setPaintAttribute( QwtPlotCanvas::BackingStore, true );

    for ( RiuDraggableOverlayFrame* overlayFrame : m_overlayFrames )
    {
        if ( overlayFrame->isVisible() )
        {
            QPoint overlayTopLeftInCanvasCoords = overlayFrame->frameGeometry().topLeft();

            QPoint overlayTopLeftInWindowCoords = overlayTopLeftInCanvasCoords + canvasTopLeftInWindowCoords;
            {
                QRect overlayRect = overlayFrame->frameGeometry();
                QSize desiredSize = overlayRect.size();
                QSize minimumSize = overlayFrame->minimumSizeHint();
                QSize actualSize  = desiredSize.expandedTo( minimumSize );
                overlayRect.moveTo( overlayTopLeftInWindowCoords );
                overlayRect.setSize( actualSize );

                QPoint overlayBottomRightInWindowCoords = overlayRect.bottomRight();
                overlayBottomRightInWindowCoords.setX(
                    std::min( overlayBottomRightInWindowCoords.x(),
                              canvasBottomRightInWindowCoords.x() - (int)scaling * m_overlayMargins ) );
                overlayBottomRightInWindowCoords.setY(
                    std::min( overlayBottomRightInWindowCoords.y(),
                              canvasBottomRightInWindowCoords.y() - (int)scaling * m_overlayMargins ) );
                overlayRect.moveBottomRight( overlayBottomRightInWindowCoords );
                overlayFrame->renderTo( painter, overlayRect );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::renderTo( QPaintDevice* paintDevice, const QRect& targetRect )
{
    int      resolution = paintDevice->logicalDpiX();
    double   scaling    = resolution / static_cast<double>( RiaGuiApplication::applicationResolution() );
    QPainter painter( paintDevice );
    renderTo( &painter, targetRect, scaling );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::overlayMargins() const
{
    return m_overlayMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuQwtPlotWidget::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::onAxisSelected( QwtScaleWidget* scale, bool toggleItemInSelection )
{
    int axisId = -1;
    for ( int i = 0; i < QwtPlot::axisCnt; ++i )
    {
        if ( scale == m_plot->axisWidget( i ) )
        {
            axisId = i;
        }
    }
    emit axisSelected( axisId, toggleItemInSelection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::recalculateAxisExtents( RiaDefines::PlotAxis axis )
{
    QwtPlot::Axis qwtAxis = RiaDefines::toQwtPlotAxis( axis );
    if ( qwtAxis == QwtPlot::yLeft || qwtAxis == QwtPlot::yRight )
    {
        int extent = axisExtent( axis );
        m_plot->axisScaleDraw( qwtAxis )->setMinimumExtent( extent );
        setMinimumWidth( defaultMinimumWidth() + extent );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::updateOverlayFrameLayout()
{
    const int spacing = 5;

    int xpos                 = spacing;
    int ypos                 = spacing;
    int widthOfCurrentColumn = 0;

    QSize canvasSize = m_plot->canvas()->size();
    QSize maxFrameSize( canvasSize.width() - 2 * m_overlayMargins, canvasSize.height() - 2 * m_overlayMargins );

    for ( RiuDraggableOverlayFrame* frame : m_overlayFrames )
    {
        if ( frame )
        {
            QSize minFrameSize     = frame->minimumSizeHint();
            QSize desiredFrameSize = frame->sizeHint();

            int width  = std::min( std::max( minFrameSize.width(), desiredFrameSize.width() ), maxFrameSize.width() );
            int height = std::min( std::max( minFrameSize.height(), desiredFrameSize.height() ), maxFrameSize.height() );

            frame->resize( width, height );

            if ( frame->anchorCorner() == RiuDraggableOverlayFrame::AnchorCorner::TopLeft )
            {
                if ( ypos + frame->height() + spacing > m_plot->canvas()->height() && widthOfCurrentColumn > 0 )
                {
                    xpos += spacing + widthOfCurrentColumn;
                    ypos                 = spacing;
                    widthOfCurrentColumn = 0;
                }
                frame->move( xpos, ypos );
                ypos += frame->height() + spacing;
                widthOfCurrentColumn = std::max( widthOfCurrentColumn, frame->width() );
            }
            else if ( frame->anchorCorner() == RiuDraggableOverlayFrame::AnchorCorner::TopRight )
            {
                QRect  frameRect      = frame->frameGeometry();
                QRect  canvasRect     = m_plot->canvas()->rect();
                QPoint canvasTopRight = canvasRect.topRight();
                frameRect.moveTopRight( QPoint( canvasTopRight.x() - spacing, canvasTopRight.y() + spacing ) );
                frame->move( frameRect.topLeft() );
            }
            frame->show();
        }
    }
}

//--------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::findClosestPlotItem( const QPoint& pos,
                                            QwtPlotItem** closestItem,
                                            int*          closestCurvePoint,
                                            double*       distanceFromClick ) const
{
    CAF_ASSERT( closestItem && closestCurvePoint && distanceFromClick );

    // Force empty defaults
    *closestItem       = nullptr;
    *closestCurvePoint = -1;
    *distanceFromClick = std::numeric_limits<double>::infinity();

    const QwtPlotItemList& itmList = m_plot->itemList();
    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); it++ )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve* candidateCurve = static_cast<QwtPlotCurve*>( *it );
            double        dist           = std::numeric_limits<double>::infinity();
            int           curvePoint     = candidateCurve->closestPoint( pos, &dist );
            if ( dist < *distanceFromClick )
            {
                *closestItem       = candidateCurve;
                *distanceFromClick = dist;
                *closestCurvePoint = curvePoint;
            }
        }
        else if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotShape )
        {
            QwtPlotShapeItem* shapeItem = static_cast<QwtPlotShapeItem*>( *it );
            QPointF           scalePos( m_plot->invTransform( QwtPlot::xBottom, pos.x() ),
                              m_plot->invTransform( QwtPlot::yLeft, pos.y() ) );
            if ( shapeItem->shape().boundingRect().contains( scalePos ) )
            {
                *closestItem       = *it;
                *distanceFromClick = 0.0;
            }
        }
        else if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotBarChart )
        {
            QwtPlotBarChart* barChart = static_cast<QwtPlotBarChart*>( *it );
            QPointF          scalePos( m_plot->invTransform( QwtPlot::xBottom, pos.x() ),
                              m_plot->invTransform( QwtPlot::yLeft, pos.y() ) );

            bool horizontal = barChart->orientation() == Qt::Horizontal;
            for ( size_t i = 0; i < barChart->dataSize(); ++i )
            {
                QPointF samplePoint = barChart->sample( (int)i );
                double  dist        = horizontal ? std::abs( samplePoint.x() - scalePos.y() )
                                         : std::abs( samplePoint.x() - scalePos.x() );
                if ( dist < *distanceFromClick )
                {
                    *closestItem       = *it;
                    *closestCurvePoint = (int)i;
                    *distanceFromClick = dist;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::selectClosestPlotItem( const QPoint& pos, bool toggleItemInSelection /*= false*/ )
{
    QwtPlotItem* closestItem       = nullptr;
    double       distanceFromClick = std::numeric_limits<double>::infinity();
    int          closestCurvePoint = -1;

    findClosestPlotItem( pos, &closestItem, &closestCurvePoint, &distanceFromClick );

    RiuPlotMainWindowTools::showPlotMainWindow();
    resetPlotItemHighlighting();
    if ( closestItem && distanceFromClick < 20 )
    {
        // TODO: highlight all selected curves
        highlightPlotItem( closestItem );
        emit plotItemSelected( closestItem, toggleItemInSelection, distanceFromClick < 10 ? closestCurvePoint : -1 );

        scheduleReplot();
    }
    else
    {
        emit plotSelected( toggleItemInSelection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::defaultMinimumWidth()
{
    return 80;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::replot()
{
    m_plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::highlightPlotItem( const QwtPlotItem* closestItem )
{
    // NB! Create a copy of the item list before the loop to avoid invalidated iterators when iterating the list
    // plotCurve->setZ() causes the ordering of items in the list to change
    auto plotItemList = m_plot->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        QwtPlotCurve*     plotCurve     = dynamic_cast<QwtPlotCurve*>( plotItem );
        QwtPlotShapeItem* plotShapeItem = dynamic_cast<QwtPlotShapeItem*>( plotItem );
        if ( plotCurve )
        {
            QPen   existingPen = plotCurve->pen();
            QColor bgColor     = m_plot->canvasBackground().color();

            QColor curveColor = existingPen.color();
            QColor symbolColor;
            QColor symbolLineColor;

            QwtSymbol* symbol = const_cast<QwtSymbol*>( plotCurve->symbol() );
            if ( symbol )
            {
                symbolColor     = symbol->brush().color();
                symbolLineColor = symbol->pen().color();
            }

            double zValue = plotCurve->z();
            if ( plotCurve == closestItem )
            {
                plotCurve->setZ( zValue + 100.0 );
            }
            else
            {
                QColor blendedColor           = RiaColorTools::blendQColors( bgColor, curveColor, 3, 1 );
                QColor blendedSymbolColor     = RiaColorTools::blendQColors( bgColor, symbolColor, 3, 1 );
                QColor blendedSymbolLineColor = RiaColorTools::blendQColors( bgColor, symbolLineColor, 3, 1 );

                plotCurve->setPen( blendedColor, existingPen.width(), existingPen.style() );
                if ( symbol )
                {
                    symbol->setColor( blendedSymbolColor );
                    symbol->setPen( blendedSymbolLineColor, symbol->pen().width(), symbol->pen().style() );
                }
            }
            CurveColors curveColors = { curveColor, symbolColor, symbolLineColor };
            m_originalCurveColors.insert( std::make_pair( plotCurve, curveColors ) );
            m_originalCurveColors.insert( std::make_pair( plotCurve, curveColors ) );
            m_originalZValues.insert( std::make_pair( plotCurve, zValue ) );
        }
        else if ( plotShapeItem && plotItem == closestItem )
        {
            QPen pen = plotShapeItem->pen();
            pen.setColor( QColor( Qt::green ) );
            pen.setWidth( 3 );
            plotShapeItem->setPen( pen );
            plotShapeItem->setZ( plotShapeItem->z() + 100.0 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resetPlotItemHighlighting()
{
    // NB! Create a copy of the item list before the loop to avoid invalidated iterators when iterating the list
    // plotCurve->setZ() causes the ordering of items in the list to change
    auto plotItemList = m_plot->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        QwtPlotCurve*     plotCurve     = dynamic_cast<QwtPlotCurve*>( plotItem );
        QwtPlotShapeItem* plotShapeItem = dynamic_cast<QwtPlotShapeItem*>( plotItem );
        if ( plotCurve && m_originalCurveColors.count( plotCurve ) )
        {
            const QPen& existingPen = plotCurve->pen();
            auto        colors      = m_originalCurveColors[plotCurve];
            double      zValue      = m_originalZValues[plotCurve];

            plotCurve->setPen( colors.lineColor, existingPen.width(), existingPen.style() );
            plotCurve->setZ( zValue );
            QwtSymbol* symbol = const_cast<QwtSymbol*>( plotCurve->symbol() );
            if ( symbol )
            {
                symbol->setColor( colors.symbolColor );
                symbol->setPen( colors.symbolLineColor, symbol->pen().width(), symbol->pen().style() );
            }
        }
        else if ( plotShapeItem )
        {
            QPen pen = plotShapeItem->pen();

            auto color = RiuGuiTheme::getColorByVariableName( "markerColor" );

            pen.setColor( color );
            pen.setWidth( 1 );
            plotShapeItem->setPen( pen );
            plotShapeItem->setZ( plotShapeItem->z() - 100.0 );
        }
    }
    m_originalCurveColors.clear();
    m_originalZValues.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlot* RiuQwtPlotWidget::qwtPlot() const
{
    return m_plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::enableAxis( RiaDefines::PlotAxis axis, bool isEnabled )
{
    m_plot->enableAxis( RiaDefines::toQwtPlotAxis( axis ), isEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::axisEnabled( RiaDefines::PlotAxis axis ) const
{
    return m_plot->axisEnabled( RiaDefines::toQwtPlotAxis( axis ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisMaxMinor( RiaDefines::PlotAxis axis, int maxMinor )
{
    m_plot->setAxisMaxMinor( RiaDefines::toQwtPlotAxis( axis ), maxMinor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisMaxMajor( RiaDefines::PlotAxis axis, int maxMajor )
{
    m_plot->setAxisMaxMajor( RiaDefines::toQwtPlotAxis( axis ), maxMajor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::removeEventFilter()
{
    m_plot->removeEventFilter( m_plot );
    m_plot->canvas()->removeEventFilter( m_plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisAutoScale( RiaDefines::PlotAxis axis, bool autoScale )
{
    m_plot->setAxisAutoScale( RiaDefines::toQwtPlotAxis( axis ), autoScale );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisScale( RiaDefines::PlotAxis axis, double min, double max )
{
    m_plot->setAxisScale( RiaDefines::toQwtPlotAxis( axis ), min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget::AxisScaleType RiuQwtPlotWidget::axisScaleType( RiaDefines::PlotAxis axis ) const
{
    QwtPlot::Axis qwtAxis = RiaDefines::toQwtPlotAxis( axis );

    QwtLogScaleEngine*  logScaleEngine  = dynamic_cast<QwtLogScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    QwtDateScaleEngine* dateScaleEngine = dynamic_cast<QwtDateScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    if ( logScaleEngine != nullptr )
        return AxisScaleType::LOGARITHMIC;
    else if ( dateScaleEngine != nullptr )
        return AxisScaleType::DATE;

    return AxisScaleType::LINEAR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisScaleType( RiaDefines::PlotAxis axis, RiuQwtPlotWidget::AxisScaleType axisScaleType )
{
    QwtPlot::Axis qwtAxis = RiaDefines::toQwtPlotAxis( axis );

    if ( axisScaleType == AxisScaleType::LOGARITHMIC ) m_plot->setAxisScaleEngine( qwtAxis, new QwtLogScaleEngine );
    if ( axisScaleType == AxisScaleType::LINEAR ) m_plot->setAxisScaleEngine( qwtAxis, new QwtLinearScaleEngine );
    if ( axisScaleType == AxisScaleType::DATE ) m_plot->setAxisScaleEngine( qwtAxis, new QwtDateScaleEngine );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::updateAxes()
{
    m_plot->updateAxes();
}
