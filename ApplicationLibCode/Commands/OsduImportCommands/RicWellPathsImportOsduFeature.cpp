/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicWellPathsImportOsduFeature.h"

#include "Cloud/RiaOsduConnector.h"
#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaPreferencesOsdu.h"

#include "RigWellPath.h"

#include "RimFileWellPath.h"
#include "RimOilField.h"
#include "RimOsduWellPath.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"
#include "RiuWellImportWizard.h"

#include "cafDataLoadController.h"
#include "cafProgressInfo.h"

#include "cvfObject.h"

#include <QAction>
#include <QDir>

CAF_CMD_SOURCE_INIT( RicWellPathsImportOsduFeature, "RicWellPathsImportOsduFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathsImportOsduFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();
    if ( !app->project() ) return;

    if ( !app->preferences() ) return;

    RimProject* project = RimProject::current();
    if ( !project ) return;

    if ( project->oilFields.empty() ) return;

    RimOilField* oilField = project->activeOilField();
    if ( !oilField ) return;

    RiaOsduConnector* osduConnector = app->makeOsduConnector();
    if ( !osduConnector )
    {
        RiaLogging::error( "Failed to create OSDU connector" );
        return;
    }

    RiuWellImportWizard wellImportwizard( osduConnector, RiuMainWindow::instance() );

    if ( QDialog::Accepted == wellImportwizard.exec() )
    {
        std::vector<RiuWellImportWizard::WellInfo> importedWells = wellImportwizard.importedWells();

        caf::ProgressInfoEventProcessingBlocker blocker;
        caf::ProgressInfo                       progress( importedWells.size(), "Importing wells from OSDU", false, true );
        int                                     colorIndex = 0;
        std::vector<RimOsduWellPath*>           newWells;
        for ( const auto& w : importedWells )
        {
            auto wellPath = new RimOsduWellPath;
            wellPath->setName( w.name );
            wellPath->setWellId( w.wellId );
            wellPath->setWellboreId( w.wellboreId );
            wellPath->setWellboreTrajectoryId( w.wellboreTrajectoryId );
            wellPath->setDatumElevationFromOsdu( w.datumElevation );
            wellPath->setWellPathColor( RiaColorTables::wellPathsPaletteColors().cycledColor3f( colorIndex++ ) );

            newWells.push_back( wellPath );
            oilField->wellPathCollection->addWellPath( wellPath );
        }

        const QString wellPathGeometryKeyword = "WELL_PATH_GEOMETRY";

        caf::DataLoadController* dataLoadController = caf::DataLoadController::instance();
        progress.setProgressDescription( QString( "Reading well path geometry." ) );
        for ( RimWellPath* wellPath : newWells )
        {
            dataLoadController->loadData( *wellPath, wellPathGeometryKeyword, progress );
        }
        dataLoadController->blockUntilDone( wellPathGeometryKeyword );

        oilField->wellPathCollection->updateConnectedEditors();

        project->updateConnectedEditors();
        app->project()->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathsImportOsduFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Well Paths from &OSDU" + RiaDefines::betaFeaturePostfix() );
    actionToSetup->setIcon( QIcon( ":/WellCollection.png" ) );
}
