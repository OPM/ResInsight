/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimcFaultInViewCollection.h"

#include "RimFaultDistanceResult.h"
#include "RimFaultDistanceResultCollection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFaultInViewCollection, RimcFaultInViewCollection_addFaultDistanceResult, "add_fault_distance_result" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFaultInViewCollection_addFaultDistanceResult::RimcFaultInViewCollection_addFaultDistanceResult( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Fault Distance Result", "", "", "Create a FAULTDIST cell result for a chosen subset of faults" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultName, "Name", "Name (default FAULTDIST<n> if empty)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_faults, "Faults", "Faults to include (empty = all)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFaultInViewCollection_addFaultDistanceResult::execute()
{
    auto* hostCollection = self<RimFaultInViewCollection>();
    if ( !hostCollection ) return std::unexpected( QString( "No fault collection" ) );

    auto* distanceCollection = hostCollection->faultDistanceResults();
    if ( !distanceCollection ) return std::unexpected( QString( "No fault distance results collection" ) );

    auto* newResult = distanceCollection->addResult();
    if ( !newResult ) return std::unexpected( QString( "Failed to create fault distance result" ) );

    if ( !m_resultName().isEmpty() ) newResult->setResultName( m_resultName() );

    std::vector<RimFaultInView*> selected = m_faults.ptrReferencedObjectsByType();
    if ( selected.empty() ) selected = hostCollection->faults();

    newResult->setSelectedFaults( selected );

    hostCollection->updateConnectedEditors();
    distanceCollection->updateConnectedEditors();

    return newResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcFaultInViewCollection_addFaultDistanceResult::classKeywordReturnedType() const
{
    return RimFaultDistanceResult::classKeywordStatic();
}
