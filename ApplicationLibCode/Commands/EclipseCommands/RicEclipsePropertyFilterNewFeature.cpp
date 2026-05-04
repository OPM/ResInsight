/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicEclipsePropertyFilterNewFeature.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"
#include "RicEclipsePropertyFilterNewExec.h"

#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEclipsePropertyFilterNewFeature, "RicEclipsePropertyFilterNewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterNewFeature::isCommandEnabled() const
{
    // Enabled if either a property-filter collection is selected, a combined filter (in either a
    // per-view or a case-level host) is selected, or a case-level data filter collection is
    // selected.
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() ) return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( combined.front() );

    auto dataColls = caf::selectedObjectsByTypeStrict<RimDataFilterCollection*>();
    if ( !dataColls.empty() ) return true;

    std::vector<RimEclipsePropertyFilterCollection*> filterCollections =
        RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilterCollections();
    if ( filterCollections.size() == 1 )
    {
        return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( filterCollections[0] );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewFeature::onActionTriggered( bool isChecked )
{
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() )
    {
        RimCombinedFilter* parent = combined.front();
        if ( parent->firstAncestorOrThisOfType<RimEclipsePropertyFilterCollection>() )
        {
            RicEclipsePropertyFilterFeatureImpl::addPropertyFilterToCombinedFilter( parent );
        }
        else
        {
            // Case-level combined filter: setCase propagation (via parent->addFilter →
            // child->setCase) binds the new filter's result definition to the eclipse case.
            auto* created = parent->addNewFilter<RimEclipsePropertyFilter>( []( RimEclipsePropertyFilter* /*f*/ ) {} );
            parent->updateConnectedEditors();
            if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        }
        return;
    }

    auto dataColls = caf::selectedObjectsByTypeStrict<RimDataFilterCollection*>();
    if ( !dataColls.empty() )
    {
        auto* created = dataColls.front()->addNewPropertyFilter();
        if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        return;
    }

    std::vector<RimEclipsePropertyFilterCollection*> filterCollections =
        RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilterCollections();
    if ( filterCollections.size() == 1 )
    {
        RicEclipsePropertyFilterNewExec* filterExec = new RicEclipsePropertyFilterNewExec( filterCollections[0] );
        caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Values.png" ) );
    actionToSetup->setText( "New Property Filter" );
}
