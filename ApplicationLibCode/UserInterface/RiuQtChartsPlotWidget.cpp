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
#include "RiuPlotWidget.h"
#include "RiuQtChartsPlotCurve.h"

#include "cafAssert.h"

#include "cvfTrace.h"

#include <QVBoxLayout>

#include <QValueAxis>

#include <limits>
#include <qnamespace.h>

using namespace QtCharts;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotWidget::RiuQtChartsPlotWidget( RimPlot* plotDefinition, QWidget* parent )
    : RiuPlotWidget( plotDefinition, parent )
{
    CAF_ASSERT( m_plotDefinition );

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    QtCharts::QChart* chart = new QtCharts::QChart();

    m_viewer = new QtCharts::QChartView( chart, parent ); // RiuQtChartView( this, parent );
    m_viewer->setRenderHint( QPainter::Antialiasing );

    layout->addWidget( m_viewer );

    QValueAxis* axisBottom = new QValueAxis();
    chart->addAxis( axisBottom, Qt::AlignBottom );
    m_axes[RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM] = axisBottom;

    QValueAxis* axisTop = new QValueAxis();
    chart->addAxis( axisTop, Qt::AlignTop );
    m_axes[RiaDefines::PlotAxis::PLOT_AXIS_TOP] = axisTop;

    QValueAxis* axisLeft = new QValueAxis();
    chart->addAxis( axisLeft, Qt::AlignLeft );
    m_axes[RiaDefines::PlotAxis::PLOT_AXIS_LEFT] = axisLeft;

    QValueAxis* axisRight = new QValueAxis();
    chart->addAxis( axisRight, Qt::AlignRight );
    m_axes[RiaDefines::PlotAxis::PLOT_AXIS_RIGHT] = axisRight;

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
int RiuQtChartsPlotWidget::axisTitleFontSize( RiaDefines::PlotAxis axis ) const
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
int RiuQtChartsPlotWidget::axisValueFontSize( RiaDefines::PlotAxis axis ) const
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
void RiuQtChartsPlotWidget::setAxisFontsAndAlignment( RiaDefines::PlotAxis axis,
                                                      int                  titleFontSize,
                                                      int                  valueFontSize,
                                                      bool                 titleBold,
                                                      int                  alignment )
{
    // int titleFontPixelSize = caf::FontTools::pointSizeToPixelSize( titleFontSize );
    // int valueFontPixelSize = caf::FontTools::pointSizeToPixelSize( valueFontSize );

    // // Axis number font

    // int   qwtAxis  = RiuQwtPlotTools::toQwtPlotAxis( axis );
    // QFont axisFont = m_plot->axisFont( qwtAxis );
    // axisFont.setPixelSize( valueFontPixelSize );
    // axisFont.setBold( false );
    // m_plot->setAxisFont( qwtAxis, axisFont );

    // // Axis title font
    // QwtText axisTitle     = m_plot->axisTitle( qwtAxis );
    // QFont   axisTitleFont = axisTitle.font();
    // axisTitleFont.setPixelSize( titleFontPixelSize );
    // axisTitleFont.setBold( titleBold );
    // axisTitle.setFont( axisTitleFont );
    // axisTitle.setRenderFlags( alignment | Qt::TextWordWrap );

    // m_plot->setAxisTitle( qwtAxis, axisTitle );
    // applyAxisTitleToQwt( axis );
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
void RiuQtChartsPlotWidget::setAxisTitleText( RiaDefines::PlotAxis axis, const QString& title )
{
    m_axisTitles[axis] = title;
    applyAxisTitleToPlot( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisTitleEnabled( RiaDefines::PlotAxis axis, bool enable )
{
    m_axisTitlesEnabled[axis] = enable;
    applyAxisTitleToPlot( axis );
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
    // auto  title = m_plot->title();
    // QFont font  = title.font();
    // font.setPixelSize( caf::FontTools::pointSizeToPixelSize( titleFontSize ) );
    // title.setFont( font );
    // m_plot->setTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setLegendFontSize( int fontSize )
{
    // if ( m_plot->legend() )
    // {
    //     QFont font = m_plot->legend()->font();
    //     font.setPixelSize( caf::FontTools::pointSizeToPixelSize( fontSize ) );
    //     m_plot->legend()->setFont( font );
    //     // Set font size for all existing labels
    //     QList<QwtLegendLabel*> labels = m_plot->legend()->findChildren<QwtLegendLabel*>();
    //     for ( QwtLegendLabel* label : labels )
    //     {
    //         label->setFont( font );
    //     }
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setInternalLegendVisible( bool visible )
{
    // if ( visible )
    // {
    //     QwtLegend* legend = new QwtLegend( this );
    //     m_plot->insertLegend( legend, QwtPlot::BottomLegend );
    // }
    // else
    // {
    //     m_plot->insertLegend( nullptr );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::insertLegend( RiuPlotWidget::Legend legendPosition )
{
    CAF_ASSERT( legendPosition == RiuPlotWidget::Legend::BOTTOM );

    // N  QwtLegend* legend = new QwtLegend( this );
    //    m_plot->insertLegend( legend, QtCharts::BottomLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::clearLegend()
{
    //    m_plot->insertLegend( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuQtChartsPlotWidget::axisRange( RiaDefines::PlotAxis axis ) const
{
    double min = 0.0;
    double max = 1.0;

    auto ax = plotAxis( axis );
    if ( ax )
    {
        min = ax->min();
        max = ax->max();
    }

    return std::make_pair( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisRange( RiaDefines::PlotAxis axis, double min, double max )
{
    // Note: Especially the Y-axis may be inverted
    // if ( m_plot->axisScaleEngine( RiuQwtPlotTools::toQwtPlotAxis( axis ) )->testAttribute( QwtScaleEngine::Inverted )
    // )
    // {
    //     setAxisScale( axis, max, min );
    // }
    // else
    // {
    //     setAxisScale( axis, min, max );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisInverted( RiaDefines::PlotAxis axis, bool isInverted )
{
    auto ax = plotAxis( axis );
    ax->setReverse( isInverted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisLabelsAndTicksEnabled( RiaDefines::PlotAxis axis, bool enableLabels, bool enableTicks )
{
    // m_plot->axisScaleDraw( RiuQwtPlotTools::toQwtPlotAxis( axis ) )->enableComponent( QwtAbstractScaleDraw::Ticks,
    // enableTicks ); m_plot->axisScaleDraw( RiuQwtPlotTools::toQwtPlotAxis( axis ) )->enableComponent(
    // QwtAbstractScaleDraw::Labels, enableLabels ); recalculateAxisExtents( axis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::enableGridLines( RiaDefines::PlotAxis axis, bool majorGridLines, bool minorGridLines )
{
    // QwtPlotItemList plotItems = m_plot->itemList( QwtPlotItem::Rtti_PlotGrid );
    // QwtPlot::Axis   qwtAxis   = RiuQwtPlotTools::toQwtPlotAxis( axis );
    // for ( QwtPlotItem* plotItem : plotItems )
    // {
    //     QwtPlotGrid* grid = static_cast<QwtPlotGrid*>( plotItem );
    //     if ( qwtAxis == QwtPlot::xTop || qwtAxis == QwtPlot::xBottom )
    //     {
    //         grid->setXAxis( qwtAxis );
    //         grid->enableX( majorGridLines );
    //         grid->enableXMin( minorGridLines );
    //     }
    //     else
    //     {
    //         grid->setYAxis( qwtAxis );
    //         grid->enableY( majorGridLines );
    //         grid->enableYMin( minorGridLines );
    //     }
    //     grid->setMajorPen( Qt::lightGray, 1.0, Qt::SolidLine );
    //     grid->setMinorPen( Qt::lightGray, 1.0, Qt::DashLine );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setMajorAndMinorTickIntervals( RiaDefines::PlotAxis axis,
                                                           double               majorTickInterval,
                                                           double               minorTickInterval,
                                                           double               minValue,
                                                           double               maxValue )
{
    // QwtPlot::Axis            qwtAxis     = RiuQwtPlotTools::toQwtPlotAxis( axis );
    // RiuQwtLinearScaleEngine* scaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( m_plot->axisScaleEngine( qwtAxis )
    // ); if ( scaleEngine )
    // {
    //     QwtScaleDiv scaleDiv =
    //         scaleEngine->divideScaleWithExplicitIntervals( minValue, maxValue, majorTickInterval, minorTickInterval
    //         );

    //     m_plot->setAxisScaleDiv( qwtAxis, scaleDiv );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setMajorAndMinorTickIntervalsAndRange( RiaDefines::PlotAxis axis,
                                                                   double               majorTickInterval,
                                                                   double               minorTickInterval,
                                                                   double               minTickValue,
                                                                   double               maxTickValue,
                                                                   double               rangeMin,
                                                                   double               rangeMax )
{
    // QwtPlot::Axis            qwtAxis     = RiuQwtPlotTools::toQwtPlotAxis( axis );
    // RiuQwtLinearScaleEngine* scaleEngine = dynamic_cast<RiuQwtLinearScaleEngine*>( m_plot->axisScaleEngine( qwtAxis )
    // ); if ( scaleEngine )
    // {
    //     QwtScaleDiv scaleDiv = scaleEngine->divideScaleWithExplicitIntervalsAndRange( minTickValue,
    //                                                                                   maxTickValue,
    //                                                                                   majorTickInterval,
    //                                                                                   minorTickInterval,
    //                                                                                   rangeMin,
    //                                                                                   rangeMax );

    //     m_plot->setAxisScaleDiv( qwtAxis, scaleDiv );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAutoTickIntervalCounts( RiaDefines::PlotAxis axis,
                                                       int                  maxMajorTickIntervalCount,
                                                       int                  maxMinorTickIntervalCount )
{
    // m_plot->setAxisMaxMajor( RiuQwtPlotTools::toQwtPlotAxis( axis ), maxMajorTickIntervalCount );
    // m_plot->setAxisMaxMinor( RiuQwtPlotTools::toQwtPlotAxis( axis ), maxMinorTickIntervalCount );
    // // Reapply axis limits to force Qwt to use the tick settings.
    // QwtInterval currentRange = m_plot->axisInterval( RiuQwtPlotTools::toQwtPlotAxis( axis ) );
    // setAxisScale( axis, currentRange.minValue(), currentRange.maxValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQtChartsPlotWidget::majorTickInterval( RiaDefines::PlotAxis axis ) const
{
    // QwtScaleDiv   scaleDiv   = m_plot->axisScaleDiv( RiuQwtPlotTools::toQwtPlotAxis( axis ) );
    // QList<double> majorTicks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    // if ( majorTicks.size() < 2 ) return 0.0;

    // return majorTicks.at( 1 ) - majorTicks.at( 0 );
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiuQtChartsPlotWidget::minorTickInterval( RiaDefines::PlotAxis axis ) const
{
    // QwtScaleDiv   scaleDiv   = m_plot->axisScaleDiv( QwtPlot::xTop );
    // QList<double> minorTicks = scaleDiv.ticks( QwtScaleDiv::MinorTick );
    // if ( minorTicks.size() < 2 ) return 0.0;

    // return minorTicks.at( 1 ) - minorTicks.at( 0 );
    return 0.5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQtChartsPlotWidget::axisExtent( RiaDefines::PlotAxis axis ) const
{
    // auto [rangeMin, rangeMax] = axisRange( axis );
    // if ( std::abs( rangeMax - rangeMin ) < 1.0e-14 ) return 0;

    // int           lineExtent = 0;
    // QwtPlot::Axis qwtAxis    = RiuQwtPlotTools::toQwtPlotAxis( axis );

    // if ( m_plot->axisScaleDraw( qwtAxis )->hasComponent( QwtAbstractScaleDraw::Ticks ) )
    // {
    //     lineExtent += m_plot->axisScaleDraw( qwtAxis )->maxTickLength();
    // }

    // if ( m_plot->axisScaleDraw( qwtAxis )->hasComponent( QwtAbstractScaleDraw::Labels ) )
    // {
    //     QFont tickLabelFont = m_plot->axisFont( RiuQwtPlotTools::toQwtPlotAxis( axis ) );
    //     // Make space for a fairly long value label
    //     QSize labelSize = QFontMetrics( tickLabelFont ).boundingRect( QString( "9.9e-9" ) ).size();

    //     if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
    //     {
    //         lineExtent = labelSize.width();
    //     }
    //     else
    //     {
    //         lineExtent = labelSize.height();
    //     }
    // }

    // if ( !m_plot->axisTitle( qwtAxis ).text().isEmpty() )
    // {
    //     auto it = m_axisTitlesEnabled.find( axis );
    //     if ( it != m_axisTitlesEnabled.end() && it->second )
    //     {
    //         QFont titleFont = m_plot->axisTitle( qwtAxis ).font();
    //         // Label is aligned vertically on vertical axes
    //         // So height is sufficient in both cases.
    //         lineExtent += QFontMetrics( titleFont ).height();
    //     }
    // }

    // return lineExtent;

    return 100;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::frameIsInFrontOfThis( const QRect& frameGeometry )
{
    // QRect ownGeometry = m_plot->canvas()->geometry();
    // ownGeometry.translate( m_plot->geometry().topLeft() );

    // if ( frameGeometry.bottom() < ownGeometry.center().y() )
    // {
    //     return true;
    // }
    // else if ( frameGeometry.left() < ownGeometry.left() && frameGeometry.top() < ownGeometry.center().y() )
    // {
    //     return true;
    // }
    // else
    // {
    //     QRect intersection = ownGeometry.intersected( frameGeometry );

    //     double ownArea          = double( ownGeometry.height() ) * double( ownGeometry.width() );
    //     double frameArea        = double( frameGeometry.height() ) * double( frameGeometry.width() );
    //     double intersectionArea = double( intersection.height() ) * double( intersection.width() );
    //     if ( intersectionArea > 0.8 * std::min( ownArea, frameArea ) )
    //     {
    //         return true;
    //     }
    // }

    return false;
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
/// Adds an overlay frame. The overlay frame becomes the responsibility of the plot widget
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::addOverlayFrame( RiuDraggableOverlayFrame* overlayFrame )
{
    // if ( std::find( m_overlayFrames.begin(), m_overlayFrames.end(), overlayFrame ) == m_overlayFrames.end() )
    // {
    //     overlayFrame->setParent( m_plot->canvas() );
    //     m_overlayFrames.push_back( overlayFrame );
    //     updateLayout();
    // }
}

//--------------------------------------------------------------------------------------------------
/// Remove the overlay widget. The frame becomes the responsibility of the caller
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::removeOverlayFrame( RiuDraggableOverlayFrame* overlayFrame ){
    // CAF_ASSERT( overlayFrame );

    // overlayFrame->hide();
    // overlayFrame->setParent( nullptr );
    // m_overlayFrames.removeOne( overlayFrame );
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::updateLayout()
{
    // m_plot->updateLayout();
    // updateOverlayFrameLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::updateLegend()
{
    // m_plot->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::eventFilter( QObject* watched, QEvent* event )
{
    // QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>( event );
    // if ( wheelEvent && watched == m_plot->canvas() )
    // {
    //     event->accept();

    //     emit onWheelEvent( wheelEvent );
    //     return true;
    // }

    // QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>( event );
    // if ( mouseEvent )
    // {
    //     if ( isZoomerActive() ) return false;

    //     bool toggleItemInSelection = ( mouseEvent->modifiers() & Qt::ControlModifier ) != 0;

    //     if ( mouseEvent->type() == QMouseEvent::MouseButtonPress && mouseEvent->button() == Qt::LeftButton )
    //     {
    //         m_clickPosition = mouseEvent->pos();
    //     }

    //     if ( watched == this && !m_plot->canvas()->geometry().contains( mouseEvent->pos() ) )
    //     {
    //         if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease && ( mouseEvent->button() ==
    //         Qt::LeftButton )
    //         &&
    //              !m_clickPosition.isNull() )
    //         {
    //             QWidget* childClicked = m_plot->childAt( m_clickPosition );
    //             if ( childClicked )
    //             {
    //                 QwtScaleWidget* scaleWidget = qobject_cast<QwtScaleWidget*>( childClicked );
    //                 if ( scaleWidget )
    //                 {
    //                     onAxisSelected( scaleWidget, toggleItemInSelection );
    //                     m_clickPosition = QPoint();
    //                     return true;
    //                 }
    //             }
    //             else
    //             {
    //                 endZoomOperations();
    //                 emit plotSelected( toggleItemInSelection );
    //                 m_clickPosition = QPoint();
    //                 return true;
    //             }
    //         }
    //     }
    //     else if ( watched == m_plot->canvas() )
    //     {
    //         if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease && mouseEvent->button() == Qt::LeftButton &&
    //              !m_clickPosition.isNull() )
    //         {
    //             endZoomOperations();
    //             selectClosestPlotItem( mouseEvent->pos(), toggleItemInSelection );
    //             m_clickPosition = QPoint();
    //             return true;
    //         }
    //     }
    // }
    // return false;
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::hideEvent( QHideEvent* event )
{
    //    resetPlotItemHighlighting();
    // TODO: remove?
    // m_plot->hideEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::showEvent( QShowEvent* event )
{
    // TODO: remove?
    // m_plot->showEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::resizeEvent( QResizeEvent* event )
{
    // TODO: remove???
    // QwtPlot::resizeEvent( event );
    // updateOverlayFrameLayout();
    // event->accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::keyPressEvent( QKeyEvent* event )
{
    // emit onKeyPressEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::applyPlotTitleToPlot()
{
    QString plotTitleToApply = m_plotTitleEnabled ? m_plotTitle : QString( "" );
    m_viewer->chart()->setTitle( plotTitleToApply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::applyAxisTitleToPlot( RiaDefines::PlotAxis axis )
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
RimViewWindow* RiuQtChartsPlotWidget::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RiuQtChartsPlotWidget::onAxisSelected( QwtScaleWidget* scale, bool toggleItemInSelection )
// {
//     // int axisId = -1;
//     // for ( int i = 0; i < QwtPlot::axisCnt; ++i )
//     // {
//     //     if ( scale == m_plot->axisWidget( i ) )
//     //     {
//     //         axisId = i;
//     //     }
//     // }
//     // emit axisSelected( axisId, toggleItemInSelection );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RiuQtChartsPlotWidget::recalculateAxisExtents( RiaDefines::PlotAxis axis )
// {
//     // QwtPlot::Axis qwtAxis = RiuQwtPlotTools::toQwtPlotAxis( axis );
//     // if ( qwtAxis == QwtPlot::yLeft || qwtAxis == QwtPlot::yRight )
//     // {
//     //     int extent = axisExtent( axis );
//     //     m_plot->axisScaleDraw( qwtAxis )->setMinimumExtent( extent );
//     //     setMinimumWidth( defaultMinimumWidth() + extent );
//     // }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::updateOverlayFrameLayout()
{
    // const int spacing = 5;

    // int xpos                 = spacing;
    // int ypos                 = spacing;
    // int widthOfCurrentColumn = 0;

    // QSize canvasSize = m_plot->canvas()->size();
    // QSize maxFrameSize( canvasSize.width() - 2 * m_overlayMargins, canvasSize.height() - 2 * m_overlayMargins );

    // for ( RiuDraggableOverlayFrame* frame : m_overlayFrames )
    // {
    //     if ( frame )
    //     {
    //         QSize minFrameSize     = frame->minimumSizeHint();
    //         QSize desiredFrameSize = frame->sizeHint();

    //         int width  = std::min( std::max( minFrameSize.width(), desiredFrameSize.width() ), maxFrameSize.width()
    //         ); int height = std::min( std::max( minFrameSize.height(), desiredFrameSize.height() ),
    //         maxFrameSize.height() );

    //         frame->resize( width, height );

    //         if ( frame->anchorCorner() == RiuDraggableOverlayFrame::AnchorCorner::TopLeft )
    //         {
    //             if ( ypos + frame->height() + spacing > m_plot->canvas()->height() && widthOfCurrentColumn > 0 )
    //             {
    //                 xpos += spacing + widthOfCurrentColumn;
    //                 ypos                 = spacing;
    //                 widthOfCurrentColumn = 0;
    //             }
    //             frame->move( xpos, ypos );
    //             ypos += frame->height() + spacing;
    //             widthOfCurrentColumn = std::max( widthOfCurrentColumn, frame->width() );
    //         }
    //         else if ( frame->anchorCorner() == RiuDraggableOverlayFrame::AnchorCorner::TopRight )
    //         {
    //             QRect  frameRect      = frame->frameGeometry();
    //             QRect  canvasRect     = m_plot->canvas()->rect();
    //             QPoint canvasTopRight = canvasRect.topRight();
    //             frameRect.moveTopRight( QPoint( canvasTopRight.x() - spacing, canvasTopRight.y() + spacing ) );
    //             frame->move( frameRect.topLeft() );
    //         }
    //         frame->show();
    //     }
    // }
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
    // m_plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::enableAxis( RiaDefines::PlotAxis axis, bool isEnabled )
{
    cvf::Trace::show( "RiuQtChartsPlotWidget::enableAxis" );

    //    m_plot->enableAxis( RiuQwtPlotTools::toQwtPlotAxis( axis ), isEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQtChartsPlotWidget::axisEnabled( RiaDefines::PlotAxis axis ) const
{
    cvf::Trace::show( "RiuQtChartsPlotWidget::axisEnabled" );

    return true;
    // return m_plot->axisEnabled( RiuQwtPlotTools::toQwtPlotAxis( axis ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisMaxMinor( RiaDefines::PlotAxis axis, int maxMinor )
{
    // m_plot->setAxisMaxMinor( RiuQwtPlotTools::toQwtPlotAxis( axis ), maxMinor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisMaxMajor( RiaDefines::PlotAxis axis, int maxMajor )
{
    // m_plot->setAxisMaxMajor( RiuQwtPlotTools::toQwtPlotAxis( axis ), maxMajor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::removeEventFilter()
{
    // m_plot->removeEventFilter( m_plot );
    // m_plot->canvas()->removeEventFilter( m_plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisAutoScale( RiaDefines::PlotAxis axis, bool autoScale )
{
    cvf::Trace::show( "RiuQtChartsPlotWidget::setAxisAutoScale" );

    if ( autoScale )
    {
        // m_plot->setAxisAutoScale( RiuQwtPlotTools::toQwtPlotAxis( axis ), autoScale );
        rescaleAxis( axis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisScale( RiaDefines::PlotAxis axis, double min, double max )
{
    cvf::Trace::show( "RiuQtChartsPlotWidget::setAxisScale" );
    // m_plot->setAxisScale( RiuQwtPlotTools::toQwtPlotAxis( axis ), min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQtChartsPlotWidget::AxisScaleType RiuQtChartsPlotWidget::axisScaleType( RiaDefines::PlotAxis axis ) const
{
    // QwtPlot::Axis qwtAxis = RiuQwtPlotTools::toQwtPlotAxis( axis );

    // QwtLogScaleEngine*  logScaleEngine  = dynamic_cast<QwtLogScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    // QwtDateScaleEngine* dateScaleEngine = dynamic_cast<QwtDateScaleEngine*>( m_plot->axisScaleEngine( qwtAxis ) );
    // if ( logScaleEngine != nullptr )
    //     return AxisScaleType::LOGARITHMIC;
    // else if ( dateScaleEngine != nullptr )
    //     return AxisScaleType::DATE;

    return AxisScaleType::LINEAR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxisScaleType( RiaDefines::PlotAxis axis, RiuQtChartsPlotWidget::AxisScaleType axisScaleType )
{
    // QwtPlot::Axis qwtAxis = RiuQwtPlotTools::toQwtPlotAxis( axis );

    // if ( axisScaleType == AxisScaleType::LOGARITHMIC ) m_plot->setAxisScaleEngine( qwtAxis, new QwtLogScaleEngine );
    // if ( axisScaleType == AxisScaleType::LINEAR ) m_plot->setAxisScaleEngine( qwtAxis, new QwtLinearScaleEngine );
    // if ( axisScaleType == AxisScaleType::DATE ) m_plot->setAxisScaleEngine( qwtAxis, new QwtDateScaleEngine );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::updateAxes()
{
    // m_plot->updateAxes();
    m_viewer->chart()->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurve* RiuQtChartsPlotWidget::createPlotCurve( const QString& title, const QColor& color )
{
    return new RiuQtChartsPlotCurve( title );
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
void RiuQtChartsPlotWidget::attach( QtCharts::QAbstractSeries* series, RiaDefines::PlotAxis xAxis, RiaDefines::PlotAxis yAxis )
{
    if ( !series->chart() )
    {
        qtChart()->addSeries( series );
        setXAxis( xAxis, series );
        setXAxis( yAxis, series );
        rescaleAxis( xAxis );
        rescaleAxis( yAxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::detachItems( RiuPlotWidget::PlotItemType plotItemType )
{
    if ( plotItemType == RiuPlotWidget::PlotItemType::CURVE )
    {
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
void RiuQtChartsPlotWidget::setXAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series )
{
    setAxis( axis, series );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setYAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series )
{
    setAxis( axis, series );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::setAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series )
{
    if ( qtChart()->series().contains( series ) && !series->attachedAxes().contains( plotAxis( axis ) ) )
    {
        auto newAxis = plotAxis( axis );

        // Detach any other axis for the same orientation
        for ( auto ax : series->attachedAxes() )
        {
            if ( ax->orientation() == orientation( axis ) )
            {
                series->detachAxis( ax );
            }
        }

        series->attachAxis( newAxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotWidget::rescaleAxis( RiaDefines::PlotAxis axis )
{
    QValueAxis* pAxis = plotAxis( axis );

    double min = std::numeric_limits<double>::max();
    double max = -std::numeric_limits<double>::max();
    for ( auto series : qtChart()->series() )
    {
        auto attachedAxes = series->attachedAxes();
        if ( attachedAxes.contains( pAxis ) )
        {
            for ( auto attachedAxis : attachedAxes )
            {
                QValueAxis*     valueAxis = dynamic_cast<QValueAxis*>( attachedAxis );
                Qt::Orientation orr       = orientation( axis );
                if ( valueAxis && valueAxis->orientation() == orr )
                {
                    for ( auto p : dynamic_cast<QLineSeries*>( series )->pointsVector() )
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
    }

    cvf::Trace::show( QString( "RESCALE: %1 - %2" ).arg( min ).arg( max ).toStdString().c_str() );
    pAxis->setRange( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QValueAxis* RiuQtChartsPlotWidget::plotAxis( RiaDefines::PlotAxis axis ) const
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
