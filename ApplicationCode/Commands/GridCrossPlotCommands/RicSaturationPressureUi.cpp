/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicSaturationPressureUi.h"

#include "RimTools.h"

#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RicSaturationPressureUi, "RicSaturationPressureUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSaturationPressureUi::RicSaturationPressureUi()
{
    CAF_PDM_InitObject( "RicSaturationPressureUi", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_caseToApply, "CaseToApply", "Case to Apply", "", "", "" );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaturationPressureUi::setSelectedCase( RimEclipseCase* eclipseCase )
{
    m_caseToApply = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicSaturationPressureUi::selectedCase() const
{
    return m_caseToApply();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicSaturationPressureUi::selectedTimeStep() const
{
    return m_timeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicSaturationPressureUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_caseToApply )
    {
        RimTools::caseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_caseToApply, &options );
    }

    return options;
}
