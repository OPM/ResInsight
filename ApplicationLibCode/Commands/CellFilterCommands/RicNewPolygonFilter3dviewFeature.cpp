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

#include "RicNewPolygonFilter3dviewFeature.h"

#include "RiaApplication.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimGridView.h"
#include "RimPolygonFilter.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPolygonFilter3dviewFeature, "RicNewPolygonFilter3dviewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPolygonFilter3dviewFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFilter3dviewFeature::onActionTriggered( bool isChecked )
{
    // Get the selected Cell Filter Collection
    RimGridView*             viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();
    RimCellFilterCollection* filtColl             = viewOrComparisonView->cellFilterCollection();

    // and the case to use
    RimCase* sourceCase = viewOrComparisonView->ownerCase();

    RimPolygonFilter* lastCreatedOrUpdated = filtColl->addNewPolygonFilter( sourceCase );
    if ( lastCreatedOrUpdated )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( lastCreatedOrUpdated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFilter3dviewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Polygon.png" ) );
    actionToSetup->setText( "Polygon Filter" );
}
