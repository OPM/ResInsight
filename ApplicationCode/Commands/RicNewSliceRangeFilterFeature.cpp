/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-  Statoil ASA
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

#include "RicNewSliceRangeFilterFeature.h"

#include "RiaApplication.h"

#include "RicRangeFilterFeatureImpl.h"
#include "RicRangeFilterNewExec.h"

#include "RimGridView.h"
#include "RimViewController.h"

#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>
#include <QList>
#include <QVariant>

CAF_CMD_SOURCE_INIT( RicNewSliceRangeFilter3dViewFeature, "RicNewSliceRangeFilter3dViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSliceRangeFilter3dViewFeature::isCommandEnabled()
{
    RimGridView* view = RiaApplication::instance()->activeGridView();
    if ( !view ) return false;

    RimGridView* viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();

    RimViewController* vc = viewOrComparisonView->viewController();
    if ( !vc ) return true;

    return ( !vc->isRangeFiltersControlled() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSliceRangeFilter3dViewFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();

    if ( !userData.isNull() && userData.type() == QVariant::List )
    {
        RimGridView* activeView           = RiaApplication::instance()->activeGridView();
        RimGridView* viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();

        RimCellRangeFilterCollection* rangeFilterCollection = viewOrComparisonView->rangeFilterCollection();

        RicRangeFilterNewExec* filterExec = new RicRangeFilterNewExec( rangeFilterCollection );

        QVariantList list = userData.toList();
        CAF_ASSERT( list.size() == 3 );

        int direction  = list[0].toInt();
        int sliceStart = list[1].toInt();
        int gridIndex  = list[2].toInt();

        filterExec->m_gridIndex = gridIndex;
        if ( direction == 0 )
        {
            filterExec->m_iSlice      = true;
            filterExec->m_iSliceStart = sliceStart;
        }
        else if ( direction == 1 )
        {
            filterExec->m_jSlice      = true;
            filterExec->m_jSliceStart = sliceStart;
        }
        else if ( direction == 2 )
        {
            filterExec->m_kSlice      = true;
            filterExec->m_kSliceStart = sliceStart;
        }

        caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
        activeView->setSurfaceDrawstyle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSliceRangeFilter3dViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Range.png" ) );
}
