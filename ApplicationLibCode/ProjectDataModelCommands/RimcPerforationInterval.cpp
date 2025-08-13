/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RimcPerforationInterval.h"

#include "RimEclipseCase.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathValve.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

#include <expected>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimPerforationInterval, RimcPerforationInterval_addValve, "AddValve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcPerforationInterval_addValve::RimcPerforationInterval_addValve( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add StimPlan Fracture", "", "", "Add StimPlan Fracture" );

    CAF_PDM_InitScriptableField( &m_startMd, "StartMd", 0.0, "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMd, "EndMd", 0.0, "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_valveCount, "ValveCount", 1, "Valve Count" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_template, "Template", "", "", "", "Valve Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcPerforationInterval_addValve::execute()
{
    auto perfInterval = self<RimPerforationInterval>();

    if ( m_startMd() > m_endMd() ) return std::unexpected( "Start MD must be smaller that end MD." );
    if ( m_valveCount() < 1 ) return std::unexpected( "Valve count must be larger than zero" );

    RimWellPathValve* valve = new RimWellPathValve;

    valve->setValveTemplate( m_template() );
    perfInterval->addValve( valve );

    double spacing = std::abs( m_startMd() - m_endMd() ) / m_valveCount();

    valve->setMeasuredDepthAndCount( m_startMd(), spacing, m_valveCount() );
    valve->applyValveLabelAndIcon();

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleRedrawAffectedViews();

    return valve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcPerforationInterval_addValve::classKeywordReturnedType() const
{
    return RimWellPathValve::classKeywordStatic();
}
