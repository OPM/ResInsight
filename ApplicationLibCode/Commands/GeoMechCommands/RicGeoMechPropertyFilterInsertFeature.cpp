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

#include "RicGeoMechPropertyFilterInsertFeature.h"

#include "RicGeoMechPropertyFilterFeatureImpl.h"
#include "RicGeoMechPropertyFilterInsertExec.h"

#include "RimGeoMechPropertyFilter.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT( RicGeoMechPropertyFilterInsertFeature, "RicGeoMechPropertyFilterInsertFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicGeoMechPropertyFilterInsertFeature::isCommandEnabled()
{
    std::vector<RimGeoMechPropertyFilter*> propertyFilters =
        RicGeoMechPropertyFilterFeatureImpl::selectedPropertyFilters();
    if ( propertyFilters.size() == 1 )
    {
        return RicGeoMechPropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( propertyFilters[0] );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterInsertFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimGeoMechPropertyFilter*> propertyFilters =
        RicGeoMechPropertyFilterFeatureImpl::selectedPropertyFilters();
    if ( propertyFilters.size() == 1 )
    {
        RicGeoMechPropertyFilterInsertExec* filterExec = new RicGeoMechPropertyFilterInsertExec( propertyFilters[0] );
        caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterInsertFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Insert Property Filter" );
}
