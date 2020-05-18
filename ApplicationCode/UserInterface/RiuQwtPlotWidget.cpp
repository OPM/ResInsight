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
#include "RiaFontCache.h"
#include "RiaGuiApplication.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RimPlot.h"

#include "RiuDraggableOverlayFrame.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtScalePicker.h"

#include "cafAssert.h"

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot_barchart.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_shapeitem.h"
#include "qwt_scale_draw.h"
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
#include <cfloat>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget::RiuQwtPlotWidget( RimPlot* plotDefinition, QWidget* parent )
    : QwtPlot( parent )
    , m_plotDefinition( plotDefinition )
    , m_overlayMargins( 5 )
{
    CAF_ASSERT( m_plotDefinition );
    RiuQwtPlotTools::setCommonPlotBehaviour( this );

    this->installEventFilter( this );
    this->canvas()->installEventFilter( this );

    this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
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
int RiuQwtPlotWidget::axisTitleFontSize( QwtPlot::Axis axis ) const
{
    if ( this->axisEnabled( axis ) )
    {
        return this->axisFont( axis ).pointSize();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::axisValueFontSize( QwtPlot::Axis axis ) const
{
    if ( this->axisEnabled( axis ) )
    {
        return this->axisTitle( axis ).font().pointSize();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisFontsAndAlignment( QwtPlot::Axis axis,
                                                 int           titleFontSize,
                                                 int           valueFontSize,
                                                 bool          titleBold,
                                                 int           alignment )
{
    // Axis number font
    QFont axisFont = this->axisFont( axis );
    axisFont.setPixelSize( RiaFontCache::pointSizeToPixelSize( valueFontSize ) );
    axisFont.setBold( false );
    this->setAxisFont( axis, axisFont );

    // Axis title font
    QwtText axisTitle     = this->axisTitle( axis );
    QFont   axisTitleFont = axisTitle.font();
    axisTitleFont.setPixelSize( RiaFontCache::pointSizeToPixelSize( titleFontSize ) );
    axisTitleFont.setBold( titleBold );
    axisTitle.setFont( axisTitleFont );
    axisTitle.setRenderFlags( alignment | Qt::TextWordWrap );

    setAxisTitle( axis, axisTitle );
    applyAxisTitleToQwt( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisTitleText( QwtPlot::Axis axis, const QString& title )
{
    m_axisTitles[axis] = title;
    applyAxisTitleToQwt( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisTitleEnabled( QwtPlot::Axis axis, bool enable )
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
    auto  title = this->title();
    QFont font  = title.font();
    font.setPixelSize( RiaFontCache::pointSizeToPixelSize( titleFontSize ) );
    title.setFont( font );
    setTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setLegendFontSize( int fontSize )
{
    if ( legend() )
    {
        QFont font = legend()->font();
        font.setPixelSize( RiaFontCache::pointSizeToPixelSize( fontSize ) );
        legend()->setFont( font );
        // Set font size for all existing labels
        QList<QwtLegendLabel*> labels = legend()->findChildren<QwtLegendLabel*>();
        for ( QwtLegendLabel* label : labels )
        {
            label->setFont( font );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setLegendVisible( bool visible )
{
    if ( visible )
    {
        QwtLegend* legend = new QwtLegend( this );
        this->insertLegend( legend, BottomLegend );
    }
    else
    {
        this->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtInterval RiuQwtPlotWidget::axisRange( QwtPlot::Axis axis ) const
{
    return axisScaleDiv( axis ).interval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisRange( QwtPlot::Axis axis, double min, double max )
{
    // Note: Especially the Y-axis may be inverted
    if ( axisScaleEngine( axis )->testAttribute( QwtScaleEngine::Inverted ) )
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
void RiuQwtPlotWidget::setAxisInverted( QwtPlot::Axis axis )
{
    axisScaleEngine( axis )->setAttribute( QwtScaleEngine::Inverted, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAxisLabelsAndTicksEnabled( QwtPlot::Axis axis, bool enableLabels, bool enableTicks )
{
    this->axisScaleDraw( axis )->enableComponent( QwtAbstractScaleDraw::Ticks, enableTicks );
    this->axisScaleDraw( axis )->enableComponent( QwtAbstractScaleDraw::Labels, enableLabels );
    recalculateAxisExtents( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::enableGridLines( QwtPlot::Axis axis, bool majorGridLines, bool minorGridLines )
{
    QwtPlotItemList plotItems = this->itemList( QwtPlotItem::Rtti_PlotGrid );
    for ( QwtPlotItem* plotItem : plotItems )
    {
        QwtPlotGrid* grid = static_cast<QwtPlotGrid*>( plotItem );
        if ( axis == QwtPlot::xTop || axis == QwtPlot::xBottom )
        {
            grid->setXAxis( axis );
            grid->enableX( majorGridLines );
            grid->enableXMin( minorGridLines );
        }
        else
        {
            grid->setYAxis( axis );
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
void RiuQwtPlotWidget::setMajorAndMinorTickIntervals( QwtPlot::Axis axis,
                                                      double        majorTickInterval,
                                                      double        minorTickInterval,
                                                      double        minValue,
                                                      double        maxValue )
{
    RiuQwtLinearScaleEngine* scaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( this->axisScaleEngine( axis ) );
    if ( scaleEngine )
    {
        QwtScaleDiv scaleDiv =
            scaleEngine->divideScaleWithExplicitIntervals( minValue, maxValue, majorTickInterval, minorTickInterval );

        this->setAxisScaleDiv( axis, scaleDiv );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setMajorAndMinorTickIntervalsAndRange( QwtPlot::Axis axis,
                                                              double        majorTickInterval,
                                                              double        minorTickInterval,
                                                              double        minTickValue,
                                                              double        maxTickValue,
                                                              double        rangeMin,
                                                              double        rangeMax )
{
    RiuQwtLinearScaleEngine* scaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( this->axisScaleEngine( axis ) );
    if ( scaleEngine )
    {
        QwtScaleDiv scaleDiv = scaleEngine->divideScaleWithExplicitIntervalsAndRange( minTickValue,
                                                                                      maxTickValue,
                                                                                      majorTickInterval,
                                                                                      minorTickInterval,
                                                                                      rangeMin,
                                                                                      rangeMax );

        this->setAxisScaleDiv( axis, scaleDiv );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setAutoTickIntervalCounts( QwtPlot::Axis axis,
                                                  int           maxMajorTickIntervalCount,
                                                  int           maxMinorTickIntervalCount )
{
    this->setAxisMaxMajor( axis, maxMajorTickIntervalCount );
    this->setAxisMaxMinor( axis, maxMinorTickIntervalCount );
    // Reapply axis limits to force Qwt to use the tick settings.
    QwtInterval currentRange = this->axisInterval( axis );
    setAxisScale( axis, currentRange.minValue(), currentRange.maxValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQwtPlotWidget::majorTickInterval( QwtPlot::Axis axis ) const
{
    QwtScaleDiv   scaleDiv   = this->axisScaleDiv( axis );
    QList<double> majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    if ( majorTicks.size() < 2 ) return 0.0;

    return majorTicks.at( 1 ) - majorTicks.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQwtPlotWidget::minorTickInterval( QwtPlot::Axis axis ) const
{
    QwtScaleDiv   scaleDiv   = this->axisScaleDiv( QwtPlot::xTop );
    QList<double> minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    if ( minorTicks.size() < 2 ) return 0.0;

    return minorTicks.at( 1 ) - minorTicks.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtPlotWidget::axisExtent( QwtPlot::Axis axis ) const
{
    if ( std::abs( axisRange( axis ).maxValue() - axisRange( axis ).minValue() ) < 1.0e-14 ) return 0;

    int lineExtent = 0;

    if ( this->axisScaleDraw( axis )->hasComponent( QwtAbstractScaleDraw::Ticks ) )
    {
        lineExtent += this->axisScaleDraw( axis )->maxTickLength();
    }

    if ( this->axisScaleDraw( axis )->hasComponent( QwtAbstractScaleDraw::Labels ) )
    {
        QFont tickLabelFont = axisFont( axis );
        // Make space for a fairly long value label
        QSize labelSize = QFontMetrics( tickLabelFont ).boundingRect( QString( "9.9e-9" ) ).size();

        if ( axis == QwtPlot::yLeft || axis == QwtPlot::yRight )
        {
            lineExtent = labelSize.width();
        }
        else
        {
            lineExtent = labelSize.height();
        }
    }

    if ( !axisTitle( axis ).text().isEmpty() )
    {
        auto it = m_axisTitlesEnabled.find( axis );
        if ( it != m_axisTitlesEnabled.end() && it->second )
        {
            QFont titleFont = axisTitle( axis ).font();
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
    QRect ownGeometry = this->canvas()->geometry();
    ownGeometry.translate( this->geometry().topLeft() );

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
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::stashWidgetStates()
{
    m_plotStyleSheet.stashWidgetStates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::restoreWidgetStates()
{
    m_plotStyleSheet.restoreWidgetStates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setWidgetState( const QString& widgetState )
{
    caf::UiStyleSheet::clearWidgetStates( this );
    m_plotStyleSheet.setWidgetState( this, widgetState );
}

//--------------------------------------------------------------------------------------------------
/// Adds an overlay frame. The overlay frame becomes the responsibility of the plot widget
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::addOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    if ( std::find( m_overlayFrames.begin(), m_overlayFrames.end(), overlayFrame ) == m_overlayFrames.end() )
    {
        overlayFrame->setParent( this->canvas() );
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
    QwtPlot::updateLayout();
    updateOverlayFrameLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotWidget::eventFilter( QObject* watched, QEvent* event )
{
    QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>( event );
    if ( wheelEvent && watched == canvas() )
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

        if ( watched == this && !this->canvas()->geometry().contains( mouseEvent->pos() ) )
        {
            if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease && ( mouseEvent->button() == Qt::LeftButton ) &&
                 !m_clickPosition.isNull() )
            {
                QWidget* childClicked = this->childAt( m_clickPosition );
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
        else if ( watched == canvas() )
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
    QwtPlot::hideEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::showEvent( QShowEvent* event )
{
    m_plotStyleSheet = createPlotStyleSheet();
    m_plotStyleSheet.applyToWidget( this );

    m_canvasStyleSheet = createCanvasStyleSheet();
    m_canvasStyleSheet.applyToWidget( canvas() );

    QwtPlot::showEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resizeEvent( QResizeEvent* event )
{
    QwtPlot::resizeEvent( event );
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
    QwtText plotTitle        = this->title();
    if ( plotTitleToApply != plotTitle.text() )
    {
        plotTitle.setText( plotTitleToApply );
        setTitle( plotTitle );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::applyAxisTitleToQwt( QwtPlot::Axis axis )
{
    QString titleToApply = m_axisTitlesEnabled[axis] ? m_axisTitles[axis] : QString( "" );
    QwtText axisTitle    = this->axisTitle( axis );
    if ( titleToApply != axisTitle.text() )
    {
        axisTitle.setText( titleToApply );

        setAxisTitle( axis, axisTitle );
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
    static_cast<QwtPlotCanvas*>( this->canvas() )->setPaintAttribute( QwtPlotCanvas::BackingStore, false );

    QPoint plotTopLeftInWindowCoords = targetRect.topLeft();

    QRectF canvasRect = this->plotLayout()->canvasRect();
    QPoint canvasTopLeftInPlotCoords( canvasRect.topLeft().x() * scaling, canvasRect.topLeft().y() * scaling );
    QPoint canvasBottomRightInPlotCoords( canvasRect.bottomRight().x(), canvasRect.bottomRight().y() );

    QPoint canvasTopLeftInWindowCoords     = canvasTopLeftInPlotCoords + plotTopLeftInWindowCoords;
    QPoint canvasBottomRightInWindowCoords = canvasBottomRightInPlotCoords + plotTopLeftInWindowCoords;

    QwtPlotRenderer renderer( this );
    renderer.render( this, painter, targetRect );
    static_cast<QwtPlotCanvas*>( this->canvas() )->setPaintAttribute( QwtPlotCanvas::BackingStore, true );

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
        if ( scale == this->axisWidget( i ) )
        {
            axisId = i;
        }
    }
    emit axisSelected( axisId, toggleItemInSelection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::recalculateAxisExtents( QwtPlot::Axis axis )
{
    if ( axis == QwtPlot::yLeft || axis == QwtPlot::yRight )
    {
        int extent = axisExtent( axis );
        axisScaleDraw( axis )->setMinimumExtent( extent );
        setMinimumWidth( defaultMinimumWidth() + extent );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet RiuQwtPlotWidget::createPlotStyleSheet() const
{
    QColor backgroundColor = QColor( "white" );
    QColor highlightColor  = QApplication::palette().highlight().color();

    caf::UiStyleSheet styleSheet;
    styleSheet.set( "background-color", backgroundColor.name() );
    styleSheet.set( "border", "1 px solid transparent" );

    styleSheet.property( "selected" ).set( "border", QString( "1px solid %1" ).arg( highlightColor.name() ) );

    return styleSheet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet RiuQwtPlotWidget::createCanvasStyleSheet() const
{
    caf::UiStyleSheet styleSheet;
    styleSheet.set( "background-color", "#FAFAFA" );
    styleSheet.set( "border", "1px solid LightGray" );
    return styleSheet;
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

    QSize canvasSize = this->canvas()->size();
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
                if ( ypos + frame->height() + spacing > this->canvas()->height() && widthOfCurrentColumn > 0 )
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
                QRect  canvasRect     = canvas()->rect();
                QPoint canvasTopRight = canvasRect.topRight();
                frameRect.moveTopRight( QPoint( canvasTopRight.x() - spacing, canvasTopRight.y() + spacing ) );
                frame->move( frameRect.topLeft() );
            }
            frame->show();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::selectClosestPlotItem( const QPoint& pos, bool toggleItemInSelection /*= false*/ )
{
    QwtPlotItem* closestItem       = nullptr;
    double       distMin           = DBL_MAX;
    int          closestCurvePoint = -1;

    const QwtPlotItemList& itmList = itemList();
    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); it++ )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve* candidateCurve = static_cast<QwtPlotCurve*>( *it );
            double        dist           = DBL_MAX;
            int           curvePoint     = candidateCurve->closestPoint( pos, &dist );
            if ( dist < distMin )
            {
                closestItem       = candidateCurve;
                distMin           = dist;
                closestCurvePoint = curvePoint;
            }
        }
        else if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotShape )
        {
            QwtPlotShapeItem* shapeItem = static_cast<QwtPlotShapeItem*>( *it );
            QPointF           scalePos( invTransform( xBottom, pos.x() ), invTransform( yLeft, pos.y() ) );
            if ( shapeItem->shape().boundingRect().contains( scalePos ) )
            {
                closestItem = *it;
                distMin     = 0.0;
            }
        }
        else if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotBarChart )
        {
            QwtPlotBarChart* barChart = static_cast<QwtPlotBarChart*>( *it );
            QPointF          scalePos( invTransform( xBottom, pos.x() ), invTransform( yLeft, pos.y() ) );

            bool horizontal = barChart->orientation() == Qt::Horizontal;
            for ( size_t i = 0; i < barChart->dataSize(); ++i )
            {
                QPointF samplePoint = barChart->sample( i );
                double  dist        = horizontal ? std::abs( samplePoint.x() - scalePos.y() )
                                         : std::abs( samplePoint.x() - scalePos.x() );
                if ( dist < distMin )
                {
                    closestItem       = *it;
                    closestCurvePoint = (int)i;
                    distMin           = dist;
                }
            }
        }
    }

    RiuPlotMainWindowTools::showPlotMainWindow();
    resetPlotItemHighlighting();
    if ( closestItem && distMin < 20 )
    {
        // TODO: highlight all selected curves
        highlightPlotItem( closestItem );
        emit plotItemSelected( closestItem, toggleItemInSelection, distMin < 10 ? closestCurvePoint : -1 );

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
    QwtPlot::replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::highlightPlotItem( const QwtPlotItem* closestItem )
{
    // NB! Create a copy of the item list before the loop to avoid invalidated iterators when iterating the list
    // plotCurve->setZ() causes the ordering of items in the list to change
    auto plotItemList = this->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        QwtPlotCurve*     plotCurve     = dynamic_cast<QwtPlotCurve*>( plotItem );
        QwtPlotShapeItem* plotShapeItem = dynamic_cast<QwtPlotShapeItem*>( plotItem );
        if ( plotCurve )
        {
            QPen   existingPen = plotCurve->pen();
            QColor bgColor     = this->canvasBackground().color();

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
    auto plotItemList = this->itemList();
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
            pen.setColor( QColor( Qt::black ) );
            pen.setWidth( 1 );
            plotShapeItem->setPen( pen );
            plotShapeItem->setZ( plotShapeItem->z() - 100.0 );
        }
    }
    m_originalCurveColors.clear();
    m_originalZValues.clear();
}
