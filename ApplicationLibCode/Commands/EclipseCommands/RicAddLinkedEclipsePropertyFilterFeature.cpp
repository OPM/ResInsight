/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RicAddLinkedEclipsePropertyFilterFeature.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAddLinkedEclipsePropertyFilterFeature, "RicAddLinkedEclipsePropertyFilterFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAddLinkedEclipsePropertyFilterFeature::isCommandEnabled() const
{
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
void RicAddLinkedEclipsePropertyFilterFeature::onActionTriggered( bool isChecked )
{
    auto filterCollections = RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilterCollections();

    if ( filterCollections.size() == 1 )
    {
        auto coll   = filterCollections[0];
        auto filter = coll->addFilterLinkedToCellResult();
        coll->updateAllRequiredEditors();

        Riu3DMainWindowTools::setExpanded( filter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddLinkedEclipsePropertyFilterFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Values.png" ) );
    actionToSetup->setText( "Add Property Filter Linked to Cell Result" );
}
