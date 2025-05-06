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

#include "RiaColorTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"

#include "RimProject.h"

#include "RiuGuiTheme.h"

#include <QDateTime>

#include <cmath>

CAF_PDM_SOURCE_INIT( RimTimeAxisAnnotation, "RimTimeAxisAnnotation" );

namespace internal
{

QString defaultDateTimeFormatString()
{
    auto dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                 RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );

    auto timeFormatString = RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                                                 RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );

    return QString( "%1 %2" ).arg( dateFormatString ).arg( timeFormatString );
}
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTimeAxisAnnotation::RimTimeAxisAnnotation()
    : RimPlotAxisAnnotation()
{
    CAF_PDM_InitObject( "Time Axis Annotation", ":/LeftAxis16x16.png" );

    m_value.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTimeAxisAnnotation* RimTimeAxisAnnotation::createTimeAnnotation( time_t time, const cvf::Color3f& color, const QString& dateTimeFormatString )
{
    RimTimeAxisAnnotation* annotation = new RimTimeAxisAnnotation();
    annotation->setTime( time, dateTimeFormatString );
    annotation->setColor( color );
    return annotation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTimeAxisAnnotation* RimTimeAxisAnnotation::createTimeRangeAnnotation( time_t              startTime,
                                                                         time_t              endTime,
                                                                         const cvf::Color3f& color,
                                                                         const QString&      dateTimeFormatString )
{
    RimTimeAxisAnnotation* annotation = new RimTimeAxisAnnotation();
    annotation->setTimeRange( startTime, endTime, dateTimeFormatString );
    annotation->setColor( color );
    return annotation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTime( time_t time, const QString& dateTimeFormatString )
{
    m_value = RiaTimeTTools::toDouble( time );

    QString formatString = dateTimeFormatString.isEmpty() ? internal::defaultDateTimeFormatString() : dateTimeFormatString;

    m_name = RiaQDateTimeTools::toStringUsingApplicationLocale( RiaQDateTimeTools::fromTime_t( time ), formatString );

    setAnnotationType( AnnotationType::LINE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTimeRange( time_t startTime, time_t endTime, const QString& dateTimeFormatString )
{
    m_rangeStart = RiaTimeTTools::toDouble( startTime );
    m_rangeEnd   = RiaTimeTTools::toDouble( endTime );

    QString formatString = dateTimeFormatString.isEmpty() ? internal::defaultDateTimeFormatString() : dateTimeFormatString;

    m_name = QString( "%0 - %1" )
                 .arg( RiaQDateTimeTools::toStringUsingApplicationLocale( RiaQDateTimeTools::fromTime_t( startTime ), formatString ) )
                 .arg( RiaQDateTimeTools::toStringUsingApplicationLocale( RiaQDateTimeTools::fromTime_t( endTime ), formatString ) );

    setAnnotationType( AnnotationType::RANGE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RimTimeAxisAnnotation::defaultColor( AnnotationType annotationType )
{
    if ( annotationType == AnnotationType::LINE )
    {
        return RiuGuiTheme::getColorByVariableName( "secondaryColor" ); // QColor(255, 0, 0);
    }
    else if ( annotationType == RimPlotAxisAnnotation::AnnotationType::RANGE )
    {
        return RiuGuiTheme::getColorByVariableName( "primaryColor" ); // QColor( 0, 0, 255 );
    }
    return RiuGuiTheme::getColorByVariableName( "textColor" ); // QColor(0, 0, 100);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setDefaultColor()
{
    setColor( RiaColorTools::fromQColorTo3f( defaultColor( annotationType() ) ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.03" ) )
    {
        setDefaultColor();
    }
}
