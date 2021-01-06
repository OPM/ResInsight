/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimTimeAxisAnnotation.h"

#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"

#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include "RiuGuiTheme.h"
#include "RiuQwtPlotCurve.h"

#include "RimEclipseCase.h"
#include "RimPlot.h"
#include "RimTools.h"
#include "RimViewWindow.h"

#include <QDateTime>

#include <cmath>

CAF_PDM_SOURCE_INIT( RimTimeAxisAnnotation, "RimTimeAxisAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTimeAxisAnnotation::RimTimeAxisAnnotation()
    : RimPlotAxisAnnotation()
{
    CAF_PDM_InitObject( "Time Axis Annotation", ":/LeftAxis16x16.png", "", "" );

    m_value.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTime( time_t time )
{
    m_value = RiaTimeTTools::toDouble( time );

    QString dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                    RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );

    QString timeFormatString =
        RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                             RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );

    QString dateTimeFormatString = QString( "%1 %2" ).arg( dateFormatString ).arg( timeFormatString );

    m_name =
        RiaQDateTimeTools::toStringUsingApplicationLocale( RiaQDateTimeTools::fromTime_t( time ), dateTimeFormatString );

    this->setAnnotationType( AnnotationType::LINE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTimeRange( time_t startTime, time_t endTime )
{
    m_rangeStart = RiaTimeTTools::toDouble( startTime );
    m_rangeEnd   = RiaTimeTTools::toDouble( endTime );

    QString dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                    RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );

    QString timeFormatString =
        RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                             RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );

    QString dateTimeFormatString = QString( "%1 %2" ).arg( dateFormatString ).arg( timeFormatString );

    m_name = QString( "%0 - %1" )
                 .arg( RiaQDateTimeTools::toStringUsingApplicationLocale( RiaQDateTimeTools::fromTime_t( startTime ),
                                                                          dateTimeFormatString ) )
                 .arg( RiaQDateTimeTools::toStringUsingApplicationLocale( RiaQDateTimeTools::fromTime_t( endTime ),
                                                                          dateTimeFormatString ) );

    this->setAnnotationType( AnnotationType::RANGE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RimTimeAxisAnnotation::color() const
{
    if ( annotationType() == AnnotationType::LINE )
    {
        return RiuGuiTheme::getColorByVariableName( "secondaryColor" ); // QColor(255, 0, 0);
    }
    else if ( annotationType() == RimPlotAxisAnnotation::AnnotationType::RANGE )
    {
        return RiuGuiTheme::getColorByVariableName( "primaryColor" ); // QColor( 0, 0, 255 );
    }
    return RiuGuiTheme::getColorByVariableName( "textColor" ); // QColor(0, 0, 100);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_value );
    uiOrdering.add( &m_rangeStart );
    uiOrdering.add( &m_rangeEnd );

    uiOrdering.skipRemainingFields();
}
