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

#include "RicNewFishbonesSubsFeature.h"

#include "RicFishbonesCreateHelper.h"
#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "Well/RigWellPath.h"

#include "Rim3dView.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <cmath>

CAF_CMD_SOURCE_INIT( RicNewFishbonesSubsFeature, "RicNewFishbonesSubsFeature" );

//---------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onActionTriggered( bool isChecked )
{
    // Nothing to do here, handled by sub menu actions
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onDrillingStandard()
{
    createFishbones( RimFishbonesDefines::drillingStandardParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onDrillingExtended()
{
    createFishbones( RimFishbonesDefines::drillingExtendedParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onAcidJetting()
{
    createFishbones( RimFishbonesDefines::acidJettingParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::createFishbones( const RimFishbonesDefines::RicFishbonesSystemParameters& customParameters )
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT( fishbonesCollection );

    RimWellPath* wellPath = fishbonesCollection->firstAncestorOrThisOfTypeAsserted<RimWellPath>();

    double measuredDepth = 100;
    double wellPathTipMd = wellPath->uniqueEndMD();
    if ( wellPathTipMd != HUGE_VAL )
    {
        double startMd = wellPathTipMd - 150 - 100;
        if ( startMd < 100 ) startMd = 100;
        measuredDepth = startMd;
    }

    RicFishbonesCreateHelper::createAndConfigureFishbones( fishbonesCollection, customParameters, measuredDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicNewFishbonesSubsFeature::selectedFishbonesCollection()
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() != 1u ) return nullptr;

    RimFishbonesCollection* objToFind = nullptr;

    caf::PdmUiItem* pdmUiItem = selectedItems.front();

    auto* objHandle = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        objToFind = objHandle->firstAncestorOrThisOfType<RimFishbonesCollection>();
    }

    if ( objToFind == nullptr )
    {
        auto wellPaths = caf::SelectionManager::instance()->objectsByType<RimWellPath>();
        if ( !wellPaths.empty() )
        {
            return wellPaths[0]->fishbonesCollection();
        }
        RimWellPathCompletions* completions = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletions>();
        if ( completions )
        {
            return completions->fishbonesCollection();
        }
    }

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::setupActionLook( QAction* actionToSetup )
{
    RicFishbonesCreateHelper::setupFishbonesSubMenu( actionToSetup, "Create Fishbones" );

    QMenu* subMenu = actionToSetup->menu();
    auto   actions = subMenu->actions();
    if ( actions.size() < 3 )
    {
        RiaLogging::error( "RicNewFishbonesSubsAtMeasuredDepthFeature::setupActionLook: Unexpected number of actions in submenu." );
        return;
    }

    connect( actions[0], &QAction::triggered, this, &RicNewFishbonesSubsFeature::onDrillingStandard );
    connect( actions[1], &QAction::triggered, this, &RicNewFishbonesSubsFeature::onDrillingExtended );
    connect( actions[2], &QAction::triggered, this, &RicNewFishbonesSubsFeature::onAcidJetting );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFishbonesSubsFeature::isCommandEnabled() const
{
    return selectedFishbonesCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::adjustWellPathScaling( RimFishbonesCollection* fishboneCollection )
{
    CVF_ASSERT( fishboneCollection );
    RimWellPathCollection* wellPathColl = RimTools::wellPathCollection();

    if ( wellPathColl->wellPathRadiusScaleFactor > 0.05 )
    {
        wellPathColl->wellPathRadiusScaleFactor = 0.01;
        RiaLogging::info( "Radius scale of well paths is reduced to ensure the fish bone laterals are visible." );
    }
}
