/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicImportObservedDataFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryObservedDataFile.h"

#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportObservedDataFeature, "RicImportObservedDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedDataFeature::selectObservedDataFileInDialog()
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "INPUT_FILES" );
    QStringList     fileNames =
        RiuFileDialogTools::getOpenFileNames( nullptr, "Import Observed Data", defaultDir, "Observed Data (*.RSM *.txt *.csv);;All Files (*.*)" );

    if ( fileNames.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "INPUT_FILES", QFileInfo( fileNames.last() ).absolutePath() );

    RimProject*                proj                   = app->project();
    RimObservedDataCollection* observedDataCollection = proj->activeOilField() ? proj->activeOilField()->observedDataCollection() : nullptr;
    if ( !observedDataCollection ) return;

    RimObservedSummaryData* observedData = nullptr;

    for ( const QString& fileName : fileNames )
    {
        QString errorText;

        if ( fileName.endsWith( ".rsm", Qt::CaseInsensitive ) )
        {
            observedData = observedDataCollection->createAndAddRsmObservedSummaryDataFromFile( fileName, &errorText );
        }
        else if ( fileName.endsWith( ".txt", Qt::CaseInsensitive ) || fileName.endsWith( ".csv", Qt::CaseInsensitive ) )
        {
            bool useSavedFieldValuesInDialog = false;
            observedData =
                observedDataCollection->createAndAddCvsObservedSummaryDataFromFile( fileName, useSavedFieldValuesInDialog, &errorText );
        }
        else
        {
            errorText = "Not able to import file. Make sure '*.rsm' is used as extension if data is in RMS format "
                        "or '*.txt' or '*.csv' if data is in CSV format.";
        }

        if ( !errorText.isEmpty() )
        {
            RiaLogging::errorInMessageBox( nullptr, "Errors detected during import", errorText );
        }
    }

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( observedData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportObservedDataFeature::isCommandEnabled() const
{
    const auto selectionObservedDataCollection = caf::SelectionManager::instance()->objectsByType<RimObservedDataCollection>();
    const auto selectionObservedData           = caf::SelectionManager::instance()->objectsByType<RimObservedSummaryData>();

    return ( !selectionObservedDataCollection.empty() || !selectionObservedData.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedDataFeature::onActionTriggered( bool isChecked )
{
    RicImportObservedDataFeature::selectObservedDataFileInDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ObservedDataFile16x16.png" ) );
    actionToSetup->setText( "Import Observed Data" );
}
