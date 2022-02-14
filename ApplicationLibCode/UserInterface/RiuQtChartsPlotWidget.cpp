////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiuQtChartsPlotWidget.h"

#include "RiaDefines.h"
#include "RiaFontCache.h"
#include "RiaGuiApplication.h"
#include "RiaPlotDefines.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RimPlot.h"

#include "RiuDraggableOverlayFrame.h"
#include "RiuGuiTheme.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuPlotWidget.h"
#include "RiuQtChartView.h"
#include "RiuQtChartsPlotCurve.h"

#include "cafAssert.h"

#include "cvfTrace.h"

#include <QDateTimeAxis>
#include <QGraphicsLayout>
#include <QLogValueAxis>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QtGlobal>

#include <limits>

using namespace QtCharts;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotWidget::RiuQtChartsPlotWidget( RimPlot* plotDefinition, QWidget* parent )
    : RiuPlotWidget( plotDefinition, parent )
{
    CAF_ASSERT( m_plotDefinition );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );

    QtCharts::QChart* chart = new QtCharts::QChart();
    chart->layout()->setContentsMargins( 0, 0, 0, 0 );
    chart->setBackgroundRoundness( 0 );
    chart->setAcceptDrops( true );
    chart->installEventFilter( this );

    m_viewer = new RiuQtChartView( nullptr, parent );
    m_viewer->setChart( chart );
    m_viewer->setRenderHint( QPainter::Antialiasing );

    layout->addWidget( m_viewer );

    addAxis( RiuPlotAxis::defaultBottom(), true, true );
    addAxis( RiuPlotAxis::defaultLeft(), true, true );
    addAxis( RiuPlotAxis::defaultRight(), true, true );
    addAxis( RiuPlotAxis::defaultTop(), false, false );

    m_viewer->setRubberBand( QChartView::RectangleRubberBand );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotWidget::~RiuQtChartsPlotWidget()
{
    if ( m_plotDefinition )
    {
        m_plotDefinition->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::axisRangeChanged()
{
    if ( qtChart()->isZoomed() ) emit plotZoomed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotWidget::axisTitleFontSize( RiuPlotAxis axis ) const
{
    if ( axisEnabled( axis ) )
    {
        return plotAxis( axis )->titleFont().pointSize();
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotWidget::axisValueFontSize( RiuPlotAxis axis ) const
{
    if ( axisEnabled( axis ) )
    {
        return plotAxis( axis )->labelsFont().pointSize();
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisFontsAndAlignment( RiuPlotAxis axis,
                                                      int         titleFontSize,
                                                      int         valueFontSize,
                                                      bool        titleBold,
                                                      int         alignment )
{
    int titleFontPixelSize = caf::FontTools::pointSizeToPixelSize( titleFontSize );
    int valueFontPixelSize = caf::FontTools::pointSizeToPixelSize( valueFontSize );

    // Axis number font
    QFont axisFont = plotAxis( axis )->labelsFont();
    axisFont.setPixelSize( valueFontPixelSize );
    axisFont.setBold( false );
    plotAxis( axis )->setLabelsFont( axisFont );

    // Axis title font
    QFont axisTitleFont = plotAxis( axis )->labelsFont();
    axisTitleFont.setPixelSize( titleFontPixelSize );
    axisTitleFont.setBold( titleBold );
    plotAxis( axis )->setTitleFont( axisTitleFont );

    applyAxisTitleToPlot( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxesFontsAndAlignment( int titleFontSize, int valueFontSize, bool titleBold, int alignment )
{
    for ( auto axisTitlePair : m_axisTitles )
    {
        setAxisFontsAndAlignment( axisTitlePair.first, titleFontSize, valueFontSize, titleBold, alignment );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisTitleText( RiuPlotAxis axis, const QString& title )
{
    m_axisTitles[axis] = title;
    applyAxisTitleToPlot( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisTitleEnabled( RiuPlotAxis axis, bool enable )
{
    m_axisTitlesEnabled[axis] = enable;
    applyAxisTitleToPlot( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisFormat( RiuPlotAxis axis, const QString& format )
{
    auto ax = plotAxis( axis );

    auto valueAxis = dynamic_cast<QValueAxis*>( ax );
    if ( valueAxis ) valueAxis->setLabelFormat( format );

    auto logAxis = dynamic_cast<QLogValueAxis*>( ax );
    if ( logAxis ) logAxis->setLabelFormat( format );

    auto dateAxis = dynamic_cast<QDateTimeAxis*>( ax );
    if ( dateAxis ) dateAxis->setFormat( format );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle = plotTitle;
    applyPlotTitleToPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiuQtChartsPlotWidget::plotTitle() const
{
    return m_plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setPlotTitleEnabled( bool enabled )
{
    m_plotTitleEnabled = enabled;
    applyPlotTitleToPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::plotTitleEnabled() const
{
    return m_plotTitleEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setPlotTitleFontSize( int titleFontSize )
{
    QFont font = qtChart()->titleFont();
    font.setPixelSize( caf::FontTools::pointSizeToPixelSize( titleFontSize ) );
    qtChart()->setTitleFont( font );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setLegendFontSize( int fontSize )
{
    if ( qtChart()->legend() )
    {
        QFont font = qtChart()->legend()->font();
        font.setPixelSize( caf::FontTools::pointSizeToPixelSize( fontSize ) );
        qtChart()->legend()->setFont( font );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setInternalLegendVisible( bool visible )
{
    if ( visible )
    {
        insertLegend( RiuPlotWidget::Legend::BOTTOM );
    }
    else
    {
        clearLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::insertLegend( RiuPlotWidget::Legend legendPosition )
{
    auto mapLegendPosition = []( RiuPlotWidget::Legend pos ) {
        if ( pos == RiuPlotWidget::Legend::BOTTOM )
            return Qt::AlignBottom;
        else if ( pos == RiuPlotWidget::Legend::TOP )
            return Qt::AlignTop;
        else if ( pos == RiuPlotWidget::Legend::LEFT )
            return Qt::AlignLeft;

        return Qt::AlignRight;
    };

    QLegend* legend = qtChart()->legend();
    legend->setAlignment( mapLegendPosition( legendPosition ) );
    if ( !legend->isAttachedToChart() )
    {
        legend->attachToChart();
    }

    replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::clearLegend()
{
    QLegend* legend = qtChart()->legend();
    legend->detachFromChart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQtChartsPlotWidget::axisRange( RiuPlotAxis axis ) const
{
    auto ax = plotAxis( axis );

    auto valueAxis = dynamic_cast<QValueAxis*>( ax );
    if ( valueAxis ) return std::make_pair( valueAxis->min(), valueAxis->max() );

    auto logAxis = dynamic_cast<QLogValueAxis*>( ax );
    if ( logAxis ) return std::make_pair( logAxis->min(), logAxis->max() );

    auto dateAxis = dynamic_cast<QDateTimeAxis*>( ax );
    if ( dateAxis ) return std::make_pair( dateAxis->min().toMSecsSinceEpoch(), dateAxis->max().toMSecsSinceEpoch() );

    return std::make_pair( 0.0, 1.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisRange( RiuPlotAxis axis, double min, double max )
{
    // Note: Especially the Y-axis may be inverted
    if ( plotAxis( axis )->isReverse() )
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
void RiuQtChartsPlotWidget::setAxisInverted( RiuPlotAxis axis, bool isInverted )
{
    auto ax = plotAxis( axis );
    ax->setReverse( isInverted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisLabelsAndTicksEnabled( RiuPlotAxis axis, bool enableLabels, bool enableTicks )
{
    plotAxis( axis )->setLabelsVisible( enableLabels );
    plotAxis( axis )->setGridLineVisible( enableTicks );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::enableGridLines( RiuPlotAxis axis, bool majorGridLines, bool minorGridLines )
{
    plotAxis( axis )->setGridLineVisible( majorGridLines );
    plotAxis( axis )->setMinorGridLineVisible( minorGridLines );

    QPen gridLinePen( Qt::lightGray, 1.0, Qt::SolidLine );
    plotAxis( axis )->setGridLinePen( gridLinePen );

    QPen minorGridLinePen( Qt::lightGray, 1.0, Qt::DashLine );
    plotAxis( axis )->setMinorGridLinePen( minorGridLinePen );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setMajorAndMinorTickIntervals( RiuPlotAxis axis,
                                                           double      majorTickInterval,
                                                           double      minorTickInterval,
                                                           double      minValue,
                                                           double      maxValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setMajorAndMinorTickIntervalsAndRange( RiuPlotAxis axis,
                                                                   double      majorTickInterval,
                                                                   double      minorTickInterval,
                                                                   double      minTickValue,
                                                                   double      maxTickValue,
                                                                   double      rangeMin,
                                                                   double      rangeMax )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAutoTickIntervalCounts( RiuPlotAxis axis,
                                                       int         maxMajorTickIntervalCount,
                                                       int         maxMinorTickIntervalCount )
{
    setAxisMaxMajor( axis, maxMajorTickIntervalCount );
    setAxisMaxMinor( axis, maxMinorTickIntervalCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQtChartsPlotWidget::majorTickInterval( RiuPlotAxis axis ) const
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 12, 0 )
    // QValueAxis::tickInterval was introduced in 5.12
    QAbstractAxis* ax        = plotAxis( axis );
    QValueAxis*    valueAxis = dynamic_cast<QValueAxis*>( ax );
    if ( valueAxis ) return valueAxis->tickInterval();
#endif
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQtChartsPlotWidget::minorTickInterval( RiuPlotAxis axis ) const
{
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotWidget::axisExtent( RiuPlotAxis axis ) const
{
    CAF_ASSERT( false && "Not implemented" );
    return 100;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPoint RiuQtChartsPlotWidget::dragStartPosition() const
{
    return m_clickPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::scheduleReplot()
{
    RiaPlotWindowRedrawScheduler::instance()->schedulePlotWidgetReplot( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::updateLayout()
{
    updateOverlayFrameLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::updateLegend()
{
    qtChart()->legend()->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent( event );
    updateOverlayFrameLayout();
    event->accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::keyPressEvent( QKeyEvent* event )
{
    switch ( event->key() )
    {
        case Qt::Key_Plus:
            qtChart()->zoomIn();
            break;
        case Qt::Key_Minus:
            qtChart()->zoomOut();
            break;
        default:
            QWidget::keyPressEvent( event );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::applyPlotTitleToPlot()
{
    QString plotTitleToApply = m_plotTitleEnabled ? m_plotTitle : QString( "" );
    m_viewer->chart()->setTitle( plotTitleToApply );
    m_viewer->chart()->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::applyAxisTitleToPlot( RiuPlotAxis axis )
{
    QString titleToApply = m_axisTitlesEnabled[axis] ? m_axisTitles[axis] : QString( "" );
    plotAxis( axis )->setTitleText( titleToApply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQtChartsPlotWidget::sizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuQtChartsPlotWidget::minimumSizeHint() const
{
    return QSize( 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::isZoomerActive() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::endZoomOperations()
{
}

//--------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::renderTo( QPainter* painter, const QRect& targetRect, double scaling )
{
    // TODO: handle scaling...
    painter->setRenderHint( QPainter::Antialiasing );
    m_viewer->render( painter, targetRect );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::renderTo( QPaintDevice* paintDevice, const QRect& targetRect )
{
    int      resolution = paintDevice->logicalDpiX();
    double   scaling    = resolution / static_cast<double>( RiaGuiApplication::applicationResolution() );
    QPainter painter( paintDevice );
    renderTo( &painter, targetRect, scaling );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotWidget::overlayMargins() const
{
    return m_overlayMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RiuQtChartsPlotWidget::getParentForOverlay() const
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuQtChartsPlotWidget::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotWidget::defaultMinimumWidth()
{
    return 80;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::replot()
{
    qtChart()->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::enableAxis( RiuPlotAxis axis, bool isEnabled )
{
    m_axesEnabled[axis] = isEnabled;
    plotAxis( axis )->setVisible( isEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::axisEnabled( RiuPlotAxis axis ) const
{
    auto it = m_axesEnabled.find( axis );
    if ( it != m_axesEnabled.end() )
        return it->second;
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisMaxMinor( RiuPlotAxis axis, int maxMinor )
{
    QAbstractAxis* ax        = plotAxis( axis );
    QValueAxis*    valueAxis = dynamic_cast<QValueAxis*>( ax );
    if ( valueAxis )
    {
        valueAxis->setMinorTickCount( maxMinor );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisMaxMajor( RiuPlotAxis axis, int maxMajor )
{
    QAbstractAxis* ax        = plotAxis( axis );
    QValueAxis*    valueAxis = dynamic_cast<QValueAxis*>( ax );
    if ( valueAxis )
    {
        valueAxis->setTickCount( maxMajor );
    }
    else
    {
        QDateTimeAxis* dateAxis = dynamic_cast<QDateTimeAxis*>( ax );
        if ( dateAxis ) dateAxis->setTickCount( maxMajor );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisAutoScale( RiuPlotAxis axis, bool autoScale )
{
    m_axesAutoScale[axis] = autoScale;

    if ( autoScale )
    {
        rescaleAxis( axis );
        QAbstractAxis* ax        = plotAxis( axis );
        QValueAxis*    valueAxis = dynamic_cast<QValueAxis*>( ax );
        if ( valueAxis )
        {
            valueAxis->applyNiceNumbers();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisScale( RiuPlotAxis axis, double min, double max )
{
    plotAxis( axis )->setRange( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotWidget::AxisScaleType RiuQtChartsPlotWidget::axisScaleType( RiuPlotAxis axis ) const
{
    if ( plotAxis( axis )->type() == QAbstractAxis::AxisTypeLogValue ) return AxisScaleType::LOGARITHMIC;
    if ( plotAxis( axis )->type() == QAbstractAxis::AxisTypeDateTime ) return AxisScaleType::DATE;
    return AxisScaleType::LINEAR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisScaleType( RiuPlotAxis axis, RiuQtChartsPlotWidget::AxisScaleType axisScaleType )
{
    QAbstractAxis* removeaxis = plotAxis( axis );
    QAbstractAxis* insertaxis = nullptr;

    if ( axisScaleType == AxisScaleType::LOGARITHMIC )
    {
        insertaxis = new QLogValueAxis;
    }
    else if ( axisScaleType == AxisScaleType::DATE )
    {
        insertaxis = new QDateTimeAxis;
    }
    else if ( axisScaleType == AxisScaleType::LINEAR )
    {
        insertaxis = new QValueAxis;
    }

    QChart* chart = qtChart();
    if ( chart->axes().contains( removeaxis ) ) chart->removeAxis( removeaxis );
    chart->addAxis( insertaxis, mapPlotAxisToQtAlignment( axis.axis() ) );

    m_axes[axis] = insertaxis;
    for ( auto serie : chart->series() )
    {
        if ( serie->attachedAxes().contains( removeaxis ) ) serie->detachAxis( removeaxis );
        serie->attachAxis( insertaxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::updateAxes()
{
    m_viewer->chart()->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurve* RiuQtChartsPlotWidget::createPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title, const QColor& color )
{
    return new RiuQtChartsPlotCurve( ownerRimCurve, title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QtCharts::QChart* RiuQtChartsPlotWidget::qtChart()
{
    return m_viewer->chart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::attach( RiuPlotCurve*              plotCurve,
                                    QtCharts::QAbstractSeries* lineSeries,
                                    QtCharts::QAbstractSeries* scatterSeries,
                                    RiuPlotAxis                xAxis,
                                    RiuPlotAxis                yAxis )
{
    auto addToChart = [this]( std::map<const RiuPlotCurve*, QtCharts::QAbstractSeries*>& curveSeriesMap,
                              auto                                                       plotCurve,
                              auto                                                       series,
                              auto                                                       xAxis,
                              auto                                                       yAxis ) {
        if ( !series->chart() )
        {
            curveSeriesMap[plotCurve] = series;
            qtChart()->addSeries( series );
            setXAxis( xAxis, series );
            setXAxis( yAxis, series );
            rescaleAxis( xAxis );
            rescaleAxis( yAxis );
        }
    };

    addToChart( m_lineSeriesMap, plotCurve, lineSeries, xAxis, yAxis );
    addToChart( m_scatterSeriesMap, plotCurve, scatterSeries, xAxis, yAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QtCharts::QAbstractSeries* RiuQtChartsPlotWidget::getLineSeries( const RiuPlotCurve* plotCurve ) const
{
    auto series = m_lineSeriesMap.find( plotCurve );
    if ( series != m_lineSeriesMap.end() )
        return series->second;
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QtCharts::QAbstractSeries* RiuQtChartsPlotWidget::getScatterSeries( const RiuPlotCurve* plotCurve ) const
{
    auto series = m_scatterSeriesMap.find( plotCurve );
    if ( series != m_scatterSeriesMap.end() )
        return series->second;
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::detachItems( RiuPlotWidget::PlotItemType plotItemType )
{
    cvf::Trace::show( "RiuQtChartsPlotWidget::detachItems" );

    if ( plotItemType == RiuPlotWidget::PlotItemType::CURVE )
    {
        m_lineSeriesMap.clear();
        m_scatterSeriesMap.clear();
        qtChart()->removeAllSeries();
    }
    else
    {
        cvf::Trace::show( "Detach items not implemented for this type." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setXAxis( RiuPlotAxis axis, QtCharts::QAbstractSeries* series )
{
    setAxis( axis, series );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setYAxis( RiuPlotAxis axis, QtCharts::QAbstractSeries* series )
{
    setAxis( axis, series );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxis( RiuPlotAxis axis, QtCharts::QAbstractSeries* series )
{
    // Make sure the axis we are about to set exists.
    if ( m_axes.find( axis ) == m_axes.end() )
    {
        addAxis( axis, true, true );
    }

    if ( qtChart()->series().contains( series ) && !series->attachedAxes().contains( plotAxis( axis ) ) )
    {
        auto newAxis = plotAxis( axis );

        // Detach any other axis for the same orientation
        for ( auto ax : series->attachedAxes() )
        {
            if ( ax->orientation() == orientation( axis.axis() ) )
            {
                series->detachAxis( ax );
            }
        }

        series->attachAxis( newAxis );

        if ( qobject_cast<QValueAxis*>( newAxis ) || qobject_cast<QLogValueAxis*>( newAxis ) )
        {
            connect( newAxis, SIGNAL( rangeChanged( double, double ) ), this, SLOT( axisRangeChanged() ), Qt::UniqueConnection );
        }
        else if ( qobject_cast<QDateTimeAxis*>( newAxis ) )
        {
            connect( newAxis,
                     SIGNAL( rangeChanged( QDateTime, QDateTime ) ),
                     this,
                     SLOT( axisRangeChanged() ),
                     Qt::UniqueConnection );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::addAxis( RiuPlotAxis plotAxis, bool isEnabled, bool isAutoScale )
{
    QValueAxis* axis = new QValueAxis();
    qtChart()->addAxis( axis, mapPlotAxisToQtAlignment( plotAxis.axis() ) );
    m_axes[plotAxis] = axis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RiuQtChartsPlotWidget::createNextPlotAxis( RiaDefines::PlotAxis axis )
{
    int minIdx = -1;
    for ( auto a : m_axes )
    {
        if ( a.first.axis() == axis )
        {
            minIdx = std::max( a.first.index(), minIdx );
        }
    }

    RiuPlotAxis plotAxis( axis, minIdx + 1 );

    addAxis( plotAxis, true, true );
    return plotAxis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::isMultiAxisSupported() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::rescaleAxis( RiuPlotAxis axis )
{
    if ( !m_axesAutoScale[axis] ) return;

    QAbstractAxis*  pAxis = plotAxis( axis );
    Qt::Orientation orr   = orientation( axis.axis() );

    double min = std::numeric_limits<double>::max();
    double max = -std::numeric_limits<double>::max();
    for ( auto series : qtChart()->series() )
    {
        auto attachedAxes = series->attachedAxes();
        if ( attachedAxes.contains( pAxis ) )
        {
            QVector<QPointF> points;
            for ( auto attachedAxis : attachedAxes )
            {
                QValueAxis* valueAxis = dynamic_cast<QValueAxis*>( attachedAxis );
                if ( valueAxis && valueAxis->orientation() == orr && dynamic_cast<QLineSeries*>( series ) )
                {
                    points = dynamic_cast<QLineSeries*>( series )->pointsVector();
                }

                QDateTimeAxis* dateTimeAxis = dynamic_cast<QDateTimeAxis*>( attachedAxis );
                if ( dateTimeAxis && dateTimeAxis->orientation() == orr && dynamic_cast<QLineSeries*>( series ) )
                {
                    points = dynamic_cast<QLineSeries*>( series )->pointsVector();
                }

                for ( auto p : points )
                {
                    if ( orr == Qt::Orientation::Horizontal )
                    {
                        min = std::min( min, p.x() );
                        max = std::max( max, p.x() );
                    }
                    else
                    {
                        min = std::min( min, p.y() );
                        max = std::max( max, p.y() );
                    }
                }
            }
        }
    }

    if ( axisScaleType( axis ) == RiuPlotWidget::AxisScaleType::DATE )
    {
        pAxis->setRange( QDateTime::fromMSecsSinceEpoch( min ), QDateTime::fromMSecsSinceEpoch( max ) );
    }
    else
    {
        pAxis->setRange( min, max );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAbstractAxis* RiuQtChartsPlotWidget::plotAxis( RiuPlotAxis axis ) const
{
    const auto ax = m_axes.find( axis );
    if ( ax != m_axes.end() )
    {
        return ax->second;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::Orientation RiuQtChartsPlotWidget::orientation( RiaDefines::PlotAxis axis ) const
{
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM || axis == RiaDefines::PlotAxis::PLOT_AXIS_TOP )
        return Qt::Orientation::Horizontal;

    return Qt::Orientation::Vertical;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::dragEnterEvent( QDragEnterEvent* event )
{
    RiuPlotWidget::handleDragDropEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::dropEvent( QDropEvent* event )
{
    RiuPlotWidget::handleDragDropEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::wheelEvent( QWheelEvent* event )
{
    float factor = event->angleDelta().y() > 0 ? 0.9 : 1.1;

    QRectF  plotAreaRect = m_viewer->chart()->plotArea();
    QPointF centerPoint  = plotAreaRect.center();

    // Adjust the size of the plot area
    plotAreaRect.setWidth( plotAreaRect.width() * factor );
    plotAreaRect.setHeight( plotAreaRect.height() * factor );

    // Find new center which keeps the mouse location in the same place in the plot
    QPointF newCenterPoint( ( 2 * centerPoint - event->pos() ) - ( centerPoint - event->pos() ) / factor );
    plotAreaRect.moveCenter( newCenterPoint );

    // Zoom in on the adjusted plot area
    m_viewer->chart()->zoomIn( plotAreaRect );

    event->accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::eventFilter( QObject* watched, QEvent* event )
{
    if ( RiuPlotWidget::handleDragDropEvent( event ) ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QColor& RiuQtChartsPlotWidget::backgroundColor() const
{
    return m_viewer->chart()->backgroundBrush().color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RiuPlotCurve*, int> RiuQtChartsPlotWidget::findClosestCurve( const QPoint& pos, double& distanceToClick ) const
{
    return std::make_pair( nullptr, -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::Alignment RiuQtChartsPlotWidget::mapPlotAxisToQtAlignment( RiaDefines::PlotAxis axis )
{
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM ) return Qt::AlignBottom;
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_TOP ) return Qt::AlignTop;
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ) return Qt::AlignLeft;
    return Qt::AlignRight;
}
