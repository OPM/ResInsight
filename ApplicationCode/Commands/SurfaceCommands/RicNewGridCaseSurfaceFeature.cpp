/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewGridCaseSurfaceFeature.h"

#include "RiaApplication.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewGridSurfaceFeature, "RicNewGridSurfaceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewGridSurfaceFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGridSurfaceFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    // Find or create the SurfaceCollection

    RimProject*           proj     = RiaApplication::instance()->project();
    RimSurfaceCollection* surfColl = proj->activeOilField()->surfaceCollection();

    if ( !surfColl )
    {
        surfColl                                  = new RimSurfaceCollection();
        proj->activeOilField()->surfaceCollection = surfColl;
        proj->updateConnectedEditors();
    }

    RimSurface* lastCreatedOrUpdated = surfColl->addGridCaseSurface( nullptr );

    if ( lastCreatedOrUpdated )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( lastCreatedOrUpdated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGridSurfaceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Create Grid Case Surfaces" );
}
