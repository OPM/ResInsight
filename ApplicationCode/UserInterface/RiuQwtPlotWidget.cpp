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

#include "RiaApplication.h"
#include "RiaColorTools.h"
#include "RiaPlotWindowRedrawScheduler.h"

#include "RimPlot.h"
#include "RimPlotCurve.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtScalePicker.h"

#include "cafAssert.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"
#include "qwt_symbol.h"
#include "qwt_text.h"

#include <QDrag>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QScrollArea>
#include <QWheelEvent>

#include <cfloat>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget::RiuQwtPlotWidget( RimPlot* plot, QWidget* parent )
    : QwtPlot( parent )
    , m_plotDefinition( plot )
    , m_draggable( true )
{
    setDefaults();

    this->installEventFilter( this );
    this->canvas()->installEventFilter( this );
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
void RiuQwtPlotWidget::setDraggable( bool draggable )
{
    m_draggable = draggable;
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
void RiuQwtPlotWidget::setAxisFontsAndAlignment( QwtPlot::Axis     axis,
                                                 int               titleFontSize,
                                                 int               valueFontSize,
                                                 bool              titleBold,
                                                 Qt::AlignmentFlag alignment )
{
    // Axis number font
    QFont axisFont = this->axisFont( axis );
    axisFont.setPointSize( valueFontSize );
    axisFont.setBold( false );
    this->setAxisFont( axis, axisFont );

    // Axis title font
    QwtText axisTitle     = this->axisTitle( axis );
    QFont   axisTitleFont = axisTitle.font();
    axisTitleFont.setPointSize( titleFontSize );
    axisTitleFont.setBold( titleBold );
    axisTitle.setFont( axisTitleFont );
    axisTitle.setRenderFlags( alignment );

    setAxisTitle( axis, axisTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RiuQwtPlotWidget::plotDefinition() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setEnabledAxes( const std::set<QwtPlot::Axis> enabledAxes )
{
    for ( int axisId = 0; axisId < QwtPlot::axisCnt; ++axisId )
    {
        QwtPlot::Axis axis = static_cast<QwtPlot::Axis>( axisId );
        if ( enabledAxes.count( axis ) )
        {
            enableAxis( axis, true );
            // Align the canvas with the actual min and max values of the curves
            axisScaleEngine( axis )->setAttribute( QwtScaleEngine::Floating, true );
            setAxisScale( axis, 0.0, 100.0 );
            axisScaleDraw( axis )->setMinimumExtent( axisExtent( axis ) );
            setMinimumWidth( defaultMinimumWidth() );

            axisWidget( axis )->setMargin( 0 );
            m_axisTitlesEnabled[axis] = true;
        }
        else
        {
            enableAxis( axis, false );
            m_axisTitlesEnabled[axis] = false;
        }
    }
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
QwtInterval RiuQwtPlotWidget::axisRange( QwtPlot::Axis axis )
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
void RiuQwtPlotWidget::setAxisLabelsAndTicksEnabled( QwtPlot::Axis axis, bool enable )
{
    this->axisScaleDraw( axis )->enableComponent( QwtAbstractScaleDraw::Ticks, enable );
    this->axisScaleDraw( axis )->enableComponent( QwtAbstractScaleDraw::Labels, enable );
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
        QwtScaleDiv scaleDiv = scaleEngine->divideScaleWithExplicitIntervals( minValue,
                                                                              maxValue,
                                                                              majorTickInterval,
                                                                              minorTickInterval );

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
    int lineExtent = 5;

    lineExtent += this->axisScaleDraw( axis )->maxTickLength();

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
void RiuQwtPlotWidget::setWidgetState( const QString& widgetState )
{
    caf::UiStyleSheet::clearWidgetStates( this );
    m_plotStyleSheet.setWidgetState( this, widgetState );
}

//--------------------------------------------------------------------------------------------------
/// Adds an overlay frame. The overlay frame becomes the responsibility of the plot widget
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::addOverlayFrame( QFrame* overlayFrame )
{
    if ( std::find( m_overlayFrames.begin(), m_overlayFrames.end(), overlayFrame ) == m_overlayFrames.end() )
    {
        overlayFrame->setParent( this );
        m_overlayFrames.push_back( overlayFrame );
        updateLayout();
    }
}

//--------------------------------------------------------------------------------------------------
/// Remove the overlay widget. The frame becomes the responsibility of the caller
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::removeOverlayFrame( QFrame* overlayFrame )
{
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
bool RiuQwtPlotWidget::eventFilter( QObject* watched, QEvent* event )
{
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>( event );
    if ( mouseEvent )
    {
        if ( isZoomerActive() ) return false;

        bool toggleItemInSelection = ( mouseEvent->modifiers() & Qt::ControlModifier ) != 0;

        if ( m_draggable && mouseEvent->type() == QMouseEvent::MouseButtonPress && mouseEvent->button() == Qt::LeftButton )
        {
            m_clickPosition = mouseEvent->pos();
        }

        if ( watched == this && !this->canvas()->geometry().contains( mouseEvent->pos() ) )
        {
            if ( m_draggable && mouseEvent->type() == QMouseEvent::MouseMove &&
                 ( mouseEvent->buttons() & Qt::LeftButton ) && !m_clickPosition.isNull() )
            {
                int dragLength = ( mouseEvent->pos() - m_clickPosition ).manhattanLength();
                if ( dragLength >= QApplication::startDragDistance() )
                {
                    QPixmap    pixmap   = this->grab();
                    QDrag*     drag     = new QDrag( this );
                    QMimeData* mimeData = new QMimeData;
                    mimeData->setImageData( pixmap );
                    drag->setMimeData( mimeData );
                    drag->setPixmap( pixmap );
                    drag->setHotSpot( mouseEvent->pos() );
                    drag->exec( Qt::MoveAction );
                    return true;
                }
            }
            else if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease &&
                      ( mouseEvent->button() == Qt::LeftButton ) && !m_clickPosition.isNull() )
            {
                QWidget* childClicked = this->childAt( m_clickPosition );

                if ( childClicked )
                {
                    QwtScaleWidget* scaleWidget = qobject_cast<QwtScaleWidget*>( childClicked );
                    if ( scaleWidget )
                    {
                        onAxisSelected( scaleWidget, toggleItemInSelection );
                        return true;
                    }
                }
                else
                {
                    selectPlotOwner( toggleItemInSelection );
                    endZoomOperations();
                    return true;
                }
            }
        }
        else if ( watched == canvas() )
        {
            if ( mouseEvent->type() == QMouseEvent::MouseButtonRelease && mouseEvent->button() == Qt::LeftButton &&
                 !m_clickPosition.isNull() )
            {
                selectClosestCurve( mouseEvent->pos(), toggleItemInSelection );
                endZoomOperations();
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
    resetCurveHighlighting();
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
void RiuQwtPlotWidget::applyAxisTitleToQwt( QwtPlot::Axis axis )
{
    QString titleToApply = m_axisTitlesEnabled[axis] ? m_axisTitles[axis] : QString( "" );
    QwtText axisTitle    = this->axisTitle( axis );
    if ( titleToApply != axisTitle.text() )
    {
        axisTitle.setText( titleToApply );

        setAxisTitle( axis, axisTitle );
        if ( axis == QwtPlot::yLeft || axis == QwtPlot::yRight )
        {
            axisScaleDraw( axis )->setMinimumExtent( axisExtent( axis ) );
            setMinimumWidth( defaultMinimumWidth() + axisExtent( axis ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::selectPoint( QwtPlotCurve* curve, int pointNumber ) {}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::clearPointSelection() {}

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
void RiuQwtPlotWidget::endZoomOperations() {}

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
    m_plotDefinition->onAxisSelected( axisId, toggleItemInSelection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet RiuQwtPlotWidget::createPlotStyleSheet() const
{
    QColor backgroundColor       = QColor( "white" );
    QColor highlightColor        = QApplication::palette().highlight().color();
    QColor blendedHighlightColor = RiaColorTools::blendQColors( highlightColor, backgroundColor, 1, 20 );
    QColor nearlyBackgroundColor = RiaColorTools::blendQColors( highlightColor, backgroundColor, 1, 40 );

    caf::UiStyleSheet styleSheet;
    styleSheet.set( "background-color", backgroundColor.name() );
    styleSheet.set( "border", "1 px solid transparent" );

    styleSheet.property( "selected" ).set( "border", QString( "1px solid %1" ).arg( highlightColor.name() ) );

    if ( m_draggable )
    {
        QString backgroundGradient = QString( QString( "qlineargradient( x1 : 1, y1 : 0, x2 : 1, y2 : 1,"
                                                       "stop: 0 %1, stop: 0.02 %2, stop:1 %3 )" )
                                                  .arg( blendedHighlightColor.name() )
                                                  .arg( nearlyBackgroundColor.name() )
                                                  .arg( backgroundColor.name() ) );

        styleSheet.pseudoState( "hover" ).set( "background", backgroundGradient );
        styleSheet.pseudoState( "hover" ).set( "border", QString( "1px dashed %1" ).arg( blendedHighlightColor.name() ) );
    }
    styleSheet.property( "dropTargetBefore" ).set( "border-left", "1px solid lime" );
    styleSheet.property( "dropTargetAfter" ).set( "border-right", "1px solid lime" );
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
    const int spacing      = 5;
    int       startMarginX = this->canvas()->pos().x() + spacing;
    int       startMarginY = this->canvas()->pos().y() + spacing;

    int xpos           = startMarginX;
    int ypos           = startMarginY;
    int maxColumnWidth = 0;
    for ( QPointer<QFrame> frame : m_overlayFrames )
    {
        if ( !frame.isNull() )
        {
            if ( ypos + frame->height() + spacing > this->canvas()->height() )
            {
                xpos += spacing + maxColumnWidth;
                ypos           = startMarginY;
                maxColumnWidth = 0;
            }
            frame->move( xpos, ypos );
            ypos += frame->height() + spacing;
            maxColumnWidth = std::max( maxColumnWidth, frame->width() );
            frame->show();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::setDefaults()
{
    setEnabledAxes( {QwtPlot::xTop, QwtPlot::yLeft} );
    RiuQwtPlotTools::setCommonPlotBehaviour( this );
}

void RiuQwtPlotWidget::selectPlotOwner( bool toggleItemInSelection )
{
    if ( toggleItemInSelection )
    {
        RiuPlotMainWindowTools::toggleItemInSelection( m_plotDefinition );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( m_plotDefinition );
    }
    scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::selectClosestCurve( const QPoint& pos, bool toggleItemInSelection /*= false*/ )
{
    QwtPlotCurve* closestCurve      = nullptr;
    double        distMin           = DBL_MAX;
    int           closestCurvePoint = -1;

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
                closestCurve      = candidateCurve;
                distMin           = dist;
                closestCurvePoint = curvePoint;
            }
        }
    }

    RiuPlotMainWindowTools::showPlotMainWindow();
    resetCurveHighlighting();
    if ( closestCurve && distMin < 20 )
    {
        if ( m_plotDefinition )
        {
            RimPlotCurve* selectedCurve = dynamic_cast<RimPlotCurve*>(
                m_plotDefinition->findPdmObjectFromQwtCurve( closestCurve ) );
            if ( selectedCurve )
            {
                if ( toggleItemInSelection )
                {
                    RiuPlotMainWindowTools::toggleItemInSelection( selectedCurve );
                }
                else
                {
                    RiuPlotMainWindowTools::selectAsCurrentItem( selectedCurve );
                }
                // TODO: highlight all selected curves
                highlightCurve( closestCurve );
            }
        }
        if ( distMin < 10 )
        {
            selectPoint( closestCurve, closestCurvePoint );
        }
        else
        {
            clearPointSelection();
        }
        scheduleReplot();
    }
    else
    {
        selectPlotOwner( toggleItemInSelection );
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
void RiuQwtPlotWidget::highlightCurve( const QwtPlotCurve* closestCurve )
{
    // NB! Create a copy of the item list before the loop to avoid invalidated iterators when iterating the list
    // plotCurve->setZ() causes the ordering of items in the list to change
    auto plotItemList = this->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        QwtPlotCurve* plotCurve = dynamic_cast<QwtPlotCurve*>( plotItem );
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
            if ( plotCurve == closestCurve )
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
            CurveColors curveColors = {curveColor, symbolColor, symbolLineColor};
            m_originalCurveColors.insert( std::make_pair( plotCurve, curveColors ) );
            m_originalCurveColors.insert( std::make_pair( plotCurve, curveColors ) );
            m_originalZValues.insert( std::make_pair( plotCurve, zValue ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotWidget::resetCurveHighlighting()
{
    // NB! Create a copy of the item list before the loop to avoid invalidated iterators when iterating the list
    // plotCurve->setZ() causes the ordering of items in the list to change
    auto plotItemList = this->itemList();
    for ( QwtPlotItem* plotItem : plotItemList )
    {
        QwtPlotCurve* plotCurve = dynamic_cast<QwtPlotCurve*>( plotItem );
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
    }
    m_originalCurveColors.clear();
    m_originalZValues.clear();
}
