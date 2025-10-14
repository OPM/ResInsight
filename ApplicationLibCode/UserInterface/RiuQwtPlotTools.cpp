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

#include "RiaApplication.h"
#include "RiaColorTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiuPlotAxis.h"

#include "RimPlotCurve.h"

#include "RiuGuiTheme.h"
#include "RiuPlotCurveSymbolImageCreator.h"
#include "RiuQwtPlotLegend.h"

#include "qwt_axis.h"
#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_graphic.h"
#include "qwt_painter.h"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_shapeitem.h"
#include "qwt_scale_widget.h"

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
    int axisFontSize =
        caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), caf::FontTools::RelativeSize::Medium );
    QFont axisFont = plot->axisFont( QwtAxis::XBottom );
    axisFont.setPointSize( axisFontSize );

    plot->setAxisFont( QwtAxis::XBottom, axisFont );
    plot->setAxisFont( QwtAxis::XTop, axisFont );
    plot->setAxisFont( QwtAxis::YLeft, axisFont );
    plot->setAxisFont( QwtAxis::YRight, axisFont );

    // Axis title font
    std::vector<QwtAxis::Position> axes = { QwtAxis::XBottom, QwtAxis::XTop, QwtAxis::YLeft, QwtAxis::YRight };

    for ( QwtAxis::Position axis : axes )
    {
        QwtText axisTitle     = plot->axisTitle( axis );
        QFont   axisTitleFont = axisTitle.font();
        axisTitleFont.setPointSize( axisFontSize );
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
    plot->setAxesCount( QwtAxis::XBottom, 1 );
    plot->setAxesCount( QwtAxis::YLeft, 1 );

    plot->axisWidget( QwtAxis::XBottom )->setMargin( 0 );
    plot->axisWidget( QwtAxis::YLeft )->setMargin( 0 );
    plot->axisWidget( QwtAxis::XTop )->setMargin( 0 );
    plot->axisWidget( QwtAxis::YRight )->setMargin( 0 );

    plot->setAxisMaxMinor( QwtAxis::XBottom, 2 );
    plot->setAxisMaxMinor( QwtAxis::YLeft, 3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::enableDateBasedBottomXAxis( QwtPlot*                         plot,
                                                  const QString&                   dateFormat,
                                                  const QString&                   timeFormat,
                                                  RiaDefines::DateFormatComponents dateComponents,
                                                  RiaDefines::TimeFormatComponents timeComponents )
{
    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw( Qt::UTC );

    std::set<QwtDate::IntervalType> intervals =
        { QwtDate::Year, QwtDate::Month, QwtDate::Week, QwtDate::Day, QwtDate::Hour, QwtDate::Minute, QwtDate::Second, QwtDate::Millisecond };

    for ( QwtDate::IntervalType interval : intervals )
    {
        scaleDraw->setDateFormat( interval, dateTimeFormatForInterval( interval, dateFormat, timeFormat, dateComponents, timeComponents ) );
    }

    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine( Qt::UTC );
    plot->setAxisScaleEngine( QwtAxis::XBottom, scaleEngine );
    plot->setAxisScaleDraw( QwtAxis::XBottom, scaleDraw );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuQwtPlotTools::dateTimeFormatForInterval( QwtDate::IntervalType            interval,
                                                    const QString&                   dateFormat,
                                                    const QString&                   timeFormat,
                                                    RiaDefines::DateFormatComponents dateComponents,
                                                    RiaDefines::TimeFormatComponents timeComponents )
{
    if ( dateComponents != RiaDefines::DateFormatComponents::DATE_FORMAT_UNSPECIFIED &&
         timeComponents != RiaDefines::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED )
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
                                                            RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND_MILLISECOND );
            case QwtDate::Second:
                return RiaQDateTimeTools::timeFormatString( timeFormat, RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );
            case QwtDate::Minute:
            {
                QString fullFormat =
                    RiaQDateTimeTools::timeFormatString( timeFormat, RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );
                fullFormat += "\n";
                fullFormat += RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
                return fullFormat;
            }
            case QwtDate::Hour:
            {
                QString fullFormat = RiaQDateTimeTools::timeFormatString( timeFormat, RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR );
                if ( !fullFormat.endsWith( "AP" ) )
                {
                    fullFormat += ":00";
                }
                fullFormat += "\n";
                fullFormat += RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
                return fullFormat;
            }
            case QwtDate::Day:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
            case QwtDate::Week:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH );
            case QwtDate::Month:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH );
            case QwtDate::Year:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR );
            default:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotShapeItem*
    RiuQwtPlotTools::createBoxShape( const QString& label, double startX, double endX, double startY, double endY, QColor color, Qt::BrushStyle brushStyle )
{
    return createBoxShapeT<QwtPlotShapeItem>( label, startX, endX, startY, endY, color, brushStyle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtAxis::Position RiuQwtPlotTools::toQwtPlotAxisEnum( RiaDefines::PlotAxis riaPlotAxis )
{
    if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
        return QwtAxis::YLeft;
    else if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        return QwtAxis::YRight;
    else if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        return QwtAxis::XBottom;
    else if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_TOP )
        return QwtAxis::XTop;

    return QwtAxis::YLeft;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RiuQwtPlotTools::fromQwtPlotAxis( QwtAxis::Position axis )
{
    if ( axis == QwtAxis::YLeft )
        return RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    else if ( axis == QwtAxis::YRight )
        return RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    else if ( axis == QwtAxis::XBottom )
        return RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
    else if ( axis == QwtAxis::XTop )
        return RiaDefines::PlotAxis::PLOT_AXIS_TOP;

    return RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::updateLegendData( RiuQwtPlotLegend* legend, const std::vector<RimPlotCurve*>& curves )
{
    QList<QwtLegendData> legendDataList = createLegendData( curves );

    legend->updateLegend( QVariant(), legendDataList );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QwtLegendData> RiuQwtPlotTools::createLegendData( const std::vector<RimPlotCurve*>& curves )
{
    QList<QwtLegendData> legendDataList;

    for ( auto c : curves )
    {
        QwtLegendData test;
        test.setValue( QwtLegendData::Role::TitleRole, c->curveName() );

        c->updateUiIconFromPlotSymbol();
        auto icon = c->uiIcon();
        auto size = icon->availableSizes().first();
        // see QwtPlotCurve::legendIcon

        QwtGraphic graphic;
        {
            graphic.setDefaultSize( size );
            graphic.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

            QPainter painter( &graphic );
            painter.setRenderHint( QPainter::Antialiasing );

            {
                QPen pn;
                pn.setCapStyle( Qt::FlatCap );
                pn.setColor( RiaColorTools::toQColor( c->color() ) );

                painter.setPen( pn );

                const double y = 0.5 * size.height();
                QwtPainter::drawLine( &painter, 0.0, y, size.width(), y );
            }

            if ( c->symbol() != RiuPlotCurveSymbol::SYMBOL_NONE )
            {
                auto image = RiuPlotCurveSymbolImageCreator::createSymbolImage( c->symbol(),
                                                                                QSize( size.width() / 2, size.height() / 2 ),
                                                                                RiaColorTools::toQColor( c->color() ) );

                QPoint p( size.width() / 4, size.height() / 4 );
                painter.drawImage( p, image );
            }
        }

        QVariant v = QVariant::fromValue( graphic );
        test.setValue( QwtLegendData::Role::IconRole, v );

        legendDataList.push_back( test );
    }

    return legendDataList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::enableGridLines( QwtPlot* plot, QwtAxis::Position axis, bool enableMajor, bool enableMinor )
{
    // NB: Make a copy, not reference to avoid iterator invalidation
    QwtPlotItemList plotItems = plot->itemList( QwtPlotItem::Rtti_PlotGrid );
    for ( QwtPlotItem* plotItem : plotItems )
    {
        auto* grid = static_cast<QwtPlotGrid*>( plotItem );
        if ( axis == QwtAxis::XTop || axis == QwtAxis::XBottom )
        {
            grid->setXAxis( axis );
            grid->enableX( enableMajor );
            grid->enableXMin( enableMinor );
        }
        else
        {
            grid->setYAxis( axis );
            grid->enableY( enableMajor );
            grid->enableYMin( enableMinor );
        }
        grid->setMajorPen( Qt::lightGray, 1.0, Qt::SolidLine );
        grid->setMinorPen( Qt::lightGray, 1.0, Qt::DashLine );
    }
}
