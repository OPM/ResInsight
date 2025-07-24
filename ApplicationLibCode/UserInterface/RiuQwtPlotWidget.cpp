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
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPlotDefines.h"
#include "RiaPlotWindowRedrawScheduler.h"

#include "RimPlot.h"
#include "RimPlotCurve.h"
#include "RimSummaryEnsembleTools.h"

#include "RiuDraggableOverlayFrame.h"
#include "RiuGuiTheme.h"
#include "RiuPlotAxis.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtScalePicker.h"

#include "cafAssert.h"

#include "qwt_axis.h"
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

#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget::RiuQwtPlotWidget( RimPlot* plotDefinition, QWidget* parent )
    : RiuPlotWidget( plotDefinition, parent )
    , m_titleRenderingFlags( Qt::AlignHCenter | Qt::TextWordWrap )
    , m_titleFontSize( -1 )
{
    auto* layout = new QVBoxLayout;
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );

    m_plot = new QwtPlot( this );
    m_plot->setAcceptDrops( true );
    layout->addWidget( m_plot );

    RiuQwtPlotTools::setCommonPlotBehaviour( m_plot );

    m_plot->installEventFilter( this );
    m_plot->canvas()->installEventFilter( this );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    if ( plotDefinition )
    {
        connect( this, SIGNAL( plotSelected( bool ) ), plotDefinition, SLOT( onPlotSelected( bool ) ) );
        connect( this, SIGNAL( axisSelected( RiuPlotAxis, bool ) ), plotDefinition, SLOT( onAxisSelected( RiuPlotAxis, bool ) ) );
        connect( this,
                 SIGNAL( plotItemSelected( std::shared_ptr<RiuPlotItem>, bool, int ) ),
                 plotDefinition,
                 SLOT( onPlotItemSelected( std::shared_ptr<RiuPlotItem>, bool, int ) ) );
        connect( this, SIGNAL( onKeyPressEvent( QKeyEvent* ) ), plotDefinition, SLOT( onKeyPressEvent( QKeyEvent* ) ) );
        connect( this, SIGNAL( onWheelEvent( QWheelEvent* ) ), plotDefinition, SLOT( onWheelEvent( QWheelEvent* ) ) );
        connect( this, SIGNAL( destroyed() ), plotDefinition, SLOT( onViewerDestroyed() ) );
    }

    ensureAxisIsCreated( RiuPlotAxis::defaultLeft() );
    ensureAxisIsCreated( RiuPlotAxis::defaultBottom() );
    ensureAxisIsCreated( RiuPlotAxis::defaultRight() );
    ensureAxisIsCreated( RiuPlotAxis::defaultTop() );
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
int RiuQwtPlotWidget::axisTitleFontSize( RiuPlotAxis axis ) const
{
    if ( axisEnabled( axis ) )
    {
        return m_plot->axisFont( toQwtPlotAxis( axis ) ).pointSize();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::axisValueFontSize( RiuPlotAxis axis ) const
{
    if ( axisEnabled( axis ) )
    {
        return m_plot->axisTitle( toQwtPlotAxis( axis ) ).font().pointSize();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisFontsAndAlignment( RiuPlotAxis axis, int titleFontSize, int valueFontSize, bool titleBold, int alignment )
{
    // Axis number font
    auto  qwtAxis  = toQwtPlotAxis( axis );
    QFont axisFont = m_plot->axisFont( qwtAxis );
    axisFont.setPointSize( valueFontSize );
    axisFont.setBold( false );
    m_plot->setAxisFont( qwtAxis, axisFont );

    // Axis title font
    QwtText axisTitle     = m_plot->axisTitle( qwtAxis );
    QFont   axisTitleFont = axisTitle.font();
    axisTitleFont.setPointSize( titleFontSize );
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
    for ( const auto& axisTitlePair : m_axisTitles )
    {
        setAxisFontsAndAlignment( axisTitlePair.first, titleFontSize, valueFontSize, titleBold, alignment );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisTitleText( RiuPlotAxis axis, const QString& title )
{
    m_axisTitles[axis] = title;
    applyAxisTitleToQwt( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisTitleEnabled( RiuPlotAxis axis, bool enable )
{
    m_axisTitlesEnabled[axis] = enable;
    applyAxisTitleToQwt( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setPlotTitle( const QString& plotTitle )
{
    m_plotTitleText = plotTitle;
    applyPlotTitleToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiuQwtPlotWidget::plotTitle() const
{
    return m_plotTitleText;
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
    m_titleFontSize = titleFontSize;

    applyPlotTitleToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setLegendFontSize( int fontSize )
{
    if ( m_plot->legend() )
    {
        QFont font = m_plot->legend()->font();
        font.setPointSize( fontSize );
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
void RiuQwtPlotWidget::insertLegend( RiuPlotWidget::Legend legendPosition )
{
    CAF_ASSERT( legendPosition == RiuPlotWidget::Legend::BOTTOM || legendPosition == RiuPlotWidget::Legend::TOP ||
                legendPosition == RiuPlotWidget::Legend::LEFT || legendPosition == RiuPlotWidget::Legend::RIGHT );

    QwtPlot::LegendPosition pos = QwtPlot::LegendPosition::BottomLegend;
    if ( legendPosition == RiuPlotWidget::Legend::TOP )
    {
        pos = QwtPlot::LegendPosition::TopLegend;
    }
    else if ( legendPosition == RiuPlotWidget::Legend::LEFT )
    {
        pos = QwtPlot::LegendPosition::LeftLegend;
    }
    else if ( legendPosition == RiuPlotWidget::Legend::RIGHT )
    {
        pos = QwtPlot::LegendPosition::RightLegend;
    }
    auto* legend = new QwtLegend( this );
    m_plot->insertLegend( legend, pos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::clearLegend()
{
    m_plot->insertLegend( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQwtPlotWidget::axisRange( RiuPlotAxis axis ) const
{
    QwtInterval interval = m_plot->axisScaleDiv( toQwtPlotAxis( axis ) ).interval();
    return std::make_pair( interval.minValue(), interval.maxValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisRange( RiuPlotAxis axis, double min, double max )
{
    // Note: Especially the Y-axis may be inverted
    if ( m_plot->axisScaleEngine( toQwtPlotAxis( axis ) )->testAttribute( QwtScaleEngine::Inverted ) )
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
void RiuQwtPlotWidget::setAxisInverted( RiuPlotAxis axis, bool isInverted )
{
    auto scaleEngine = m_plot->axisScaleEngine( toQwtPlotAxis( axis ) );
    if ( scaleEngine ) scaleEngine->setAttribute( QwtScaleEngine::Inverted, isInverted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisLabelsAndTicksEnabled( RiuPlotAxis axis, bool enableLabels, bool enableTicks )
{
    m_plot->axisScaleDraw( toQwtPlotAxis( axis ) )->enableComponent( QwtAbstractScaleDraw::Ticks, enableTicks );
    m_plot->axisScaleDraw( toQwtPlotAxis( axis ) )->enableComponent( QwtAbstractScaleDraw::Labels, enableLabels );
    recalculateAxisExtents( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::enableGridLines( RiuPlotAxis axis, bool majorGridLines, bool minorGridLines )
{
    QwtPlotItemList plotItems = m_plot->itemList( QwtPlotItem::Rtti_PlotGrid );
    auto            qwtAxis   = toQwtPlotAxis( axis );
    for ( QwtPlotItem* plotItem : plotItems )
    {
        auto* grid = static_cast<QwtPlotGrid*>( plotItem );
        if ( qwtAxis == QwtAxis::XTop || qwtAxis == QwtAxis::XBottom )
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
void RiuQwtPlotWidget::setMajorTicksList( RiuPlotAxis axis, const QList<double>& majorTicks, double minValue, double maxValue )
{
    auto        qwtAxis = toQwtPlotAxis( axis );
    QwtScaleDiv scaleDiv( minValue, maxValue );
    scaleDiv.setTicks( QwtScaleDiv::TickType::MajorTick, majorTicks );
    m_plot->setAxisScaleDiv( qwtAxis, scaleDiv );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setMajorAndMinorTickIntervals( RiuPlotAxis axis,
                                                      double      majorTickInterval,
                                                      double      minorTickInterval,
                                                      double      minValue,
                                                      double      maxValue )
{
    auto  qwtAxis           = toQwtPlotAxis( axis );
    auto* linearScaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    if ( linearScaleEngine )
    {
        QwtScaleDiv scaleDiv = linearScaleEngine->divideScaleWithExplicitIntervals( minValue, maxValue, majorTickInterval, minorTickInterval );

        m_plot->setAxisScaleDiv( qwtAxis, scaleDiv );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setMajorAndMinorTickIntervalsAndRange( RiuPlotAxis axis,
                                                              double      majorTickInterval,
                                                              double      minorTickInterval,
                                                              double      minTickValue,
                                                              double      maxTickValue,
                                                              double      rangeMin,
                                                              double      rangeMax )
{
    auto  qwtAxis     = toQwtPlotAxis( axis );
    auto* scaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
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
void RiuQwtPlotWidget::setAutoTickIntervalCounts( RiuPlotAxis axis, int maxMajorTickIntervalCount, int maxMinorTickIntervalCount )
{
    m_plot->setAxisMaxMajor( toQwtPlotAxis( axis ), maxMajorTickIntervalCount );
    m_plot->setAxisMaxMinor( toQwtPlotAxis( axis ), maxMinorTickIntervalCount );
    // Reapply axis limits to force Qwt to use the tick settings.
    QwtInterval currentRange = m_plot->axisInterval( toQwtPlotAxis( axis ) );
    setAxisScale( axis, currentRange.minValue(), currentRange.maxValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQwtPlotWidget::majorTickInterval( RiuPlotAxis axis ) const
{
    QwtScaleDiv   scaleDiv   = m_plot->axisScaleDiv( toQwtPlotAxis( axis ) );
    QList<double> majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    if ( majorTicks.size() < 2 ) return 0.0;

    return majorTicks.at( 1 ) - majorTicks.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQwtPlotWidget::minorTickInterval( RiuPlotAxis axis ) const
{
    QwtScaleDiv   scaleDiv   = m_plot->axisScaleDiv( QwtAxis::XTop );
    QList<double> minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    if ( minorTicks.size() < 2 ) return 0.0;

    return minorTicks.at( 1 ) - minorTicks.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::axisExtent( RiuPlotAxis axis ) const
{
    auto [rangeMin, rangeMax] = axisRange( axis );
    if ( std::abs( rangeMax - rangeMin ) < 1.0e-14 ) return 0;

    int  lineExtent = 0;
    auto qwtAxis    = toQwtPlotAxis( axis );

    if ( m_plot->axisScaleDraw( qwtAxis )->hasComponent( QwtAbstractScaleDraw::Ticks ) )
    {
        lineExtent += m_plot->axisScaleDraw( qwtAxis )->maxTickLength();
    }

    if ( m_plot->axisScaleDraw( qwtAxis )->hasComponent( QwtAbstractScaleDraw::Labels ) )
    {
        QFont tickLabelFont = m_plot->axisFont( toQwtPlotAxis( axis ) );
        // Make space for a fairly long value label
        QSize labelSize = QFontMetrics( tickLabelFont ).boundingRect( QString( "9.9e-9" ) ).size();

        if ( axis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || axis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
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
    if ( RiuPlotWidget::handleDragDropEvent( event ) ) return true;

    auto* wheelEvent = dynamic_cast<QWheelEvent*>( event );
    if ( wheelEvent && watched == m_plot->canvas() )
    {
        event->accept();

        emit onWheelEvent( wheelEvent );
        return true;
    }

    auto* mouseEvent = dynamic_cast<QMouseEvent*>( event );
    if ( mouseEvent )
    {
        if ( isZoomerActive() ) return false;

        if ( mouseEvent->type() == QMouseEvent::MouseButtonDblClick )
        {
            if ( m_plotDefinition ) m_plotDefinition->zoomAllForMultiPlot();
            return true;
        }

        if ( ( mouseEvent->type() == QMouseEvent::MouseButtonPress ) &&
             ( mouseEvent->button() == Qt::LeftButton || mouseEvent->button() == Qt::RightButton ) && m_plotDefinition )
        {
            // Select the plot clicked at in the Project Tree for both left and right mouse button clicks
            // If we have plots contained in other plots, select the first visible plot item in the hierarchy

            auto firstVisibleItem = RiuPlotMainWindowTools::firstVisibleAncestorOrThis( m_plotDefinition );
            RiuPlotMainWindowTools::selectAsCurrentItem( firstVisibleItem );
        }

        bool toggleItemInSelection = ( mouseEvent->modifiers() & Qt::ControlModifier ) != 0;

        if ( mouseEvent->type() == QMouseEvent::MouseButtonPress && mouseEvent->button() == Qt::LeftButton )
        {
            m_clickPosition = mouseEvent->pos();
        }

        if ( watched == m_plot && !m_plot->canvas()->geometry().contains( mouseEvent->pos() ) )
        {
            if ( mouseEvent->type() == QMouseEvent::MouseButtonPress && ( mouseEvent->button() == Qt::LeftButton ) && !m_clickPosition.isNull() )
            {
                QWidget* childClicked = m_plot->childAt( m_clickPosition );
                if ( childClicked )
                {
                    auto* scaleWidget = qobject_cast<QwtScaleWidget*>( childClicked );
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
            if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease && mouseEvent->button() == Qt::LeftButton && !m_clickPosition.isNull() )
            {
                endZoomOperations();

                auto hasRecentlyBeenZoomed = [this]() -> bool
                {
                    if ( !m_plotDefinition ) return false;
                    auto lastZoom = m_plotDefinition->valueForKey( "TimeStampZoomOperation" );
                    if ( lastZoom.has_value() )
                    {
                        try
                        {
                            auto retrieved_time = std::any_cast<std::chrono::steady_clock::time_point>( lastZoom );
                            auto t0             = std::chrono::steady_clock::now();

                            auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>( t0 - retrieved_time ).count();
                            if ( timeDiff < 10 )
                            {
                                return true;
                            }
                        }
                        catch ( ... )
                        {
                        }
                    }
                    return false;
                };

                if ( !hasRecentlyBeenZoomed() )
                {
                    // Avoid selecting the curve if a zoom operation has been performed recently
                    // It is confusing to select a curve when the user is trying to zoom

                    selectClosestPlotItem( mouseEvent->pos(), toggleItemInSelection );
                }

                m_clickPosition = QPoint();
                return true;
            }
        }

        onMouseMoveEvent( mouseEvent );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::hideEvent( QHideEvent* event )
{
    resetPlotItemHighlighting();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resizeEvent( QResizeEvent* event )
{
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
    QString plotTitleToApply = m_plotTitleEnabled ? m_plotTitleText : QString( "" );
    QwtText qwtText          = m_plot->title();
    qwtText.setRenderFlags( m_titleRenderingFlags );
    if ( m_titleFontSize > 0 )
    {
        QFont font = qwtText.font();
        font.setPointSize( m_titleFontSize );
        qwtText.setFont( font );
    }
    qwtText.setText( plotTitleToApply );

    // Always set the title, as Qwt does not do anything if the text is the same
    m_plot->setTitle( qwtText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::applyAxisTitleToQwt( RiuPlotAxis axis )
{
    QString titleToApply = m_axisTitlesEnabled[axis] ? m_axisTitles[axis] : QString( "" );
    auto    qwtAxis      = toQwtPlotAxis( axis );
    QwtText axisTitle    = m_plot->axisTitle( qwtAxis );
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
    return { 0, 0 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQwtPlotWidget::minimumSizeHint() const
{
    return { 0, 0 };
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
                overlayBottomRightInWindowCoords.setX( std::min( overlayBottomRightInWindowCoords.x(),
                                                                 canvasBottomRightInWindowCoords.x() - (int)scaling * m_overlayMargins ) );
                overlayBottomRightInWindowCoords.setY( std::min( overlayBottomRightInWindowCoords.y(),
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
    auto scaling = RiaDefines::scalingFactor( paintDevice );

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
    for ( int i = 0; i < QwtAxis::AxisPositions; ++i )
    {
        auto pos   = static_cast<QwtAxis::Position>( i );
        int  count = m_plot->axesCount( pos );
        for ( int id = 0; id < count; id++ )
        {
            QwtAxisId axisId( pos, id );
            if ( scale == m_plot->axisWidget( axisId ) )
            {
                resetPlotItemHighlighting();
                highlightPlotItemsForQwtAxis( axisId );
                scheduleReplot();

                RiuPlotAxis plotAxis = findPlotAxisForQwtAxis( axisId );
                emit        axisSelected( plotAxis, toggleItemInSelection );
                return;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::recalculateAxisExtents( RiuPlotAxis axis )
{
    auto qwtAxis = toQwtPlotAxis( axis );
    if ( qwtAxis.pos == QwtAxis::YLeft || qwtAxis.pos == QwtAxis::YRight )
    {
        int extent = axisExtent( axis );
        m_plot->axisScaleDraw( qwtAxis )->setMinimumExtent( extent );
        setMinimumWidth( defaultMinimumWidth() + extent );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::highlightItemWidthAdjustment()
{
    return 2;
}

//--------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RiuQwtPlotWidget::getParentForOverlay() const
{
    return m_plot->canvas();
}

//--------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::findClosestPlotItem( const QPoint& pos, QwtPlotItem** closestItem, int* closestCurveSampleIndex, double* distanceFromClick ) const
{
    CAF_ASSERT( closestItem && closestCurveSampleIndex && distanceFromClick );

    *closestItem             = nullptr;
    *closestCurveSampleIndex = -1;
    *distanceFromClick       = std::numeric_limits<double>::infinity();

    const QwtPlotItemList& itmList = m_plot->itemList();
    for ( auto it : itmList )
    {
        if ( it->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            auto*  candidateCurve   = static_cast<QwtPlotCurve*>( it );
            double dist             = std::numeric_limits<double>::infinity();
            int    curveSampleIndex = candidateCurve->closestPoint( pos, &dist );

            if ( curveSampleIndex != -1 )
            {
                // Use the sample index before and after to interpolate the closest point on the curve.
                const auto firstIndex  = std::max( 0, curveSampleIndex - 1 );
                const auto secondIndex = std::min( curveSampleIndex + 1, (int)candidateCurve->dataSize() - 1 );

                if ( firstIndex != secondIndex )
                {
                    const QPointF p0 = candidateCurve->sample( firstIndex );
                    const QPointF p1 = candidateCurve->sample( secondIndex );

                    const QwtScaleMap xMap = candidateCurve->plot()->canvasMap( candidateCurve->xAxis() );
                    const QwtScaleMap yMap = candidateCurve->plot()->canvasMap( candidateCurve->yAxis() );

                    const auto x0 = xMap.transform( p0.x() );
                    const auto y0 = yMap.transform( p0.y() );
                    const auto x1 = xMap.transform( p1.x() );
                    const auto y1 = yMap.transform( p1.y() );

                    const double dx = x1 - x0;
                    const double dy = y1 - y0;
                    const double d  = dx * dx + dy * dy;
                    if ( d > 0.0 )
                    {
                        const double t = ( ( pos.x() - x0 ) * dx + ( pos.y() - y0 ) * dy ) / d;
                        if ( t >= 0.0 && t <= 1.0 )
                        {
                            const double cx = x0 + t * dx - pos.x();
                            const double cy = y0 + t * dy - pos.y();
                            dist            = std::sqrt( cx * cx + cy * cy );
                        }
                    }
                }
            }

            if ( dist < *distanceFromClick )
            {
                *closestItem             = candidateCurve;
                *distanceFromClick       = dist;
                *closestCurveSampleIndex = curveSampleIndex;
            }
        }
        else if ( it->rtti() == QwtPlotItem::Rtti_PlotShape )
        {
            auto*   shapeItem = static_cast<QwtPlotShapeItem*>( it );
            QPointF scalePos( m_plot->invTransform( QwtAxis::XBottom, pos.x() ), m_plot->invTransform( QwtAxis::YLeft, pos.y() ) );
            if ( shapeItem->shape().boundingRect().contains( scalePos ) )
            {
                *closestItem       = it;
                *distanceFromClick = 0.0;
            }
        }
        else if ( it->rtti() == QwtPlotItem::Rtti_PlotBarChart )
        {
            auto*   barChart = static_cast<QwtPlotBarChart*>( it );
            QPointF scalePos( m_plot->invTransform( QwtAxis::XBottom, pos.x() ), m_plot->invTransform( QwtAxis::YLeft, pos.y() ) );

            bool horizontal = barChart->orientation() == Qt::Horizontal;
            for ( size_t i = 0; i < barChart->dataSize(); ++i )
            {
                QPointF samplePoint = barChart->sample( (int)i );
                double  dist        = horizontal ? std::abs( samplePoint.x() - scalePos.y() ) : std::abs( samplePoint.x() - scalePos.x() );
                if ( dist < *distanceFromClick )
                {
                    *closestItem             = it;
                    *closestCurveSampleIndex = (int)i;
                    *distanceFromClick       = dist;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RiuPlotCurve*, int> RiuQwtPlotWidget::findClosestCurve( const QPoint& pos, double& distanceFromClick ) const
{
    QwtPlotItem* closestItem = nullptr;

    int    closestCurvePoint = -1;
    QPoint globalPos         = mapToGlobal( pos );
    QPoint localPos          = qwtPlot()->canvas()->mapFromGlobal( globalPos );

    findClosestPlotItem( localPos, &closestItem, &closestCurvePoint, &distanceFromClick );
    auto plotCurve = dynamic_cast<RiuQwtPlotCurve*>( closestItem );
    return std::make_pair( plotCurve, closestCurvePoint );
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
    if ( closestItem )
    {
        RimPlotCurve* clickedCurve = nullptr;
        if ( auto curve = dynamic_cast<RiuPlotCurve*>( closestItem ) )
        {
            clickedCurve = curve->ownerRimCurve();
        }

        std::vector<RimPlotCurve*> curvesToSelect;

        bool wasToggledOff = false;
        if ( toggleItemInSelection )
        {
            for ( auto highlightedCurve : m_hightlightedCurves )
            {
                if ( highlightedCurve == clickedCurve )
                {
                    wasToggledOff = true;
                    continue;
                }

                curvesToSelect.push_back( highlightedCurve );
            }
        }

        if ( !wasToggledOff && clickedCurve )
        {
            if ( distanceFromClick < 100 )
            {
                curvesToSelect.push_back( clickedCurve );
            }
        }

        bool updateCurveOrder = false;
        resetPlotItemHighlighting( updateCurveOrder );

        if ( !curvesToSelect.empty() )
        {
            if ( RimSummaryEnsembleTools::isEnsembleCurve( curvesToSelect.front() ) )
            {
                auto summaryCases = RimSummaryEnsembleTools::summaryCasesFromCurves( curvesToSelect );
                RimSummaryEnsembleTools::selectSummaryCasesInProjectTree( summaryCases );
                RimSummaryEnsembleTools::highlightCurvesForSummaryCases( summaryCases );
            }
            else
            {
                highlightCurvesUpdateOrder( curvesToSelect );
            }
        }
        else
        {
            // Currently used from the matrix plot to highlight the selected cell in the matrix plot, see
            // RimCorrelationMatrixPlot::createMatrix()
            highlightPlotShapeItems( { closestItem } );
        }

        auto plotItem = std::make_shared<RiuQwtPlotItem>( closestItem );
        emit plotItemSelected( plotItem, toggleItemInSelection, distanceFromClick < 10 ? closestCurvePoint : -1 );
    }
    else
    {
        RimSummaryEnsembleTools::resetHighlightAllPlots();

        emit plotSelected( toggleItemInSelection );
    }

    // Always do a replot, as the reset operation also requires replot
    replot();
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
void RiuQwtPlotWidget::highlightCurvesUpdateOrder( const std::vector<RimPlotCurve*>& curves )
{
    highlightPlotCurves( curves );

    updateCurveOrder();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::highlightPlotCurves( const std::vector<RimPlotCurve*>& curves )
{
    if ( !m_plotDefinition || !m_plotDefinition->isCurveHighlightSupported() )
    {
        return;
    }

    auto plotItemList = m_plot->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        auto* riuPlotCurve = dynamic_cast<RiuPlotCurve*>( plotItem );
        if ( !riuPlotCurve ) continue;

        auto currentRimPlotCurve = riuPlotCurve->ownerRimCurve();

        // Do not modify curve objects with no associated Rim object, as the Rim object is used to restore color after highlight
        // manipulation
        if ( !currentRimPlotCurve ) continue;

        auto* plotCurve = dynamic_cast<QwtPlotCurve*>( plotItem );
        if ( plotCurve )
        {
            QPen   existingPen = plotCurve->pen();
            QColor bgColor     = m_plot->canvasBackground().color();

            QColor curveColor = existingPen.color();
            QColor symbolColor;
            QColor symbolLineColor;
            auto   penWidth = existingPen.width();

            auto* symbol = const_cast<QwtSymbol*>( plotCurve->symbol() );
            if ( symbol )
            {
                symbolColor     = symbol->brush().color();
                symbolLineColor = symbol->pen().color();
            }

            double zValue = plotCurve->z();

            auto it = std::find( curves.begin(), curves.end(), currentRimPlotCurve );
            if ( it != curves.end() )
            {
                auto highlightColor = curveColor;

                const double threshold = 0.05;
                if ( curveColor.hsvHueF() > threshold || curveColor.saturationF() > threshold )
                {
                    auto saturation = 1.0;
                    auto value      = 1.0;
                    auto hue        = curveColor.hueF();

                    highlightColor = QColor::fromHsvF( hue, saturation, value );
                }

                existingPen.setColor( highlightColor );
                existingPen.setWidth( penWidth + highlightItemWidthAdjustment() );
                plotCurve->setPen( existingPen );
                plotCurve->setZ( zValue + 100.0 );
                highlightPlotAxes( plotCurve->xAxis(), plotCurve->yAxis() );

                m_hightlightedCurves.push_back( currentRimPlotCurve );
            }
            else
            {
                int    backgroundWeight       = 2;
                QColor blendedColor           = RiaColorTools::blendQColors( bgColor, curveColor, backgroundWeight, 1 );
                QColor blendedSymbolColor     = RiaColorTools::blendQColors( bgColor, symbolColor, backgroundWeight, 1 );
                QColor blendedSymbolLineColor = RiaColorTools::blendQColors( bgColor, symbolLineColor, backgroundWeight, 1 );

                plotCurve->setPen( blendedColor, existingPen.width(), existingPen.style() );
                if ( symbol )
                {
                    symbol->setColor( blendedSymbolColor );
                    symbol->setPen( blendedSymbolLineColor, symbol->pen().width(), symbol->pen().style() );
                }
            }
            m_originalZValues.insert( std::make_pair( plotCurve, zValue ) );

            continue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::highlightPlotShapeItems( const std::set<const QwtPlotItem*>& closestItems )
{
    auto plotItemList = m_plot->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        auto* plotShapeItem = dynamic_cast<QwtPlotShapeItem*>( plotItem );
        if ( plotShapeItem && closestItems.count( plotItem ) > 0 )
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
void RiuQwtPlotWidget::resetPlotItemHighlighting( bool doUpdateCurveOrder )
{
    resetPlotCurveHighlighting();
    resetPlotShapeItemHighlighting();

    resetPlotAxisHighlighting();

    if ( doUpdateCurveOrder ) updateCurveOrder();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RiuQwtPlotWidget::highlightedCurves() const
{
    return m_hightlightedCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resetPlotCurveHighlighting()
{
    m_hightlightedCurves.clear();

    if ( !m_plotDefinition || m_originalZValues.empty() )
    {
        return;
    }

    const auto& plotItemList = m_plot->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        if ( auto* riuPlotCurve = dynamic_cast<RiuQwtPlotCurve*>( plotItem ) )
        {
            if ( auto rimPlotCurve = riuPlotCurve->ownerRimCurve() )
            {
                rimPlotCurve->updateCurveAppearance();
                double zValue = m_originalZValues[riuPlotCurve];
                riuPlotCurve->setZ( zValue );
                continue;
            }
        }
    }
    m_originalZValues.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resetPlotShapeItemHighlighting()
{
    const auto& plotItemList = m_plot->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        auto* plotShapeItem = dynamic_cast<QwtPlotShapeItem*>( plotItem );
        if ( plotShapeItem )
        {
            QPen pen = plotShapeItem->pen();

            auto color = RiuGuiTheme::getColorByVariableName( "markerColor" );

            pen.setColor( color );
            pen.setWidth( 1 );
            plotShapeItem->setPen( pen );
            plotShapeItem->setZ( plotShapeItem->z() - 100.0 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resetPlotAxisHighlighting()
{
    // Reset axis widgets highlighting
    std::vector<QwtAxis::Position> axisPositions = { QwtAxis::Position::YLeft,
                                                     QwtAxis::Position::YRight,
                                                     QwtAxis::Position::XTop,
                                                     QwtAxis::Position::XBottom };

    // Use text color from theme
    QColor  textColor = RiuGuiTheme::getColorByVariableName( "textColor" );
    QString style     = QString( "color: %1;" ).arg( textColor.name() );

    for ( auto pos : axisPositions )
    {
        int count = m_plot->axesCount( pos );
        for ( int i = 0; i < count; i++ )
        {
            QwtAxisId axisId( pos, i );
            auto      axisWidget = m_plot->axisWidget( axisId );
            axisWidget->setStyleSheet( style );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::highlightPlotItemsForQwtAxis( QwtAxisId axisId )
{
    std::vector<RimPlotCurve*> curves;

    auto plotItemList = m_plot->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        auto* qwtPlotCurve = dynamic_cast<RiuQwtPlotCurve*>( plotItem );
        if ( qwtPlotCurve )
        {
            QwtAxisId xAxis = qwtPlotCurve->xAxis();
            QwtAxisId yAxis = qwtPlotCurve->yAxis();
            if ( xAxis == axisId || yAxis == axisId )
            {
                if ( auto curve = qwtPlotCurve->ownerRimCurve() )
                {
                    curves.push_back( curve );
                }
            }
        }
    }

    std::sort( curves.begin(), curves.end() );
    curves.erase( std::unique( curves.begin(), curves.end() ), curves.end() );

    highlightCurvesUpdateOrder( curves );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::highlightPlotAxes( QwtAxisId axisIdX, QwtAxisId axisIdY )
{
    std::vector<QwtAxis::Position> axisPositions = { QwtAxis::Position::YLeft,
                                                     QwtAxis::Position::YRight,
                                                     QwtAxis::Position::XTop,
                                                     QwtAxis::Position::XBottom };

    // Highlight selected axis by toning down the others in same dimension
    for ( auto pos : axisPositions )
    {
        int count = m_plot->axesCount( pos );
        for ( int i = 0; i < count; i++ )
        {
            QwtAxisId axisId( pos, i );

            if ( axisId != axisIdX && axisId != axisIdY )
            {
                auto axisWidget = m_plot->axisWidget( axisId );

                auto color     = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
                auto colorText = color.name();

                axisWidget->setStyleSheet( QString( "color: %1;" ).arg( colorText ) );
            }
        }
    }
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
void RiuQwtPlotWidget::ensureAxisIsCreated( RiuPlotAxis axis )
{
    // Check if the axis already exists
    auto it = m_axisMapping.find( axis );
    if ( it != m_axisMapping.end() ) return;

    // Special handling for default axis (index == 0):
    // These are already created by qwt.
    if ( axis.index() == 0 )
    {
        QwtAxisId newQwtAxis( RiuQwtPlotTools::toQwtPlotAxisEnum( axis.axis() ), 0 );
        m_axisMapping.insert( std::make_pair( axis, newQwtAxis ) );
    }
    else
    {
        auto qwtAxisId = RiuQwtPlotTools::toQwtPlotAxisEnum( axis.axis() );

        int count         = m_plot->axesCount( qwtAxisId );
        int requiredCount = count + 1;

        m_plot->setAxesCount( qwtAxisId, requiredCount );
        QwtAxisId newQwtAxis( qwtAxisId, count );
        m_axisMapping.insert( std::make_pair( axis, newQwtAxis ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::enableAxis( RiuPlotAxis axis, bool isEnabled )
{
    ensureAxisIsCreated( axis );

    m_plot->setAxisVisible( toQwtPlotAxis( axis ), isEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::enableAxisNumberLabels( RiuPlotAxis axis, bool isEnabled )
{
    m_plot->axisScaleDraw( toQwtPlotAxis( axis ) )->enableComponent( QwtAbstractScaleDraw::Labels, isEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::axisEnabled( RiuPlotAxis axis ) const
{
    auto qwtPlotAxis = toQwtPlotAxis( axis );
    if ( qwtPlotAxis.pos < 0 ) return false;

    return m_plot->isAxisVisible( qwtPlotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisMaxMinor( RiuPlotAxis axis, int maxMinor )
{
    m_plot->setAxisMaxMinor( toQwtPlotAxis( axis ), maxMinor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisMaxMajor( RiuPlotAxis axis, int maxMajor )
{
    m_plot->setAxisMaxMajor( toQwtPlotAxis( axis ), maxMajor );
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
void RiuQwtPlotWidget::setAxisAutoScale( RiuPlotAxis axis, bool autoScale )
{
    m_plot->setAxisAutoScale( toQwtPlotAxis( axis ), autoScale );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisScale( RiuPlotAxis axis, double min, double max )
{
    setAxisScale( toQwtPlotAxis( axis ), min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisScale( QwtAxisId axis, double min, double max )
{
    m_plot->setAxisScale( axis, min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget::AxisScaleType RiuQwtPlotWidget::axisScaleType( RiuPlotAxis axis ) const
{
    auto qwtAxis = toQwtPlotAxis( axis );

    auto* logScaleEngine  = dynamic_cast<QwtLogScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    auto* dateScaleEngine = dynamic_cast<QwtDateScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    if ( logScaleEngine != nullptr ) return AxisScaleType::LOGARITHMIC;
    if ( dateScaleEngine != nullptr ) return AxisScaleType::DATE;

    return AxisScaleType::LINEAR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisScaleType( RiuPlotAxis axis, RiuQwtPlotWidget::AxisScaleType axisScaleType )
{
    auto qwtAxis = toQwtPlotAxis( axis );
    setAxisScaleType( qwtAxis, axisScaleType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisScaleType( QwtAxisId axis, RiuQwtPlotWidget::AxisScaleType axisScaleType )
{
    if ( axisScaleType == AxisScaleType::LOGARITHMIC ) m_plot->setAxisScaleEngine( axis, new QwtLogScaleEngine );
    if ( axisScaleType == AxisScaleType::LINEAR ) m_plot->setAxisScaleEngine( axis, new QwtLinearScaleEngine );
    if ( axisScaleType == AxisScaleType::DATE ) m_plot->setAxisScaleEngine( axis, new QwtDateScaleEngine );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::updateAxes()
{
    m_plot->updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RiuQwtPlotWidget::createNextPlotAxis( RiaDefines::PlotAxis axis )
{
    auto qwtAxis = RiuQwtPlotTools::toQwtPlotAxisEnum( axis );

    auto count = m_plot->axesCount( qwtAxis );

    return RiuPlotAxis( axis, count );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurve* RiuQwtPlotWidget::createPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title )
{
    return new RiuQwtPlotCurve( ownerRimCurve, title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::detachItems( RiuPlotWidget::PlotItemType plotItemType )
{
    CAF_ASSERT( plotItemType == RiuPlotWidget::PlotItemType::CURVE );
    if ( plotItemType == RiuPlotWidget::PlotItemType::CURVE ) qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotCurve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QColor& RiuQwtPlotWidget::backgroundColor() const
{
    return m_plot->canvasBackground().color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::isMultiAxisSupported() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::pruneAxes( const std::set<RiuPlotAxis>& usedAxes )
{
    // Make a list of axes to remove since moving the axis invalidates the m_axisMapping iterator
    std::vector<RiuPlotAxis> axesToRemove;
    for ( auto [plotAxis, qwtMapping] : m_axisMapping )
    {
        if ( usedAxes.count( plotAxis ) == 0 )
        {
            axesToRemove.push_back( plotAxis );
        }
    }

    for ( const auto& plotAxis : axesToRemove )
        moveAxis( plotAxis, RiuPlotAxis::defaultLeft() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RiuQwtPlotWidget::findPlotAxisForQwtAxis( const QwtAxisId& qwtAxisId ) const
{
    for ( auto [plotAxis, qwtMapping] : m_axisMapping )
        if ( qwtMapping == qwtAxisId ) return plotAxis;

    CAF_ASSERT( false );
    return RiuPlotAxis::defaultLeft();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::onMouseMoveEvent( QMouseEvent* event )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::moveAxis( RiuPlotAxis oldAxis, RiuPlotAxis newAxis )
{
    auto countAxis = [this]( RiaDefines::PlotAxis axis )
    {
        int count = 0;
        for ( auto [plotAxis, qwtMapping] : m_axisMapping )
        {
            if ( plotAxis.axis() == axis ) count++;
        }
        return count;
    };

    auto isLastItem = [this]( RiuPlotAxis plotAxis, int count )
    {
        auto qwtAxis = toQwtPlotAxis( plotAxis );
        return qwtAxis.id == ( count - 1 );
    };

    auto removeAxis = [this, countAxis, isLastItem]( RiuPlotAxis plotAxis )
    {
        auto qwtAxisPos = RiuQwtPlotTools::toQwtPlotAxisEnum( plotAxis.axis() );

        int count = countAxis( plotAxis.axis() );

        bool isLast = isLastItem( plotAxis, count );
        if ( isLast )
        {
            // If axis to remove is the last axis item on the given side it
            // is safe to let qwt delete it in setAxesCount.
            m_axisMapping.erase( plotAxis );
            m_plot->setAxesCount( qwtAxisPos, count - 1 );
        }
        else
        {
            // When the axis to delete is not the last axis item on the given side
            // we have to move the last axis into the position of the axis to remove.

            // Move the last item into the spot which has been freed up
            auto targetQwtAxis = m_axisMapping.find( plotAxis )->second;

            // Last item on the same side as we are deleting from
            auto sourceQwtAxis  = QwtAxisId( qwtAxisPos, count - 1 );
            auto sourcePlotAxis = findPlotAxisForQwtAxis( sourceQwtAxis );

            // Copy properties of the last axis item
            setAxisScaleType( targetQwtAxis, axisScaleType( sourcePlotAxis ) );
            auto range = axisRange( sourcePlotAxis );
            setAxisScale( targetQwtAxis, range.first, range.second );

            bool autoScale = m_plot->axisAutoScale( sourceQwtAxis );
            m_plot->setAxisAutoScale( targetQwtAxis, autoScale );

            // Finally remove the last item (which has been overwritten the item to remove).
            m_axisMapping.erase( plotAxis );
            m_axisMapping.erase( sourcePlotAxis );
            m_axisMapping.insert( std::make_pair( sourcePlotAxis, targetQwtAxis ) );
            m_plot->setAxesCount( qwtAxisPos, count - 1 );
        }
    };

    removeAxis( oldAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtAxisId RiuQwtPlotWidget::toQwtPlotAxis( RiuPlotAxis plotAxis ) const
{
    auto it = m_axisMapping.find( plotAxis );
    if ( it != m_axisMapping.end() )
    {
        return it->second;
    }

    return { -1, -1 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::highlightPlotItem( const QwtPlotItem* plotItem )
{
    bool refreshCurveOrder = false;
    resetPlotItemHighlighting( refreshCurveOrder );

    if ( auto curve = dynamic_cast<RiuQwtPlotCurve*>( const_cast<QwtPlotItem*>( plotItem ) ) )
    {
        highlightCurvesUpdateOrder( { curve->ownerRimCurve() } );
    }
    else
    {
        highlightPlotShapeItems( { plotItem } );
    }

    replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::onLegendClicked( const QVariant& itemInfo, int index )
{
    if ( !itemInfo.canConvert<QwtPlotItem*>() ) return;

    QwtPlotItem* plotItem = qvariant_cast<QwtPlotItem*>( itemInfo );
    if ( plotItem )
    {
        highlightPlotItem( plotItem );

        auto wrappedPlotItem = std::make_shared<RiuQwtPlotItem>( plotItem );
        emit plotItemSelected( wrappedPlotItem, false, -1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::updateCurveOrder()
{
    emit curveOrderNeedsUpdate();
}
