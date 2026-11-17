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

#include "RicEclipseFormationFilterNewFeature.h"

#include "CellFilterCommands/RicCellFilterFeatureTools.h"
#include "RicEclipsePropertyFilterFeatureImpl.h"
#include "RicEclipsePropertyFilterNewExec.h"

#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimFormationFilter.h"

#include "Riu3DMainWindowTools.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEclipseFormationFilterNewFeature, "RicEclipseFormationFilterNewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseFormationFilterNewFeature::isCommandEnabled() const
{
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() ) return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( combined.front() );

    if ( RicCellFilterFeatureTools::selectedDataFilterCollection() ) return true;

    if ( auto* target = RicEclipsePropertyFilterFeatureImpl::resolveTargetPropertyFilterCollection() )
    {
        return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( target );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseFormationFilterNewFeature::onActionTriggered( bool isChecked )
{
    auto combined = caf::selectedObjectsByTypeStrict<RimCombinedFilter*>();
    if ( !combined.empty() )
    {
        RimCombinedFilter* parent = combined.front();
        if ( parent->firstAncestorOrThisOfType<RimEclipsePropertyFilterCollection>() )
        {
            RicEclipsePropertyFilterFeatureImpl::addPropertyFilterToCombinedFilter( parent, true );
        }
        else
        {
            auto* created = parent->addNewFilter<RimFormationFilter>( []( RimFormationFilter* filter ) { filter->setToFormationNames(); } );
            parent->updateConnectedEditors();
            if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        }
        return;
    }

    if ( auto* dataColl = RicCellFilterFeatureTools::selectedDataFilterCollection() )
    {
        auto* created = dataColl->addNewFormationFilter();
        if ( created ) Riu3DMainWindowTools::selectAsCurrentItem( created );
        return;
    }

    if ( auto* target = RicEclipsePropertyFilterFeatureImpl::resolveTargetPropertyFilterCollection() )
    {
        caf::CmdExecCommandManager::instance()->processExecuteCommand( new RicEclipsePropertyFilterNewExec( target, true ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseFormationFilterNewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Formations16x16.png" ) );
    actionToSetup->setText( "New Formation Filter" );
}
