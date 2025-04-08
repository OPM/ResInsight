/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicNewWellPathFractureFeature.h"

#include "RiaApplication.h"

#include "RicFractureNameGenerator.h"

#include "Well/RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "Riu3DMainWindowTools.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellPathFractureFeature, "RicNewWellPathFractureFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFracture* RicNewWellPathFractureFeature::addFracture( gsl::not_null<RimWellPath*> wellPath, double measuredDepth )
{
    if ( !RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem( wellPath ) ) return nullptr;

    RimWellPathFractureCollection* fractureCollection = wellPath->fractureCollection();
    CVF_ASSERT( fractureCollection );

    if ( fractureCollection->allFractures().empty() )
    {
        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
        if ( activeView )
        {
            activeView->fractureColors()->setDefaultResultName();
        }
    }

    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );
    if ( !wellPathGeometry ) return nullptr;

    RimProject* project = RimProject::current();

    RimWellPathFracture* fracture = new RimWellPathFracture();
    fractureCollection->addFracture( fracture );

    fracture->setMeasuredDepth( measuredDepth );
    fracture->setFractureUnit( wellPath->unitSystem() );

    cvf::Vec3d positionAtWellpath = wellPathGeometry->interpolatedPointAlongWellPath( measuredDepth );
    fracture->setAnchorPosition( positionAtWellpath );

    fracture->setName( RicFractureNameGenerator::nameForNewFracture() );

    auto unitSet = wellPath->unitSystem();
    fracture->setFractureUnit( unitSet );

    RimFractureTemplate* fracDef = project->activeOilField()->fractureDefinitionCollection()->firstFractureOfUnit( unitSet );
    if ( fracDef )
    {
        fracture->setFractureTemplate( fracDef );
    }

    if ( project )
    {
        project->reloadCompletionTypeResultsInAllViews();
    }

    RimWellPathFractureCollection::updateAfterFractureIsAdded();

    // Update well path, as the completion collections are hidden if empty. This update will make sure that the folder
    // and completion is visible
    wellPath->updateAllRequiredEditors();

    Riu3DMainWindowTools::selectAsCurrentItem( fracture );
    return fracture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureFeature::onActionTriggered( bool isChecked )
{
    RimWellPathFractureCollection* fractureColl = RicNewWellPathFractureFeature::selectedWellPathFractureCollection();
    if ( !fractureColl ) return;

    auto wellPath = fractureColl->firstAncestorOrThisOfTypeAsserted<RimWellPath>();

    double defaultMeasuredDepth = wellPath->uniqueStartMD();
    RicNewWellPathFractureFeature::addFracture( wellPath, defaultMeasuredDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureSymbol16x16.png" ) );
    actionToSetup->setText( "Create Fracture" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathFractureFeature::isCommandEnabled() const
{
    return selectedWellPathFractureCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RicNewWellPathFractureFeature::selectedWellPathFractureCollection()
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() != 1u ) return nullptr;

    RimWellPathFractureCollection* objToFind = nullptr;

    caf::PdmUiItem* pdmUiItem = selectedItems.front();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        objToFind = objHandle->firstAncestorOrThisOfType<RimWellPathFractureCollection>();
    }

    if ( objToFind == nullptr )
    {
        auto wellPaths = caf::SelectionManager::instance()->objectsByType<RimWellPath>();
        if ( !wellPaths.empty() )
        {
            return wellPaths[0]->fractureCollection();
        }
        RimWellPathCompletions* completions = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCompletions>();
        if ( completions )
        {
            return completions->fractureCollection();
        }
    }

    return objToFind;
}
