/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicNewRangeFilterSliceFeature.h"

#include "RiaApplication.h"

#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimCellRangeFilter.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridView.h"

#include "Riu3DMainWindowTools.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

RicNewRangeFilterSliceFeature::RicNewRangeFilterSliceFeature( QString cmdText, QString radialText, int sliceDirection )
    : m_sliceDirection( sliceDirection )
    , m_sliceText( cmdText )
    , m_radialText( radialText )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRangeFilterSliceFeature::onActionTriggered( bool isChecked )
{
    RimCellFilterCollection* filterCollection = nullptr;
    RimCase*                 sourceCase       = nullptr;

    std::vector<RimCellFilterCollection*> colls = caf::selectedObjectsByTypeStrict<RimCellFilterCollection*>();
    if ( !colls.empty() )
    {
        filterCollection = colls.front();
        sourceCase       = filterCollection->firstAncestorOrThisOfTypeAsserted<Rim3dView>()->ownerCase();
    }

    if ( !filterCollection )
    {
        // Find filter collection for active view

        RimGridView* viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();
        if ( !viewOrComparisonView ) return;

        filterCollection = viewOrComparisonView->cellFilterCollection();
        sourceCase       = viewOrComparisonView->ownerCase();
    }

    if ( sourceCase && filterCollection )
    {
        int            gridIndex            = 0;
        RimCellFilter* lastCreatedOrUpdated = filterCollection->addNewCellRangeFilter( sourceCase, gridIndex, m_sliceDirection );
        if ( lastCreatedOrUpdated )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( lastCreatedOrUpdated );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRangeFilterSliceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Range.png" ) );

    if ( !m_radialText.isEmpty() && isMainGridRadial() )
    {
        actionToSetup->setText( m_radialText );
    }
    else
    {
        actionToSetup->setText( m_sliceText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewRangeFilterSliceFeature::isMainGridRadial()
{
    RimGridView* viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !viewOrComparisonView ) return false;

    auto eclipseView = dynamic_cast<RimEclipseView*>( viewOrComparisonView );
    if ( !eclipseView || !eclipseView->eclipseCase() ) return false;

    auto mainGrid = eclipseView->eclipseCase()->mainGrid();
    if ( !mainGrid ) return false;

    return mainGrid->isRadial();
}
