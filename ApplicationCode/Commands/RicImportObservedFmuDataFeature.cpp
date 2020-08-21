/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicImportObservedFmuDataFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RicImportFormationNamesFeature.h"

#include "RifReaderFmuRft.h"

#include "RimFormationNames.h"
#include "RimObservedDataCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicImportObservedFmuDataFeature, "RicImportObservedFmuDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataFeature::selectObservedDataPathInDialog()
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "SUMMARY_CASE_DIR" );
    QString         directory  = RiuFileDialogTools::getExistingDirectory( nullptr,
                                                                  "Import Observed FMU Data Recursively from Directory",
                                                                  defaultDir );

    QStringList subDirsWithFmuData = RifReaderFmuRft::findSubDirectoriesWithFmuRftData( directory );
    if ( subDirsWithFmuData.empty() )
    {
        QString message = QString( "Could not find the file %1 in any sub-folder of %2.\nThis file is required for "
                                   "import of FMU data." )
                              .arg( RifReaderFmuRft::wellPathFileName() )
                              .arg( directory );

        RiaGuiApplication* guiApp = RiaGuiApplication::instance();
        if ( guiApp )
        {
            QMessageBox::warning( nullptr, "Import of Observed FMU Data", message );
        }

        RiaLogging::warning( message );

        return;
    }

    RimProject*                proj = app->project();
    RimObservedDataCollection* observedDataCollection =
        proj->activeOilField() ? proj->activeOilField()->observedDataCollection() : nullptr;
    if ( !observedDataCollection ) return;

    const RimObservedFmuRftData* importedData = nullptr;
    for ( const QString& subDir : subDirsWithFmuData )
    {
        importedData = observedDataCollection->createAndAddFmuRftDataFromPath( subDir );
        QDir    dir( subDir );
        QString layerZoneFile = dir.absoluteFilePath( RimFormationNames::layerZoneTableFileName() );
        if ( QFileInfo::exists( layerZoneFile ) )
        {
            QStringList fileNames;
            fileNames << layerZoneFile;
            RicImportFormationNamesFeature::importFormationFiles( fileNames );
        }
    }
    if ( importedData != nullptr )
    {
        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::selectAsCurrentItem( importedData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportObservedFmuDataFeature::isCommandEnabled()
{
    std::vector<RimObservedDataCollection*> selectionObservedDataCollection;
    caf::SelectionManager::instance()->objectsByType( &selectionObservedDataCollection );

    std::vector<RimObservedSummaryData*> selectionObservedData;
    caf::SelectionManager::instance()->objectsByType( &selectionObservedData );

    return ( selectionObservedDataCollection.size() > 0 || selectionObservedData.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataFeature::onActionTriggered( bool isChecked )
{
    selectObservedDataPathInDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ObservedDataFile16x16.png" ) );
    actionToSetup->setText( "Import Observed FMU Data" );
}
