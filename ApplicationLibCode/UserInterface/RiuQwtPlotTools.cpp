/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiuQwtPlotTools.h"

#include "RiuGuiTheme.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_shapeitem.h"
#include "qwt_scale_widget.h"

#include <QRegExp>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::setCommonPlotBehaviour( QwtPlot* plot )
{
    // Plot background and frame look

    QPalette newPalette( plot->palette() );
    newPalette.setColor( QPalette::Window, Qt::white );
    plot->setPalette( newPalette );

    plot->setAutoFillBackground( true );
    plot->setCanvasBackground( Qt::white );
    plot->plotLayout()->setCanvasMargin( 0, -1 );

    QFrame* canvasFrame = dynamic_cast<QFrame*>( plot->canvas() );
    canvasFrame->setFrameShape( QFrame::Box );

    // Grid
    QwtPlotGrid* grid = new QwtPlotGrid;
    grid->attach( plot );
    QPen gridPen( Qt::SolidLine );
    grid->setPen( gridPen );
    RiuGuiTheme::styleQwtItem( grid );

    // Axis number font
    int   axisFontSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(),
                                                          caf::FontTools::RelativeSize::Medium );
    QFont axisFont     = plot->axisFont( QwtPlot::xBottom );
    axisFont.setPixelSize( caf::FontTools::pointSizeToPixelSize( axisFontSize ) );

    plot->setAxisFont( QwtPlot::xBottom, axisFont );
    plot->setAxisFont( QwtPlot::xTop, axisFont );
    plot->setAxisFont( QwtPlot::yLeft, axisFont );
    plot->setAxisFont( QwtPlot::yRight, axisFont );

    // Axis title font
    std::vector<QwtPlot::Axis> axes = { QwtPlot::xBottom, QwtPlot::xTop, QwtPlot::yLeft, QwtPlot::yRight };

    for ( QwtPlot::Axis axis : axes )
    {
        QwtText axisTitle     = plot->axisTitle( axis );
        QFont   axisTitleFont = axisTitle.font();
        axisTitleFont.setPixelSize( caf::FontTools::pointSizeToPixelSize( axisFontSize ) );
        axisTitleFont.setBold( false );
        axisTitle.setFont( axisTitleFont );
        axisTitle.setRenderFlags( Qt::AlignRight );

        plot->setAxisTitle( axis, axisTitle );
    }

    // Set a focus policy to allow it taking key press events.
    // This is not strictly necessary since this widget inherit QwtPlot
    // which already has a focus policy.
    // However, for completeness we still do it here.
    plot->setFocusPolicy( Qt::WheelFocus );

    // Enable mousetracking and event filter
    plot->canvas()->setMouseTracking( true );
    plot->plotLayout()->setAlignCanvasToScales( true );

    plot->setContentsMargins( 1, 1, 1, 1 );

    // Store the pointer address as an object name. This way each plot can be identified uniquely for CSS-stylesheets
    QString objectName = QString( "%1" ).arg( reinterpret_cast<uint64_t>( plot ) );
    plot->setObjectName( objectName );

    QString canvasName = QString( "%1" ).arg( reinterpret_cast<uint64_t>( plot->canvas() ) );
    plot->canvas()->setObjectName( canvasName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::setDefaultAxes( QwtPlot* plot )
{
    plot->enableAxis( QwtPlot::xBottom, true );
    plot->enableAxis( QwtPlot::yLeft, true );
    plot->enableAxis( QwtPlot::xTop, false );
    plot->enableAxis( QwtPlot::yRight, false );

    plot->axisWidget( QwtPlot::xBottom )->setMargin( 0 );
    plot->axisWidget( QwtPlot::yLeft )->setMargin( 0 );
    plot->axisWidget( QwtPlot::xTop )->setMargin( 0 );
    plot->axisWidget( QwtPlot::yRight )->setMargin( 0 );

    plot->setAxisMaxMinor( QwtPlot::xBottom, 2 );
    plot->setAxisMaxMinor( QwtPlot::yLeft, 3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::enableDateBasedBottomXAxis( QwtPlot*                                plot,
                                                  const QString&                          dateFormat,
                                                  const QString&                          timeFormat,
                                                  RiaQDateTimeTools::DateFormatComponents dateComponents,
                                                  RiaQDateTimeTools::TimeFormatComponents timeComponents )
{
    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw( Qt::UTC );

    std::set<QwtDate::IntervalType> intervals = { QwtDate::Year,
                                                  QwtDate::Month,
                                                  QwtDate::Week,
                                                  QwtDate::Day,
                                                  QwtDate::Hour,
                                                  QwtDate::Minute,
                                                  QwtDate::Second,
                                                  QwtDate::Millisecond };

    for ( QwtDate::IntervalType interval : intervals )
    {
        scaleDraw->setDateFormat( interval,
                                  dateTimeFormatForInterval( interval, dateFormat, timeFormat, dateComponents, timeComponents ) );
    }

    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine( Qt::UTC );
    plot->setAxisScaleEngine( QwtPlot::xBottom, scaleEngine );
    plot->setAxisScaleDraw( QwtPlot::xBottom, scaleDraw );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuQwtPlotTools::dateTimeFormatForInterval( QwtDate::IntervalType                   interval,
                                                    const QString&                          dateFormat,
                                                    const QString&                          timeFormat,
                                                    RiaQDateTimeTools::DateFormatComponents dateComponents,
                                                    RiaQDateTimeTools::TimeFormatComponents timeComponents )
{
    if ( dateComponents != RiaQDateTimeTools::DATE_FORMAT_UNSPECIFIED &&
         timeComponents != RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED )
    {
        return RiaQDateTimeTools::timeFormatString( timeFormat, timeComponents ) + "\n" +
               RiaQDateTimeTools::dateFormatString( dateFormat, dateComponents );
    }
    else
    {
        switch ( interval )
        {
            case QwtDate::Millisecond:
                return RiaQDateTimeTools::timeFormatString( timeFormat,
                                                            RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND_MILLISECOND );
            case QwtDate::Second:
                return RiaQDateTimeTools::timeFormatString( timeFormat,
                                                            RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );
            case QwtDate::Minute:
            {
                QString fullFormat =
                    RiaQDateTimeTools::timeFormatString( timeFormat,
                                                         RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );
                fullFormat += "\n";
                fullFormat +=
                    RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
                return fullFormat;
            }
            case QwtDate::Hour:
            {
                QString fullFormat =
                    RiaQDateTimeTools::timeFormatString( timeFormat,
                                                         RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR );
                if ( !fullFormat.endsWith( "AP" ) )
                {
                    fullFormat += ":00";
                }
                fullFormat += "\n";
                fullFormat +=
                    RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
                return fullFormat;
            }
            case QwtDate::Day:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
            case QwtDate::Week:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH );
            case QwtDate::Month:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH );
            case QwtDate::Year:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR );
            default:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotShapeItem* RiuQwtPlotTools::createBoxShape( const QString& label,
                                                   double         startX,
                                                   double         endX,
                                                   double         startY,
                                                   double         endY,
                                                   QColor         color,
                                                   Qt::BrushStyle brushStyle )
{
    return createBoxShapeT<QwtPlotShapeItem>( label, startX, endX, startY, endY, color, brushStyle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlot::Axis RiuQwtPlotTools::toQwtPlotAxis( RiaDefines::PlotAxis axis )
{
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
        return QwtPlot::yLeft;
    else if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        return QwtPlot::yRight;
    else if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        return QwtPlot::xBottom;

    return QwtPlot::xTop;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RiuQwtPlotTools::fromQwtPlotAxis( QwtPlot::Axis axis )
{
    if ( axis == QwtPlot::yLeft )
        return RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    else if ( axis == QwtPlot::yRight )
        return RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    else if ( axis == QwtPlot::xBottom )
        return RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;

    return RiaDefines::PlotAxis::PLOT_AXIS_TOP;
}
