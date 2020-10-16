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

CAF_PDM_SOURCE_INIT( RimTimeAxisAnnotation, "RimTimeAxisAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTimeAxisAnnotation::RimTimeAxisAnnotation()
    : RimPlotAxisAnnotation()
{
    CAF_PDM_InitObject( "Equilibrium Annotation", ":/LeftAxis16x16.png", "", "" );

    m_value.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_time, "Time", "Time", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_startTime, "StartTime", "StartTime", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_endTime, "EndTime", "EndTime", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTime( time_t time )
{
    m_time = RiaTimeTTools::toDouble( time );
    this->setAnnotationType( AnnotationType::LINE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::setTimeRange( time_t startTime, time_t endTime )
{
    m_rangeStart = RiaTimeTTools::toDouble( startTime );
    m_rangeEnd   = RiaTimeTTools::toDouble( endTime );
    this->setAnnotationType( AnnotationType::RANGE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RimTimeAxisAnnotation::color() const
{
    if ( annotationType() == AnnotationType::LINE )
    {
        return QColor( 255, 0, 0 );
    }
    else if ( annotationType() == RimPlotAxisAnnotation::AnnotationType::RANGE )
    {
        return QColor( 0, 0, 255 );
    }
    return QColor( 0, 0, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTimeAxisAnnotation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_value );

    if ( annotationType() == RimPlotAxisAnnotation::AnnotationType::LINE )
    {
        uiOrdering.add( &m_time );
    }
    else if ( annotationType() == RimPlotAxisAnnotation::AnnotationType::RANGE )
    {
        uiOrdering.add( &m_startTime );
        uiOrdering.add( &m_endTime );
    }

    uiOrdering.skipRemainingFields();
}
