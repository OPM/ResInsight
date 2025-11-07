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

#include "RicNewFishbonesSubsAtMeasuredDepthFeature.h"

#include "RiaLogging.h"

#include "RicFishbonesCreateHelper.h"
#include "RicNewFishbonesSubsFeature.h"
#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMenu>

CAF_CMD_SOURCE_INIT( RicNewFishbonesSubsAtMeasuredDepthFeature, "RicNewFishbonesSubsAtMeasuredDepthFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::onActionTriggered( bool isChecked )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::setupActionLook( QAction* actionToSetup )
{
    RicFishbonesCreateHelper::setupFishbonesSubMenu( actionToSetup, "Create Fishbones at this Depth" );

    QMenu* subMenu = actionToSetup->menu();
    auto   actions = subMenu->actions();

    if ( actions.size() < 3 )
    {
        RiaLogging::error( "RicNewFishbonesSubsAtMeasuredDepthFeature::setupActionLook: Unexpected number of actions in submenu." );
        return;
    }

    connect( actions[0], &QAction::triggered, this, &RicNewFishbonesSubsAtMeasuredDepthFeature::onDrillingStandard );
    connect( actions[1], &QAction::triggered, this, &RicNewFishbonesSubsAtMeasuredDepthFeature::onDrillingExtended );
    connect( actions[2], &QAction::triggered, this, &RicNewFishbonesSubsAtMeasuredDepthFeature::onAcidJetting );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFishbonesSubsAtMeasuredDepthFeature::isCommandEnabled() const
{
    return RiuWellPathSelectionItem::wellPathSelectionItem() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::createFishbones( const RimFishbonesDefines::RicFishbonesSystemParameters& customParameters )
{
    RiuWellPathSelectionItem* wellPathSelItem = RiuWellPathSelectionItem::wellPathSelectionItem();
    CVF_ASSERT( wellPathSelItem );

    RimWellPath* wellPath = wellPathSelItem->m_wellpath;
    CVF_ASSERT( wellPath );

    RicFishbonesCreateHelper::createAndConfigureFishbones( wellPath->fishbonesCollection(), customParameters, wellPathSelItem->m_measuredDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::onDrillingStandard()
{
    createFishbones( RimFishbonesDefines::drillingStandardParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::onDrillingExtended()
{
    createFishbones( RimFishbonesDefines::drillingExtendedParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsAtMeasuredDepthFeature::onAcidJetting()
{
    createFishbones( RimFishbonesDefines::acidJettingParameters() );
}
