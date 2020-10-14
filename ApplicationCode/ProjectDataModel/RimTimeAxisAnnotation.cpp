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

#include "RiaTimeTTools.h"

#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include "RiuQwtPlotCurve.h"

#include "RimEclipseCase.h"
#include "RimPlot.h"
#include "RimTools.h"
#include "RimViewWindow.h"

#include <cmath>

namespace caf
{
template <>
void caf::AppEnum<RimTimeAxisAnnotation::TimeAnnotationType>::setUp()
{
    addItem( RimTimeAxisAnnotation::TimeAnnotationType::TIME, "TIME", "Time" );
    addItem( RimTimeAxisAnnotation::TimeAnnotationType::TIME_RANGE, "TIME_RANGE", "Time Range" );
    setDefault( RimTimeAxisAnnotation::TimeAnnotationType::TIME );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimTimeAxisAnnotation, "RimTimeAxisAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTimeAxisAnnotation::RimTimeAxisAnnotation()
    : RimPlotAxisAnnotation()
{
    CAF_PDM_InitObject( "Equilibrium Annotation", ":/LeftAxis16x16.png", "", "" );

    m_value.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_annotationType, "AnnotationType", "AnnotationType", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_time, "Time", "Time", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_startTime, "StartTime", "StartTime", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_endTime, "EndTime", "EndTime", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTime( time_t time )
{
    m_time           = time;
    m_annotationType = TimeAnnotationType::TIME;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTimeRange( time_t startTime, time_t endTime )
{
    m_startTime      = startTime;
    m_endTime        = endTime;
    m_annotationType = TimeAnnotationType::TIME_RANGE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RimTimeAxisAnnotation::color() const
{
    if ( m_annotationType() == TimeAnnotationType::TIME )
    {
        return QColor( 255, 0, 0 );
    }
    else if ( m_annotationType() == TimeAnnotationType::TIME_RANGE )
    {
        return QColor( 0, 255, 0 );
    }
    return QColor( 0, 0, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimTimeAxisAnnotation::value() const
{
    return RiaTimeTTools::toDouble( m_time );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_value );

    if ( m_annotationType() == TimeAnnotationType::TIME )
    {
        uiOrdering.add( &m_time );
    }
    else if ( m_annotationType() == TimeAnnotationType::TIME_RANGE )
    {
        uiOrdering.add( &m_startTime );
        uiOrdering.add( &m_endTime );
    }

    uiOrdering.skipRemainingFields();
}
