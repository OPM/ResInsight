/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Eqinor ASA
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

#include "RimEclipseCaseEnsemble.h"

#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEclipseCaseEnsemble, "RimEclipseCaseEnsemble" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCaseEnsemble::RimEclipseCaseEnsemble()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Grid Ensemble", ":/GridCaseGroup16x16.png", "", "", "EclipseCaseEnsemble", "Grid Ensemble" );

    CAF_PDM_InitScriptableField( &m_groupId, "Id", -1, "Id" );
    m_groupId.uiCapability()->setUiReadOnly( true );
    m_groupId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitFieldNoDefault( &m_caseCollection, "CaseCollection", "Ensemble Cases" );

    m_caseCollection = new RimCaseCollection;
    m_caseCollection->uiCapability()->setUiName( "Cases" );
    m_caseCollection->uiCapability()->setUiIconFromResourceString( ":/Cases16x16.png" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCaseEnsemble::~RimEclipseCaseEnsemble()
{
    delete m_caseCollection;
    m_caseCollection = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::addCase( RimEclipseCase* reservoir )
{
    CVF_ASSERT( reservoir );

    m_caseCollection()->reservoirs().push_back( reservoir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseEnsemble::removeCase( RimEclipseCase* reservoir )
{
    if ( m_caseCollection()->reservoirs().count( reservoir ) == 0 )
    {
        return;
    }

    m_caseCollection()->reservoirs().removeChild( reservoir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCaseEnsemble::contains( RimEclipseCase* reservoir ) const
{
    CVF_ASSERT( reservoir );

    for ( size_t i = 0; i < m_caseCollection()->reservoirs().size(); i++ )
    {
        RimEclipseCase* rimReservoir = m_caseCollection()->reservoirs()[i];
        if ( reservoir->gridFileName() == rimReservoir->gridFileName() )
        {
            return true;
        }
    }

    return false;
}
