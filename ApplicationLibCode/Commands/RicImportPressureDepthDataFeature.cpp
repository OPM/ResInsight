/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicImportPressureDepthDataFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RicImportFormationNamesFeature.h"

#include "RifReaderFmuRft.h"

#include "Formations/RimFormationNames.h"
#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimPressureDepthData.h"
#include "RimProject.h"

#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportPressureDepthDataFeature, "RicImportPressureDepthDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportPressureDepthDataFeature::selectPressureDepthDataPathInDialog()
{
    RiaApplication*            app                    = RiaApplication::instance();
    RimProject*                proj                   = app->project();
    RimObservedDataCollection* observedDataCollection = proj->activeOilField() ? proj->activeOilField()->observedDataCollection() : nullptr;
    if ( !observedDataCollection ) return;

    QString defaultDir = app->lastUsedDialogDirectory( "SUMMARY_CASE_DIR" );
    QString filterText = QString( "Text Files (*.txt);;All Files (*.*)" );

    RimPressureDepthData* firstImportedObject = nullptr;
    QStringList           filePaths = RiuFileDialogTools::getOpenFileNames( nullptr, "Import Pressure/Depth Data", defaultDir, filterText );
    for ( const QString& filePath : filePaths )
    {
        RimPressureDepthData* importedData = observedDataCollection->createAndAddPressureDepthDataFromPath( filePath );
        if ( !firstImportedObject && importedData ) firstImportedObject = importedData;
    }
    if ( firstImportedObject != nullptr )
    {
        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::selectAsCurrentItem( firstImportedObject );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportPressureDepthDataFeature::isCommandEnabled() const
{
    const auto selectionObservedDataCollection = caf::SelectionManager::instance()->objectsByType<RimObservedDataCollection>();
    const auto selectionObservedData           = caf::SelectionManager::instance()->objectsByType<RimObservedSummaryData>();

    return ( !selectionObservedDataCollection.empty() || !selectionObservedData.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportPressureDepthDataFeature::onActionTriggered( bool isChecked )
{
    selectPressureDepthDataPathInDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportPressureDepthDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ObservedDataFile16x16.png" ) );
    actionToSetup->setText( "Import Pressure Depth Data" );
}
