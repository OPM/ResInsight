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

#include "RimcEclipseView.h"

#include "RimEclipseView.h"
#include "RimFaultDistance.h"
#include "RimFaultDistanceCollection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseView, RimcEclipseView_addFaultDistance, "add_fault_distance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseView_addFaultDistance::RimcEclipseView_addFaultDistance( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Fault Distance", "", "", "Create a FAULTDIST cell result for a chosen subset of faults" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultName, "Name", "Name (default FAULTDIST<n> if empty)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_faults, "Faults", "Faults to include (empty = all)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseView_addFaultDistance::execute()
{
    auto* eclipseView = self<RimEclipseView>();
    if ( !eclipseView ) return std::unexpected( QString( "No view" ) );

    auto* distanceCollection = eclipseView->faultDistanceCollection();
    if ( !distanceCollection ) return std::unexpected( QString( "No fault distance results collection" ) );

    auto* newResult = distanceCollection->addResult();
    if ( !newResult ) return std::unexpected( QString( "Failed to create fault distance result" ) );

    if ( !m_resultName().isEmpty() ) newResult->setResultName( m_resultName() );

    std::vector<RimFaultInView*> selected = m_faults.ptrReferencedObjectsByType();
    if ( selected.empty() && eclipseView->faultCollection() )
    {
        // Default to all faults, but leave the ResInsight-generated faults unticked.
        selected = eclipseView->faultCollection()->faults();
        std::erase_if( selected, []( RimFaultInView* fault ) { return fault && fault->isGeneratedFault(); } );
    }

    newResult->setSelectedFaults( selected );

    // When created from Python, always trigger the calculation (the UI uses an explicit Generate button instead).
    newResult->compute();

    eclipseView->updateConnectedEditors();

    return newResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcEclipseView_addFaultDistance::classKeywordReturnedType() const
{
    return RimFaultDistance::classKeywordStatic();
}
