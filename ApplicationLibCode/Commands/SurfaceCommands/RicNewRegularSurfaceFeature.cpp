/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicNewRegularSurfaceFeature.h"

#include "RimCase.h"
#include "RimProject.h"
#include "RimRegularSurface.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfBoundingBox.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewRegularSurfaceFeature, "RicNewRegularSurfaceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRegularSurfaceFeature::onActionTriggered( bool isChecked )
{
    auto colls = caf::selectedObjectsByTypeStrict<RimSurfaceCollection*>();
    if ( colls.empty() ) return;

    auto surface = new RimRegularSurface;

    surface->setColor( cvf::Color3f::BLUE );
    surface->setOpacity( true, 0.6f );
    surface->setUserDescription( "Regular Surface" );
    surface->loadDataIfRequired();

    auto surfColl = colls.front();
    surfColl->addSurfacesAtIndex( -1, { surface } );
    surfColl->updateConnectedEditors();

    Riu3DMainWindowTools::selectAsCurrentItem( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRegularSurfaceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Create Regular Surface" );
}
