/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicNewDepthSurfaceFeature.h"

#include "RimCase.h"
#include "RimDepthSurface.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfBoundingBox.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDepthSurfaceFeature, "RicNewDepthSurfaceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDepthSurfaceFeature::onActionTriggered( bool isChecked )
{
    auto colls = caf::selectedObjectsByTypeStrict<RimSurfaceCollection*>();
    if ( colls.empty() ) return;

    auto surface = new RimDepthSurface;

    auto allCases = RimProject::current()->allGridCases();
    if ( !allCases.empty() )
    {
        auto sourceCase = allCases.front();
        auto bb         = sourceCase->activeCellsBoundingBox();
        surface->setPlaneExtent( bb.min().x(), bb.min().y(), bb.max().x(), bb.max().y() );
        surface->setDepth( -bb.center().z() );

        bb.expand( 0.1 * bb.extent().z() );

        auto lowerDepthLimit = -bb.max().z();
        auto upperDepthLimit = -bb.min().z();
        surface->setDepthSliderLimits( lowerDepthLimit, upperDepthLimit );
    }
    surface->loadDataIfRequired();

    auto surfColl = colls.front();
    surfColl->addSurfacesAtIndex( -1, { surface } );
    surfColl->updateConnectedEditors();

    Riu3DMainWindowTools::selectAsCurrentItem( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDepthSurfaceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Create Depth Surface" );
}
