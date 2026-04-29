/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RicEclipseCombinedPropertyFilterNewFeature.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCombinedFilter.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEclipseCombinedPropertyFilterNewFeature, "RicEclipseCombinedPropertyFilterNewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseCombinedPropertyFilterNewFeature::isCommandEnabled() const
{
    // Enable on either the collection or a combined filter within the collection (for nesting).
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() && combined.front()->firstAncestorOrThisOfType<RimEclipsePropertyFilterCollection>() )
    {
        return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( combined.front() );
    }

    auto filterCollections = RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilterCollections();
    if ( filterCollections.size() == 1 )
    {
        return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( filterCollections[0] );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseCombinedPropertyFilterNewFeature::onActionTriggered( bool isChecked )
{
    // Nested combined filter: add the new one inside the currently selected combined filter.
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() && combined.front()->firstAncestorOrThisOfType<RimEclipsePropertyFilterCollection>() )
    {
        RimCombinedFilter* parent  = combined.front();
        RimCase*           srcCase = parent->firstAncestorOrThisOfTypeAsserted<Rim3dView>()->ownerCase();
        if ( srcCase )
        {
            RimCombinedFilter* created = parent->addNewFilter<RimCombinedFilter>( []( RimCombinedFilter* ) {} );
            parent->updateConnectedEditors();
            if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        }
        return;
    }

    auto filterCollections = RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilterCollections();
    if ( filterCollections.size() != 1 ) return;

    RimCombinedFilter* created = filterCollections[0]->addNewCombinedFilter();
    filterCollections[0]->updateConnectedEditors();
    if ( created )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( created );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseCombinedPropertyFilterNewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Values.png" ) );
    actionToSetup->setText( "New Combined Property Filter" );
}
