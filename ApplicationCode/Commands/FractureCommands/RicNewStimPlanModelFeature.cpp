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

#include "RicNewStimPlanModelFeature.h"

#include "RiaApplication.h"

#include "RicFractureNameGenerator.h"

#include "RimEclipseView.h"
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "Riu3DMainWindowTools.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewStimPlanModelFeature, "RicNewStimPlanModelFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModel* RicNewStimPlanModelFeature::addStimPlanModel( RimWellPath*           wellPath,
                                                                RimWellPathCollection* wellPathCollection,
                                                                RimEclipseCase*        eclipseCase,
                                                                int                    timeStep,
                                                                double                 measuredDepth )
{
    CVF_ASSERT( wellPath );

    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return nullptr;

    RimStimPlanModelCollection* stimPlanModelCollection = wellPath->stimPlanModelCollection();
    CVF_ASSERT( stimPlanModelCollection );

    RimOilField* oilfield = nullptr;
    stimPlanModelCollection->firstAncestorOrThisOfType( oilfield );
    if ( !oilfield ) return nullptr;

    RimStimPlanModel* stimPlanModel = new RimStimPlanModel();
    stimPlanModelCollection->addStimPlanModel( stimPlanModel );

    stimPlanModel->setEclipseCase( eclipseCase );
    stimPlanModel->setTimeStep( timeStep );

    QString stimPlanModelName = RicFractureNameGenerator::nameForNewStimPlanModel();
    stimPlanModel->setName( stimPlanModelName );

    RimProject* project = nullptr;
    stimPlanModelCollection->firstAncestorOrThisOfType( project );

    // Add a "fake" well path for thickess direction
    RimModeledWellPath* thicknessDirectionWellPath = new RimModeledWellPath;
    stimPlanModel->setThicknessDirectionWellPath( thicknessDirectionWellPath );

    std::vector<RimWellPath*> wellPaths = {thicknessDirectionWellPath};
    wellPathCollection->addWellPaths( wellPaths, false );

    if ( project )
    {
        project->reloadCompletionTypeResultsInAllViews();
        project->updateAllRequiredEditors();
    }

    if ( measuredDepth > 0.0 )
    {
        stimPlanModel->setMD( measuredDepth );
    }

    Riu3DMainWindowTools::selectAsCurrentItem( stimPlanModel );
    return stimPlanModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelFeature::onActionTriggered( bool isChecked )
{
    RimStimPlanModelCollection* fractureColl = RicNewStimPlanModelFeature::selectedStimPlanModelCollection();
    if ( !fractureColl ) return;

    RimWellPath* wellPath = nullptr;
    fractureColl->firstAncestorOrThisOfTypeAsserted( wellPath );

    RimWellPathCollection* wellPathCollection = nullptr;
    fractureColl->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

    RimEclipseView* activeView  = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );
    RimEclipseCase* eclipseCase = nullptr;
    int             timeStep    = 0;
    if ( activeView )
    {
        eclipseCase = activeView->eclipseCase();
        timeStep    = activeView->currentTimeStep();
    }

    RicNewStimPlanModelFeature::addStimPlanModel( wellPath, wellPathCollection, eclipseCase, timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureSymbol16x16.png" ) );
    actionToSetup->setText( "Create StimPlan Model" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStimPlanModelFeature::isCommandEnabled()
{
    return selectedStimPlanModelCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCollection* RicNewStimPlanModelFeature::selectedStimPlanModelCollection()
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );
    if ( selectedItems.size() != 1u ) return nullptr;

    caf::PdmUiItem* pdmUiItem = selectedItems.front();

    RimStimPlanModelCollection* stimPlanModelCollection = nullptr;
    caf::PdmObjectHandle*       objHandle               = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        objHandle->firstAncestorOrThisOfType( stimPlanModelCollection );

        if ( stimPlanModelCollection ) return stimPlanModelCollection;

        RimWellPath* wellPath = dynamic_cast<RimWellPath*>( objHandle );
        if ( wellPath ) return wellPath->stimPlanModelCollection();

        RimWellPathCompletions* completions =
            caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletions>();
        if ( completions ) return completions->stimPlanModelCollection();
    }
    return nullptr;
}
