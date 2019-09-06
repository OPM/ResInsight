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

#include "RicGeoMechPropertyFilterNewFeature.h"

#include "RicGeoMechPropertyFilterFeatureImpl.h"
#include "RicGeoMechPropertyFilterNewExec.h"

#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicGeoMechPropertyFilterNewFeature, "RicGeoMechPropertyFilterNewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicGeoMechPropertyFilterNewFeature::isCommandEnabled()
{
    std::vector<RimGeoMechPropertyFilterCollection*> filterCollections =
        RicGeoMechPropertyFilterFeatureImpl::selectedPropertyFilterCollections();
    if ( filterCollections.size() == 1 )
    {
        return RicGeoMechPropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( filterCollections[0] );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNewFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimGeoMechPropertyFilterCollection*> filterCollections =
        RicGeoMechPropertyFilterFeatureImpl::selectedPropertyFilterCollections();
    if ( filterCollections.size() == 1 )
    {
        RicGeoMechPropertyFilterNewExec* filterExec = new RicGeoMechPropertyFilterNewExec( filterCollections[0] );
        caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Values.png" ) );
    actionToSetup->setText( "New Property Filter" );
}
