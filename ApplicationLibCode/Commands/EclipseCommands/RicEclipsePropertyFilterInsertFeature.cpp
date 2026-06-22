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

#include "RicEclipsePropertyFilterInsertFeature.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"
#include "RicEclipsePropertyFilterInsertExec.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT( RicEclipsePropertyFilterInsertFeature, "RicEclipsePropertyFilterInsertFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterInsertFeature::isCommandEnabled() const
{
    std::vector<RimEclipsePropertyFilter*> propertyFilters = RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilters();
    if ( propertyFilters.size() != 1 ) return false;

    RimEclipsePropertyFilter* propertyFilter = propertyFilters[0];

    // Insert places a sibling next to the selected filter in its property-filter collection. This is
    // only possible when the filter is a direct child of the collection - not when it is nested in a
    // combined filter, or hosted at case level with no property-filter collection ancestor.
    auto* propertyFilterCollection = propertyFilter->firstAncestorOrThisOfType<RimEclipsePropertyFilterCollection>();
    if ( !propertyFilterCollection ) return false;
    if ( propertyFilterCollection->propertyFiltersField().indexOf( propertyFilter ) >= propertyFilterCollection->propertyFiltersField().size() )
    {
        return false;
    }

    return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( propertyFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterInsertFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimEclipsePropertyFilter*> propertyFilters = RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilters();
    if ( propertyFilters.size() == 1 )
    {
        RicEclipsePropertyFilterInsertExec* filterExec = new RicEclipsePropertyFilterInsertExec( propertyFilters[0] );
        caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterInsertFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Values.png" ) );
    actionToSetup->setText( "Insert Property Filter" );
}
