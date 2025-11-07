/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicFishbonesCreateHelper.h"

#include "RicNewFishbonesSubsFeature.h"
#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuMainWindow.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbones* RicFishbonesCreateHelper::createAndConfigureFishbones( RimFishbonesCollection* fishbonesCollection,
                                                                     const RimFishbonesDefines::RicFishbonesSystemParameters& customParameters,
                                                                     double measuredDepth )
{
    if ( !fishbonesCollection ) return nullptr;

    RimWellPath* wellPath = fishbonesCollection->firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return nullptr;

    auto* obj = new RimFishbones;
    fishbonesCollection->appendFishbonesSubs( obj );

    obj->setSystemParameters( customParameters.lateralsPerSub,
                              customParameters.lateralLength,
                              customParameters.holeDiameter,
                              customParameters.buildAngle,
                              customParameters.icdsPerSub );

    obj->setMeasuredDepthAndCount( measuredDepth, customParameters.spacing, 13 );

    finalizeFishbonesCreation( obj, fishbonesCollection );

    return obj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFishbonesCreateHelper::setupFishbonesSubMenu( QAction* actionToSetup, const QString& actionText )
{
    auto icon = QIcon( ":/FishBoneGroup16x16.png" );
    actionToSetup->setIcon( icon );
    actionToSetup->setText( actionText );

    auto subMenu = new QMenu;

    {
        auto action = subMenu->addAction( "Drilling Standard" );
        action->setIcon( icon );
    }

    {
        auto action = subMenu->addAction( "Drilling Extended" );
        action->setIcon( icon );
    }
    {
        auto action = subMenu->addAction( "Acid Jetting" );
        action->setIcon( icon );
    }

    actionToSetup->setMenu( subMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFishbonesCreateHelper::finalizeFishbonesCreation( RimFishbones* fishbones, RimFishbonesCollection* fishbonesCollection )
{
    RicNewFishbonesSubsFeature::adjustWellPathScaling( fishbonesCollection );

    if ( auto wellPathCollection = RimTools::wellPathCollection() )
    {
        wellPathCollection->uiCapability()->updateConnectedEditors();
    }

    Riu3DMainWindowTools::selectAsCurrentItem( fishbones );

    RimProject* proj = RimProject::current();
    proj->reloadCompletionTypeResultsInAllViews();
}
