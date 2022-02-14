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

    // Axis number font
    int axisFontSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(),
                                                          caf::FontTools::RelativeSize::Medium );

    // Axis title font
    int  titleFontSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(),
                                                           caf::FontTools::RelativeSize::Medium );
    bool titleBold     = false;
    plot->setAxesFontsAndAlignment( titleFontSize, axisFontSize, titleBold, Qt::AlignRight );

    // Store the pointer address as an object name. This way
    // each plot can be identified uniquely for CSS-stylesheets
    QString objectName = QString( "%1" ).arg( reinterpret_cast<uint64_t>( plot ) );
    plot->setObjectName( objectName );

    QString canvasName = QString( "%1" ).arg( reinterpret_cast<uint64_t>( plot->getParentForOverlay() ) );
    plot->getParentForOverlay()->setObjectName( canvasName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQtChartsPlotTools::setDefaultAxes( RiuQtChartsPlotWidget* plot )
{
    plot->enableAxis( RiuPlotAxis::defaultBottom(), true );
    plot->enableAxis( RiuPlotAxis::defaultLeft(), true );
    plot->enableAxis( RiuPlotAxis::defaultTop(), false );
    plot->enableAxis( RiuPlotAxis::defaultRight(), false );

    plot->setAxisMaxMinor( RiuPlotAxis::defaultBottom(), 2 );
    plot->setAxisMaxMinor( RiuPlotAxis::defaultLeft(), 0 );
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
    plot->setAxisFormat( RiuPlotAxis::defaultBottom(), format );
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
