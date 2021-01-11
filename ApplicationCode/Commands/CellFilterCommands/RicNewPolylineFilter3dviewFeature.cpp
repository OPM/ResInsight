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

#include "RicNewPolylineFilter3dviewFeature.h"

#include "RiaApplication.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimGridView.h"
#include "RimPolylineFilter.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPolylineFilter3dviewFeature, "RicNewPolylineFilter3dviewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPolylineFilter3dviewFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolylineFilter3dviewFeature::onActionTriggered( bool isChecked )
{
    // Get the selected Cell Filter Collection
    RimGridView*             activeView           = RiaApplication::instance()->activeGridView();
    RimGridView*             viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();
    RimCellFilterCollection* filtColl             = viewOrComparisonView->cellFilterCollection();

    // and the case to use
    RimCase* sourceCase = viewOrComparisonView->ownerCase();

    RimPolylineFilter* lastCreatedOrUpdated = filtColl->addNewPolylineFilter( sourceCase );
    if ( lastCreatedOrUpdated )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( lastCreatedOrUpdated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolylineFilter3dviewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Polyline.png" ) );
    actionToSetup->setText( "Polyline Filter" );
}
