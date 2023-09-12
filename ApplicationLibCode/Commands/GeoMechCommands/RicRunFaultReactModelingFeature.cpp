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
#include "RimProcess.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
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

    QString outErrorText;

    caf::ProgressInfo runProgress( 3, frmTitle + " running , please wait..." );

    runProgress.setProgressDescription( "Writing input files." );

    if ( model->fault() == nullptr )
    {
        QMessageBox::critical( nullptr, frmTitle, "A fault has not selected. Please check your model settings." );
        return;
    }

    // if ( !modelSettings->extractModelData() )
    //{
    //     QMessageBox::critical( nullptr,
    //                            frmTitle,
    //                            "Unable to get necessary data from the defined model box. Is the model box center "
    //                            "outside the reservoir?" );
    //     return;
    // }

    QString exportFile     = model->inputFilename();
    auto [result, errText] = RifFaultReactivationModelExporter::exportToFile( exportFile.toStdString(), *model->model() );

    if ( !result )
    {
        outErrorText = QString( "Failed to export INP model to file %1.\n\n%2" ).arg( exportFile ).arg( QString::fromStdString( errText ) );
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
    process.setParameters( parameters );

    if ( !process.execute() )
    {
        QMessageBox::critical( nullptr, frmTitle, "Failed to run modeling. Check log window for additional information." );
        return;
    }

    runProgress.incrementProgress();
    runProgress.setProgressDescription( "Loading modeling results." );

    RiaApplication* app = RiaApplication::instance();
    if ( !app->openOdbCaseFromFile( model->outputOdbFilename() ) )
    {
        QMessageBox::critical( nullptr,
                               frmTitle,
                               "Failed to load modeling results from file \"" + model->outputOdbFilename() +
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
