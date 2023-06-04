/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimResultSelectionUi.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaPorosityModel.h"
#include "RiaResultNames.h"

#include "RigCaseCellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultAddress.h"
#include "RimEclipseResultDefinition.h"
#include "RimGridCalculation.h"
#include "RimTools.h"

#include "RiuDragDrop.h"

#include "RimEclipseCaseTools.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"

CAF_PDM_SOURCE_INIT( RimResultSelectionUi, "RimResultSelectionUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimResultSelectionUi::RimResultSelectionUi()

{
    CAF_PDM_InitObject( "RimResultSelectionUi", ":/octave.png" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "Case", "Case" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseResult, "Result", "Result" );

    m_eclipseResult = new RimEclipseResultDefinition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultSelectionUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_eclipseCase );

    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup( "Result" );
    m_eclipseResult->uiOrdering( uiConfigName, *group1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultSelectionUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_eclipseCase )
    {
        m_eclipseResult->setEclipseCase( m_eclipseCase() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimResultSelectionUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        auto cases = RimEclipseCaseTools::eclipseCases();
        for ( auto* c : cases )
        {
            options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
        }
    }

    return options;
}
