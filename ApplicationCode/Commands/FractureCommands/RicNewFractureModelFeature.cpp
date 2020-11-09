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
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
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
RimFractureModel* RicNewFractureModelFeature::addFractureModel( RimWellPath*           wellPath,
                                                                RimWellPathCollection* wellPathCollection,
                                                                double                 measuredDepth )
{
    CVF_ASSERT( wellPath );

    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return nullptr;

    RimFractureModelCollection* fractureModelCollection = wellPath->fractureModelCollection();
    CVF_ASSERT( fractureModelCollection );

    RimOilField* oilfield = nullptr;
    fractureModelCollection->firstAncestorOrThisOfType( oilfield );
    if ( !oilfield ) return nullptr;

    RimFractureModel* fractureModel = new RimFractureModel();
    fractureModelCollection->addFractureModel( fractureModel );

    QString fractureModelName = RicFractureNameGenerator::nameForNewFractureModel();
    fractureModel->setName( fractureModelName );

    RimProject* project = nullptr;
    fractureModelCollection->firstAncestorOrThisOfType( project );

    // Add a "fake" well path for thickess direction
    RimModeledWellPath* thicknessDirectionWellPath = new RimModeledWellPath;
    fractureModel->setThicknessDirectionWellPath( thicknessDirectionWellPath );

    std::vector<RimWellPath*> wellPaths = { thicknessDirectionWellPath };
    wellPathCollection->addWellPaths( wellPaths );

    if ( project )
    {
        project->reloadCompletionTypeResultsInAllViews();
        project->updateAllRequiredEditors();
    }

    if ( measuredDepth > 0.0 )
    {
        fractureModel->setMD( measuredDepth );
    }

    Riu3DMainWindowTools::selectAsCurrentItem( fractureModel );
    return fractureModel;
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

    RimWellPathCollection* wellPathCollection = nullptr;
    fractureColl->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

    RicNewFractureModelFeature::addFractureModel( wellPath, wellPathCollection );
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
    return selectedFractureModelCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCollection* RicNewFractureModelFeature::selectedFractureModelCollection()
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );
    if ( selectedItems.size() != 1u ) return nullptr;

    caf::PdmUiItem* pdmUiItem = selectedItems.front();

    RimFractureModelCollection* fractureModelCollection = nullptr;
    caf::PdmObjectHandle*       objHandle               = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        objHandle->firstAncestorOrThisOfType( fractureModelCollection );

        if ( fractureModelCollection ) return fractureModelCollection;

        RimWellPath* wellPath = dynamic_cast<RimWellPath*>( objHandle );
        if ( wellPath ) return wellPath->fractureModelCollection();

        RimWellPathCompletions* completions =
            caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletions>();
        if ( completions ) return completions->fractureModelCollection();
    }
    return nullptr;
}
