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

#include "RimDataAnalyticsCollection.h"

#include "RimEclipseCase.h"
#include "RimFaultDistance.h"
#include "RimFaultDistanceCollection.h"
#include "RimWellTargetMapping.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimDataAnalyticsCollection, "DataAnalyticsCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataAnalyticsCollection::RimDataAnalyticsCollection()
{
    CAF_PDM_InitObject( "Data Analytics", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &m_faultDistanceCollection, "FaultDistanceCollection", "Fault Distance" );
    m_faultDistanceCollection = new RimFaultDistanceCollection;

    CAF_PDM_InitFieldNoDefault( &m_wellTargetMappings, "WellTargetMappings", "Well Target Mappings" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultDistanceCollection* RimDataAnalyticsCollection::faultDistanceCollection() const
{
    return m_faultDistanceCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataAnalyticsCollection::addWellTargetMapping( RimWellTargetMapping* wellTargetMapping )
{
    m_wellTargetMappings.push_back( wellTargetMapping );
    wellTargetMapping->updateResultDefinition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellTargetMapping*> RimDataAnalyticsCollection::wellTargetMappings() const
{
    return m_wellTargetMappings.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataAnalyticsCollection::isEmpty() const
{
    const bool hasFaultDistance = m_faultDistanceCollection() && !m_faultDistanceCollection()->isEmpty();
    return !hasFaultDistance && m_wellTargetMappings.empty();
}

//--------------------------------------------------------------------------------------------------
/// Show the analytics objects flattened directly under this folder (no intermediate collection node).
//--------------------------------------------------------------------------------------------------
void RimDataAnalyticsCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    if ( m_faultDistanceCollection() )
    {
        for ( auto* result : m_faultDistanceCollection()->items() )
        {
            uiTreeOrdering.add( result );
        }
    }

    for ( RimWellTargetMapping* wellTargetMapping : m_wellTargetMappings )
    {
        uiTreeOrdering.add( wellTargetMapping );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataAnalyticsCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewFaultDistanceResultFeature";
    menuBuilder << "RicNewWellTargetMappingFeature";
}

//--------------------------------------------------------------------------------------------------
/// The case-level "Data Analytics" folder is hidden from the tree while empty, so refresh the owner
/// case to re-run its defineUiTreeOrdering and remove the folder when the last item is removed.
//--------------------------------------------------------------------------------------------------
void RimDataAnalyticsCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateConnectedEditors();

    if ( auto* eclipseCase = firstAncestorOrThisOfType<RimEclipseCase>() ) eclipseCase->updateConnectedEditors();
}
