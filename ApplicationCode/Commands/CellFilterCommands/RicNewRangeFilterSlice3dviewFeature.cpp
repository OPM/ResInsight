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

#include "RicNewRangeFilterSlice3dviewFeature.h"

#include "RiaApplication.h"
#include "RimCellFilterCollection.h"
#include "RimCellRangeFilter.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "Riu3DMainWindowTools.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewRangeFilterSlice3dviewFeature, "RicNewRangeFilterSlice3dviewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewRangeFilterSlice3dviewFeature::isCommandEnabled()
{
    RimGridView* view = RiaApplication::instance()->activeGridView();
    if ( !view ) return false;

    RimGridView* viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();

    RimViewController* vc = viewOrComparisonView->viewController();
    if ( !vc ) return true;

    return ( !vc->isCellFiltersControlled() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRangeFilterSlice3dviewFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();
    if ( userData.isNull() || userData.type() != QVariant::List ) return;

    RimProject* proj = RimProject::current();

    RimCase* sourceCase = nullptr;
    auto     allCases   = proj->allGridCases();
    if ( !allCases.empty() ) sourceCase = allCases.front();

    RimGridView* activeView           = RiaApplication::instance()->activeGridView();
    RimGridView* viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();

    // Get the selected Cell Filter Collection
    RimCellFilterCollection* filtColl = viewOrComparisonView->cellFilterCollection();

    // and the parameters from the user choice
    QVariantList list = userData.toList();
    CAF_ASSERT( list.size() == 3 );

    int direction  = list[0].toInt();
    int sliceStart = list[1].toInt();
    int gridIndex  = list[2].toInt();

    RimCellFilter* newFilter = filtColl->addNewCellRangeFilter( sourceCase, direction, sliceStart );
    if ( newFilter )
    {
        newFilter->setGridIndex( gridIndex );
        Riu3DMainWindowTools::selectAsCurrentItem( newFilter );
        activeView->setSurfaceDrawstyle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRangeFilterSlice3dviewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Range.png" ) );
}
