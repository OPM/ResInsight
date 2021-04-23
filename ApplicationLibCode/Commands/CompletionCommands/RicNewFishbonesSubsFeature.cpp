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

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

#include <cmath>

CAF_CMD_SOURCE_INIT( RicNewFishbonesSubsFeature, "RicNewFishbonesSubsFeature" );

//---------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onActionTriggered( bool isChecked )
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT( fishbonesCollection );

    RimWellPath* wellPath;
    fishbonesCollection->firstAncestorOrThisOfTypeAsserted( wellPath );
    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return;

    RimFishbones* obj = new RimFishbones;
    fishbonesCollection->appendFishbonesSubs( obj );

    double wellPathTipMd = wellPath->uniqueEndMD();
    if ( wellPathTipMd != HUGE_VAL )
    {
        double startMd = wellPathTipMd - 150 - 100;
        if ( startMd < 100 ) startMd = 100;

        obj->setMeasuredDepthAndCount( startMd, 12.5, 13 );
    }

    RicNewFishbonesSubsFeature::adjustWellPathScaling( fishbonesCollection );

    RimWellPathCollection* wellPathCollection = nullptr;
    fishbonesCollection->firstAncestorOrThisOfType( wellPathCollection );
    if ( wellPathCollection )
    {
        wellPathCollection->uiCapability()->updateConnectedEditors();
    }

    RiuMainWindow::instance()->selectAsCurrentItem( obj );

    RimProject* proj;
    fishbonesCollection->firstAncestorOrThisOfTypeAsserted( proj );
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicNewFishbonesSubsFeature::selectedFishbonesCollection()
{
    std::vector<caf::PdmUiItem*> allSelectedItems;
    caf::SelectionManager::instance()->selectedItems( allSelectedItems );
    if ( allSelectedItems.size() != 1u ) return nullptr;

    RimFishbonesCollection* objToFind = nullptr;

    caf::PdmUiItem* pdmUiItem = allSelectedItems.front();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        objHandle->firstAncestorOrThisOfType( objToFind );
    }

    if ( objToFind == nullptr )
    {
        std::vector<RimWellPath*> wellPaths;
        caf::SelectionManager::instance()->objectsByType( &wellPaths );
        if ( !wellPaths.empty() )
        {
            return wellPaths[0]->fishbonesCollection();
        }
        RimWellPathCompletions* completions =
            caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletions>();
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
    actionToSetup->setIcon( QIcon( ":/FishBoneGroup16x16.png" ) );
    actionToSetup->setText( "Create Fishbones" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFishbonesSubsFeature::isCommandEnabled()
{
    if ( selectedFishbonesCollection() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::adjustWellPathScaling( RimFishbonesCollection* fishboneCollection )
{
    CVF_ASSERT( fishboneCollection );
    RimWellPathCollection* wellPathColl = nullptr;
    fishboneCollection->firstAncestorOrThisOfTypeAsserted( wellPathColl );

    if ( wellPathColl->wellPathRadiusScaleFactor > 0.05 )
    {
        wellPathColl->wellPathRadiusScaleFactor = 0.01;
        RiaLogging::info( "Radius scale of well paths is reduced to ensure the fish bone laterals are visible." );
    }
}
