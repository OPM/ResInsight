/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-    Equinor ASA
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

#include "RicRunWellIntegrityAnalysisFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesGeoMech.h"

#include "RifWellIAFileWriter.h"

#include "RimGeoMechView.h"
#include "RimProcess.h"
#include "RimProject.h"
#include "RimWellIASettings.h"
#include "RimWellIASettingsCollection.h"
#include "RimWellPath.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RiuFileDialogTools.h"

#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicRunWellIntegrityAnalysisFeature, "RicRunWellIntegrityAnalysisFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunWellIntegrityAnalysisFeature::onActionTriggered( bool isChecked )
{
    RimWellIASettings* modelSettings =
        dynamic_cast<RimWellIASettings*>( caf::SelectionManager::instance()->selectedItem() );

    if ( modelSettings == nullptr ) return;

    const QString wiaTitle( "Well Integrity Analysis" );

    QString outErrorText;

    caf::ProgressInfo runProgress( 3, wiaTitle + " running , please wait..." );

    runProgress.setProgressDescription( "Writing input files." );

    if ( !modelSettings->extractModelData() )
    {
        QMessageBox::critical( nullptr,
                               wiaTitle,
                               "Unable to get necessary data from the defined model box. Is the model box center "
                               "outside the reservoir?" );
        return;
    }

    if ( !RifWellIAFileWriter::writeToJsonFile( *modelSettings, outErrorText ) )
    {
        QMessageBox::critical( nullptr, wiaTitle, outErrorText );
        return;
    }

    if ( !RifWellIAFileWriter::writeToCSVFile( *modelSettings, outErrorText ) )
    {
        QMessageBox::critical( nullptr, wiaTitle, outErrorText );
        return;
    }

    runProgress.incrementProgress();
    runProgress.setProgressDescription( "Running Abaqus modeling." );

    QString     command    = RiaPreferencesGeoMech::current()->geomechWIACommand();
    QStringList parameters = modelSettings->commandParameters();

    RimProcess process;
    process.setCommand( command );
    process.setParameters( parameters );

    if ( !process.execute() )
    {
        QMessageBox::critical( nullptr, wiaTitle, "Failed to run modeling. Check log window for additional information." );
        return;
    }

    runProgress.incrementProgress();
    runProgress.setProgressDescription( "Loading modeling results." );

    RiaApplication* app = RiaApplication::instance();
    if ( !app->openOdbCaseFromFile( modelSettings->outputHeatOdbFilename() ) )
    {
        QMessageBox::critical( nullptr,
                               wiaTitle,
                               "Failed to load modeling results from file \"" + modelSettings->outputHeatOdbFilename() +
                                   "\". Check log window for additional information." );
    }

    if ( !app->openOdbCaseFromFile( modelSettings->outputOdbFilename() ) )
    {
        QMessageBox::critical( nullptr,
                               wiaTitle,
                               "Failed to load modeling results from file \"" + modelSettings->outputOdbFilename() +
                                   "\". Check log window for additional information." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunWellIntegrityAnalysisFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/WellIntAnalysis.png" ) );
    actionToSetup->setText( "Run..." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunWellIntegrityAnalysisFeature::isCommandEnabled()
{
    return true;
}
