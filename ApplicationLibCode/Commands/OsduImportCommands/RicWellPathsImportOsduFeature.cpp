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

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaOsduConnector.h"
#include "RiaPreferences.h"
#include "RiaPreferencesOsdu.h"

#include "RigWellPath.h"

#include "RimFileWellPath.h"
#include "RimOilField.h"
#include "RimOsduWellPath.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathImport.h"

#include "RiuMainWindow.h"
#include "RiuWellImportWizard.h"

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

    // Update the UTM bounding box from the reservoir
    app->project()->computeUtmAreaOfInterest();

    QString wellPathsFolderPath = QStandardPaths::writableLocation( QStandardPaths::CacheLocation ) + QString( "/wellpaths/" );
    QDir::root().mkpath( wellPathsFolderPath );

    if ( !app->project()->wellPathImport() ) return;

    // Keep a copy of the import settings, and restore if cancel is pressed in the import wizard
    QString copyOfOriginalObject = app->project()->wellPathImport()->writeObjectToXmlString();

    if ( !app->preferences() ) return;

    RimProject* project = RimProject::current();
    if ( !project ) return;

    if ( project->oilFields.empty() ) return;

    RimOilField* oilField = project->activeOilField();
    if ( !oilField ) return;

    RiaOsduConnector* osduConnector = app->makeOsduConnector();

    RiuWellImportWizard wellImportwizard( wellPathsFolderPath, osduConnector, app->project()->wellPathImport(), RiuMainWindow::instance() );

    if ( QDialog::Accepted == wellImportwizard.exec() )
    {
        std::vector<RiuWellImportWizard::WellInfo> importedWells = wellImportwizard.importedWells();

        caf::ProgressInfo progress( importedWells.size(), "Importing wells from OSDU" );
        for ( auto w : importedWells )
        {
            auto task = progress.task( QString( "Importing well: %1" ).arg( w.name ) );

            auto [wellPathGeometry, errorMessage] =
                RimWellPathCollection::loadWellPathGeometryFromOsdu( osduConnector, w.wellboreTrajectoryId, w.datumElevation );
            if ( wellPathGeometry.notNull() )
            {
                auto wellPath = new RimOsduWellPath;
                wellPath->setName( w.name );
                wellPath->setWellId( w.wellId );
                wellPath->setWellboreId( w.wellboreId );
                wellPath->setWellboreTrajectoryId( w.wellboreTrajectoryId );
                wellPath->setDatumElevationFromOsdu( w.datumElevation );

                oilField->wellPathCollection->addWellPath( wellPath );

                wellPath->setWellPathGeometry( wellPathGeometry.p() );
            }
            else
            {
                RiaLogging::error( "Importing OSDU well failed: " + errorMessage );
            }

            oilField->wellPathCollection->updateConnectedEditors();
        }

        project->updateConnectedEditors();
        app->project()->scheduleCreateDisplayModelAndRedrawAllViews();
    }
    else
    {
        app->project()->wellPathImport()->readObjectFromXmlString( copyOfOriginalObject, caf::PdmDefaultObjectFactory::instance() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathsImportOsduFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Well Paths from &OSDU" );
    actionToSetup->setIcon( QIcon( ":/WellCollection.png" ) );
}
