/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewFractureModelFeature.h"

#include "RiaApplication.h"

#include "RicFractureNameGenerator.h"

#include "RimFractureModel.h"
#include "RimFractureModelCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

#include "Riu3DMainWindowTools.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewFractureModelFeature, "RicNewFractureModelFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelFeature::addFractureModel( RimWellPath* wellPath )
{
    CVF_ASSERT( wellPath );

    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return;

    RimFractureModelCollection* fractureModelCollection = wellPath->fractureModelCollection();
    CVF_ASSERT( fractureModelCollection );

    RimFractureModel* fractureModel = new RimFractureModel();
    fractureModelCollection->addFractureModel( fractureModel );

    RimOilField* oilfield = nullptr;
    fractureModelCollection->firstAncestorOrThisOfType( oilfield );
    if ( !oilfield ) return;

    fractureModel->setName( RicFractureNameGenerator::nameForNewFractureModel() );

    RimProject* project = nullptr;
    fractureModelCollection->firstAncestorOrThisOfType( project );
    if ( project )
    {
        project->reloadCompletionTypeResultsInAllViews();
        project->updateAllRequiredEditors();
    }

    Riu3DMainWindowTools::selectAsCurrentItem( fractureModel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelFeature::onActionTriggered( bool isChecked )
{
    RimFractureModelCollection* fractureColl = RicNewFractureModelFeature::selectedFractureModelCollection();
    if ( !fractureColl ) return;

    RimWellPath* wellPath = nullptr;
    fractureColl->firstAncestorOrThisOfTypeAsserted( wellPath );

    RicNewFractureModelFeature::addFractureModel( wellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureSymbol16x16.png" ) );
    actionToSetup->setText( "Create Fracture Model" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFractureModelFeature::isCommandEnabled()
{
    if ( selectedFractureModelCollection() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCollection* RicNewFractureModelFeature::selectedFractureModelCollection()
{
    std::vector<caf::PdmUiItem*> allSelectedItems;
    caf::SelectionManager::instance()->selectedItems( allSelectedItems );
    if ( allSelectedItems.size() != 1u ) return nullptr;

    caf::PdmUiItem* pdmUiItem = allSelectedItems.front();

    RimFractureModelCollection* objToFind = nullptr;

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
            return wellPaths[0]->fractureModelCollection();
        }
        RimWellPathCompletions* completions =
            caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletions>();
        if ( completions )
        {
            return completions->fractureModelCollection();
        }
    }

    return objToFind;
}
