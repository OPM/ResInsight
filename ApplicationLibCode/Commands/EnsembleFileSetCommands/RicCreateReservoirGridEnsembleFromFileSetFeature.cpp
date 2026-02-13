/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicCreateReservoirGridEnsembleFromFileSetFeature.h"

#include "RiaLogging.h"

#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirGridEnsemble.h"

#include "Riu3DMainWindowTools.h"

#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicCreateReservoirGridEnsembleFromFileSetFeature, "RicCreateReservoirGridEnsembleFromFileSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateReservoirGridEnsembleFromFileSetFeature::isCommandEnabled() const
{
    return !caf::selectedObjectsByType<RimEnsembleFileSet*>().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateReservoirGridEnsembleFromFileSetFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimEnsembleFileSet*> selectedFileSets = caf::selectedObjectsByType<RimEnsembleFileSet*>();

    if ( selectedFileSets.empty() ) return;

    RimProject*               project         = RimProject::current();
    RimEclipseCaseCollection* eclipseCaseColl = project->activeOilField()->analysisModels();

    for ( RimEnsembleFileSet* fileSet : selectedFileSets )
    {
        // Get grid file paths from the file set
        QStringList gridFiles = fileSet->createPaths( ".EGRID" );
        if ( gridFiles.empty() )
        {
            gridFiles = fileSet->createPaths( ".GRID" );
        }

        // Filter out non-existing files
        QStringList existingGridFiles;
        for ( const QString& filePath : gridFiles )
        {
            if ( QFileInfo::exists( filePath ) )
            {
                existingGridFiles.append( filePath );
            }
            else
            {
                RiaLogging::warning( QString( "Grid file does not exist: %1" ).arg( filePath ) );
            }
        }
        gridFiles = existingGridFiles;

        if ( gridFiles.empty() )
        {
            RiaLogging::warning( QString( "No existing grid files found for ensemble '%1'" ).arg( fileSet->name() ) );
            continue;
        }

        // Always create RimReservoirGridEnsemble
        RiaLogging::info(
            QString( "Creating Reservoir Grid Ensemble for '%1' with %2 grid files." ).arg( fileSet->name() ).arg( gridFiles.size() ) );

        RimReservoirGridEnsemble* gridEnsemble = new RimReservoirGridEnsemble();
        gridEnsemble->setEnsembleFileSet( fileSet );
        gridEnsemble->createGridCasesFromEnsembleFileSet();

        project->assignIdToCaseGroup( gridEnsemble );
        eclipseCaseColl->reservoirGridEnsembles.push_back( gridEnsemble );

        gridEnsemble->loadDataAndUpdate();

        // Create view for first case if available
        auto allCases = gridEnsemble->cases();
        if ( !allCases.empty() )
        {
            RimEclipseView* view = gridEnsemble->addViewForCase( allCases[0] );
            if ( view )
            {
                view->loadDataAndUpdate();
            }
        }

        Riu3DMainWindowTools::selectAsCurrentItem( gridEnsemble );
    }

    eclipseCaseColl->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateReservoirGridEnsembleFromFileSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/GridCaseGroup16x16.png" ) );
    actionToSetup->setText( "Create Reservoir Grid Ensemble" );
}
