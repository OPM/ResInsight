/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023    Equinor ASA
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

#include "RicRunFaultReactModelingFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesGeoMech.h"

#include "RifFaultReactivationModelExporter.h"

#include "RimFaultReactivationModel.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProcess.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicRunFaultReactModelingFeature, "RicRunFaultReactModelingFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunFaultReactModelingFeature::isCommandEnabled() const
{
    return RiaPreferencesGeoMech::current()->validateFRMSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactModelingFeature::onActionTriggered( bool isChecked )
{
    RimFaultReactivationModel* model = dynamic_cast<RimFaultReactivationModel*>( caf::SelectionManager::instance()->selectedItem() );
    if ( model == nullptr ) return;

    const QString frmTitle( "Fault Reactivation Modeling" );

    caf::ProgressInfo runProgress( 3, frmTitle + " running , please wait..." );

    runProgress.setProgressDescription( "Writing input files." );

    auto [result, errText] = RifFaultReactivationModelExporter::exportToFile( *model );

    if ( !result )
    {
        QString outErrorText = QString( "Failed to export INP model to file %1.\n\n%2" )
                                   .arg( QString::fromStdString( model->inputFilename() ) )
                                   .arg( QString::fromStdString( errText ) );
        QMessageBox::critical( nullptr, frmTitle, outErrorText );
        return;
    }

    if ( RiaPreferencesGeoMech::current()->waitBeforeRun() )
    {
        runProgress.setProgressDescription( "Waiting for input file modifications." );

        QString infoText = "Input parameter files can now be found in the working folder:";
        infoText += " \"" + model->baseDir() + "\"\n";
        infoText += "\nClick OK to start the Abaqus modeling or Cancel to stop.";

        auto reply = QMessageBox::information( nullptr, frmTitle, infoText, QMessageBox::Ok | QMessageBox::Cancel );

        if ( reply != QMessageBox::Ok ) return;
    }

    runProgress.incrementProgress();
    runProgress.setProgressDescription( "Running Abaqus modeling." );

    QString     command    = RiaPreferencesGeoMech::current()->geomechFRMCommand();
    QStringList parameters = model->commandParameters();

    RimProcess process;
    process.setCommand( command );
    process.addParameters( parameters );

    if ( !process.execute() )
    {
        QMessageBox::critical( nullptr, frmTitle, "Failed to run modeling. Check log window for additional information." );
        return;
    }

    runProgress.incrementProgress();
    runProgress.setProgressDescription( "Loading modeling results." );

    for ( auto gCase : RimProject::current()->geoMechCases() )
    {
        if ( QString::fromStdString( model->outputOdbFilename() ) == gCase->gridFileName() )
        {
            gCase->reloadDataAndUpdate();
            auto& views = gCase->geoMechViews();
            if ( !views.empty() )
            {
                Riu3DMainWindowTools::selectAsCurrentItem( views[0] );
            }
            else
            {
                Riu3DMainWindowTools::selectAsCurrentItem( gCase );
            }
            return;
        }
    }

    RiaApplication* app = RiaApplication::instance();
    if ( !app->openOdbCaseFromFile( QString::fromStdString( model->outputOdbFilename() ) ) )
    {
        QMessageBox::critical( nullptr,
                               frmTitle,
                               "Failed to load modeling results from file \"" + QString::fromStdString( model->outputOdbFilename() ) +
                                   "\". Check log window for additional information." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactModelingFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "Run..." );
}
