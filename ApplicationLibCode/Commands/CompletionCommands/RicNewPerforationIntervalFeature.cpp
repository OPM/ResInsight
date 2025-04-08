/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewPerforationIntervalFeature.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "Riu3DMainWindowTools.h"

#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPerforationIntervalFeature, "RicNewPerforationIntervalFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPerforationIntervalFeature::isCommandEnabled() const
{
    return selectedPerforationCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPerforationIntervalFeature::onActionTriggered( bool isChecked )
{
    RimPerforationCollection* perforationCollection = selectedPerforationCollection();
    if ( perforationCollection == nullptr ) return;

    RimWellPath* wellPath = perforationCollection->firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return;

    RimPerforationInterval* perforationInterval = new RimPerforationInterval;
    perforationInterval->setStartAndEndMD( wellPath->uniqueStartMD(), wellPath->uniqueEndMD() );
    perforationInterval->setUnitSystemSpecificDefaults();

    perforationCollection->appendPerforation( perforationInterval );

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( !wellPathCollection ) return;

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleRedrawAffectedViews();

    Riu3DMainWindowTools::selectAsCurrentItem( perforationInterval );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPerforationIntervalFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/PerforationInterval16x16.png" ) );
    actionToSetup->setText( "Create Perforation Interval" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPerforationCollection* RicNewPerforationIntervalFeature::selectedPerforationCollection()
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() != 1u ) return nullptr;

    caf::PdmUiItem* pdmUiItem = selectedItems.front();

    RimPerforationCollection* perforationCollection = nullptr;
    caf::PdmObjectHandle*     objHandle             = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        perforationCollection = objHandle->firstAncestorOrThisOfType<RimPerforationCollection>();

        if ( perforationCollection ) return perforationCollection;

        RimWellPath* wellPath = dynamic_cast<RimWellPath*>( objHandle );
        if ( wellPath ) return wellPath->perforationIntervalCollection();

        RimWellPathCompletions* completions = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletions>();
        if ( completions ) return completions->perforationCollection();
    }
    return nullptr;
}
