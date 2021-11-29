/////////////////////////////////////////////////////////////////////////////////
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
#include "RiuQtChartsPlotTools.h"

#include "RiaPlotDefines.h"
#include "RiuGuiTheme.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiuQtChartsPlotWidget.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotTools::setCommonPlotBehaviour( RiuQtChartsPlotWidget* plot )
{
    // Plot background and frame look

    QPalette newPalette( plot->palette() );
    newPalette.setColor( QPalette::Window, Qt::white );
    plot->setPalette( newPalette );

    plot->setAutoFillBackground( true );
    // plot->setCanvasBackground( Qt::white );
    // plot->plotLayout()->setCanvasMargin( 0, -1 );

    // Grid
    // RiuQtChartsPlotWidgetGrid* grid = new RiuQtChartsPlotWidgetGrid;
    // grid->attach( plot );
    // QPen gridPen( Qt::SolidLine );
    // grid->setPen( gridPen );
    // RiuGuiTheme::styleQwtItem( grid );

    // Axis number font
    int axisFontSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(),
                                                          caf::FontTools::RelativeSize::Medium );

    // Axis title font
    int  titleFontSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(),
                                                           caf::FontTools::RelativeSize::Medium );
    bool titleBold     = false;
    plot->setAxesFontsAndAlignment( titleFontSize, axisFontSize, titleBold, Qt::AlignRight );

    // Set a focus policy to allow it taking key press events.
    // This is not strictly necessary since this widget inherit
    // RiuQtChartsPlotWidget which already has a focus policy.
    // However, for completeness we still do it here.
    // plot->setFocusPolicy( Qt::WheelFocus );

    // Enable mousetracking and event filter
    // plot->canvas()->setMouseTracking( true );
    // plot->plotLayout()->setAlignCanvasToScales( true );

    // plot->setContentsMargins( 1, 1, 1, 1 );

    // // Store the pointer address as an object name. This way
    // each plot can be identified uniquely for CSS-stylesheets
    // QString objectName = QString( "%1" ).arg(
    // reinterpret_cast<uint64_t>( plot ) ); plot->setObjectName(
    // objectName );

    // QString canvasName = QString( "%1" ).arg(
    // reinterpret_cast<uint64_t>( plot->canvas() ) );
    // plot->canvas()->setObjectName( canvasName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotTools::setDefaultAxes( RiuQtChartsPlotWidget* plot )
{
    plot->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, true );
    plot->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, true );
    plot->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_TOP, false );
    plot->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT, false );

    plot->setAxisMaxMinor( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, 2 );
    plot->setAxisMaxMinor( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, 3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotTools::enableDateBasedBottomXAxis( RiuQtChartsPlotWidget*                  plot,
                                                       const QString&                          dateFormat,
                                                       const QString&                          timeFormat,
                                                       RiaQDateTimeTools::DateFormatComponents dateComponents,
                                                       RiaQDateTimeTools::TimeFormatComponents timeComponents )
{
    QString format = dateTimeFormatForInterval( dateFormat, timeFormat, dateComponents, timeComponents );
    plot->setAxisFormat( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, format );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuQtChartsPlotTools::dateTimeFormatForInterval( const QString&                          dateFormat,
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

    // Default:
    return RiaQDateTimeTools::timeFormatString( timeFormat, RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_NONE ) +
           "\n" +
           RiaQDateTimeTools::dateFormatString( dateFormat,
                                                RiaQDateTimeTools::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
}
