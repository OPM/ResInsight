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

#include "RicEclipseFormationFilterNewInViewFeature.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"
#include "RicEclipsePropertyFilterNewExec.h"

#include "RiaApplication.h"

#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEclipseFormationFilterNewInViewFeature, "RicEclipseFormationFilterNewInViewFeature" );

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection* activeViewFilterCollection()
{
    auto* eclView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeMainOrComparisonGridView() );
    return eclView ? eclView->eclipsePropertyFilterCollection() : nullptr;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseFormationFilterNewInViewFeature::isCommandEnabled() const
{
    auto* filterCollection = activeViewFilterCollection();
    return filterCollection && RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( filterCollection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseFormationFilterNewInViewFeature::onActionTriggered( bool isChecked )
{
    if ( auto* filterCollection = activeViewFilterCollection() )
    {
        caf::CmdExecCommandManager::instance()->processExecuteCommand( new RicEclipsePropertyFilterNewExec( filterCollection, true ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseFormationFilterNewInViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Formations16x16.png" ) );
    actionToSetup->setText( "Formation Filter" );
}
