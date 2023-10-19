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
    createFishbones( RicNewFishbonesSubsFeature::drillingStandardParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onDrillingExtended()
{
    createFishbones( RicNewFishbonesSubsFeature::drillingExtendedParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::onAcidJetting()
{
    createFishbones( RicNewFishbonesSubsFeature::acidJettingParameters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFishbonesSubsFeature::createFishbones( const RicFishbonesSystemParameters& customParameters )
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    CVF_ASSERT( fishbonesCollection );

    RimWellPath* wellPath = fishbonesCollection->firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return;

    auto* obj = new RimFishbones;
    fishbonesCollection->appendFishbonesSubs( obj );

    obj->setSystemParameters( customParameters.lateralsPerSub,
                              customParameters.lateralLength,
                              customParameters.holeDiameter,
                              customParameters.buildAngle,
                              customParameters.icdsPerSub );

    double wellPathTipMd = wellPath->uniqueEndMD();
    if ( wellPathTipMd != HUGE_VAL )
    {
        double startMd = wellPathTipMd - 150 - 100;
        if ( startMd < 100 ) startMd = 100;

        obj->setMeasuredDepthAndCount( startMd, 12.5, 13 );
    }

    RicNewFishbonesSubsFeature::adjustWellPathScaling( fishbonesCollection );

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( wellPathCollection )
    {
        wellPathCollection->uiCapability()->updateConnectedEditors();
    }

    RiuMainWindow::instance()->selectAsCurrentItem( obj );

    RimProject* proj = RimProject::current();
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicFishbonesSystemParameters RicNewFishbonesSubsFeature::drillingStandardParameters()
{
    return { .lateralsPerSub = 3, .lateralLength = 11.0, .holeDiameter = 12.5, .buildAngle = 6.0, .icdsPerSub = 3 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicFishbonesSystemParameters RicNewFishbonesSubsFeature::drillingExtendedParameters()
{
    return { .lateralsPerSub = 3, .lateralLength = 18.0, .holeDiameter = 12.5, .buildAngle = 4.0, .icdsPerSub = 3 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicFishbonesSystemParameters RicNewFishbonesSubsFeature::acidJettingParameters()
{
    return { .lateralsPerSub = 4, .lateralLength = 12.0, .holeDiameter = 15.0, .buildAngle = 6.0, .icdsPerSub = 4 };
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

    auto* objHandle = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        objToFind = objHandle->firstAncestorOrThisOfType<RimFishbonesCollection>();
    }

    if ( objToFind == nullptr )
    {
        std::vector<RimWellPath*> wellPaths;
        caf::SelectionManager::instance()->objectsByType( &wellPaths );
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
    auto icon = QIcon( ":/FishBoneGroup16x16.png" );
    actionToSetup->setIcon( icon );
    actionToSetup->setText( "Create Fishbones" );

    auto subMenu = new QMenu;

    {
        auto action = subMenu->addAction( "Drilling Standard" );
        action->setIcon( icon );
        connect( action, &QAction::triggered, this, &RicNewFishbonesSubsFeature::onDrillingStandard );
    }

    {
        auto action = subMenu->addAction( "Drilling Extended" );
        action->setIcon( icon );
        connect( action, &QAction::triggered, this, &RicNewFishbonesSubsFeature::onDrillingExtended );
    }
    {
        auto action = subMenu->addAction( "Acid Jetting" );
        action->setIcon( icon );
        connect( action, &QAction::triggered, this, &RicNewFishbonesSubsFeature::onAcidJetting );
    }

    actionToSetup->setMenu( subMenu );
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
