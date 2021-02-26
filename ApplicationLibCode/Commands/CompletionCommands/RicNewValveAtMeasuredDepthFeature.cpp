/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RicNewValveAtMeasuredDepthFeature.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RimFishbones.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathValve.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewValveAtMeasuredDepthFeature, "RicNewValveAtMeasuredDepthFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveAtMeasuredDepthFeature::onActionTriggered( bool isChecked )
{
    RiuWellPathSelectionItem* wellPathSelItem = wellPathSelectionItem();
    CVF_ASSERT( wellPathSelItem );

    RimWellPath* wellPath = wellPathSelItem->m_wellpath;
    CVF_ASSERT( wellPath );

    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return;

    RimPerforationInterval* perfInterval = dynamic_cast<RimPerforationInterval*>( wellPathSelItem->m_wellPathComponent );
    double                  measuredDepth = wellPathSelItem->m_measuredDepth;

    RimWellPathValve* valve = new RimWellPathValve;

    std::vector<RimWellPathValve*> existingValves = perfInterval->valves();
    valve->setName( QString( "Valve #%1" ).arg( existingValves.size() + 1 ) );

    RimProject* project;
    perfInterval->firstAncestorOrThisOfTypeAsserted( project );

    std::vector<RimValveTemplate*> allValveTemplates = project->allValveTemplates();
    if ( !allValveTemplates.empty() )
    {
        valve->setValveTemplate( allValveTemplates.front() );
    }
    perfInterval->addValve( valve );
    valve->setMeasuredDepthAndCount( measuredDepth, perfInterval->endMD() - measuredDepth, 1 );

    RimWellPathCollection* wellPathCollection = nullptr;
    wellPath->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleRedrawAffectedViews();

    Riu3DMainWindowTools::selectAsCurrentItem( valve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveAtMeasuredDepthFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ICDValve16x16.png" ) );
    actionToSetup->setText( "Create Valve at this Depth" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewValveAtMeasuredDepthFeature::isCommandEnabled()
{
    if ( wellPathSelectionItem() && dynamic_cast<RimPerforationInterval*>( wellPathSelectionItem()->m_wellPathComponent ) )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathSelectionItem* RicNewValveAtMeasuredDepthFeature::wellPathSelectionItem()
{
    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem*      selItem       = riuSelManager->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );

    RiuWellPathSelectionItem* wellPathItem = dynamic_cast<RiuWellPathSelectionItem*>( selItem );

    return wellPathItem;
}
