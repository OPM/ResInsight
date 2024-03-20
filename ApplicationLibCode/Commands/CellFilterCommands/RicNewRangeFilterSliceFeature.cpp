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

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimCellRangeFilter.h"

#include "Riu3DMainWindowTools.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

RicNewRangeFilterSliceFeature::RicNewRangeFilterSliceFeature( QString cmdText, int sliceDirection )
    : m_sliceDirection( sliceDirection )
    , m_sliceText( cmdText )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRangeFilterSliceFeature::onActionTriggered( bool isChecked )
{
    // Find the selected Cell Filter Collection
    std::vector<RimCellFilterCollection*> colls = caf::selectedObjectsByTypeStrict<RimCellFilterCollection*>();
    if ( colls.empty() ) return;
    RimCellFilterCollection* filtColl = colls[0];

    // and the case to use
    RimCase* sourceCase = filtColl->firstAncestorOrThisOfTypeAsserted<Rim3dView>()->ownerCase();

    if ( sourceCase )
    {
        int            gridIndex            = 0;
        RimCellFilter* lastCreatedOrUpdated = filtColl->addNewCellRangeFilter( sourceCase, gridIndex, m_sliceDirection );
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
    actionToSetup->setText( m_sliceText );
}
