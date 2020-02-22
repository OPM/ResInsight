/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicNewPerforationIntervalAtMeasuredDepthFeature.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPerforationIntervalAtMeasuredDepthFeature, "RicNewPerforationIntervalAtMeasuredDepthFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPerforationIntervalAtMeasuredDepthFeature::onActionTriggered( bool isChecked )
{
    RiuWellPathSelectionItem* wellPathSelItem = wellPathSelectionItem();
    CVF_ASSERT( wellPathSelItem );

    RimWellPath* wellPath = wellPathSelItem->m_wellpath;
    CVF_ASSERT( wellPath );

    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return;

    RimPerforationInterval* perforationInterval = new RimPerforationInterval;
    double                  measuredDepth       = wellPathSelItem->m_measuredDepth;
    perforationInterval->setStartAndEndMD( measuredDepth, measuredDepth + 50 );

    wellPath->perforationIntervalCollection()->appendPerforation( perforationInterval );

    RimWellPathCollection* wellPathCollection = nullptr;
    wellPath->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleRedrawAffectedViews();

    Riu3DMainWindowTools::selectAsCurrentItem( perforationInterval );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathSelectionItem* RicNewPerforationIntervalAtMeasuredDepthFeature::wellPathSelectionItem()
{
    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem*      selItem       = riuSelManager->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );

    RiuWellPathSelectionItem* wellPathItem = dynamic_cast<RiuWellPathSelectionItem*>( selItem );

    return wellPathItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPerforationIntervalAtMeasuredDepthFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/PerforationIntervals16x16.png" ) );
    actionToSetup->setText( "Create Perforation Interval at this Depth" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPerforationIntervalAtMeasuredDepthFeature::isCommandEnabled()
{
    if ( wellPathSelectionItem() )
    {
        return true;
    }

    return false;
}
