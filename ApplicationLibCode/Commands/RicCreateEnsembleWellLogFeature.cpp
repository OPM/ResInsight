/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicCreateEnsembleWellLogFeature.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"

#include "ExportCommands/RicExportToLasFileFeature.h"
#include "RicCloseCaseFeature.h"
#include "RicCreateEnsembleWellLogUi.h"
#include "RicImportEnsembleWellLogsFeature.h"
#include "RicImportGeneralDataFeature.h"
#include "RicRecursiveFileSearchDialog.h"
#include "WellPathCommands/RicImportWellPaths.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEnsembleWellLogCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimcWellLogPlot.h"
#include "RimcWellLogPlotCollection.h"
#include "RimcWellLogTrack.h"

#include "Riu3DMainWindowTools.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuPropertyViewTabWidget.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmSettings.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicCreateEnsembleWellLogFeature, "RicCreateEnsembleWellLogFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogFeature::openDialogAndExecuteCommand()
{
    // Get the list of egrid files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );

    QString pathFilter( "*" );
    QString fileNameFilter( "*" );

    RicRecursiveFileSearchDialogResult result =
        RicRecursiveFileSearchDialog::runRecursiveSearchDialog( nullptr,
                                                                "Choose Eclipse Cases",
                                                                defaultDir,
                                                                pathFilter,
                                                                fileNameFilter,
                                                                { ".GRDECL", ".EGRID" } );

    if ( !result.ok || result.files.isEmpty() )
    {
        return;
    }

    // Use case data from first case
    RimEclipseCase* eclipseCase = loadEclipseCase( result.files[0] );
    if ( !eclipseCase ) return;

    RicCreateEnsembleWellLogUi* ui = RimProject::current()->dialogData()->createEnsembleWellLogUi();
    ui->setCaseData( eclipseCase->eclipseCaseData() );

    // Automatically selected the well path the dialog was triggered on (if any)
    auto* wellPath = caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>();
    if ( wellPath )
    {
        ui->setWellPathSource( RicCreateEnsembleWellLogUi::WellPathSource::PROJECT_WELLS );
        ui->setWellPathFromProject( wellPath );
    }

    RiuPropertyViewTabWidget propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                             ui,
                                             "Create Ensemble Well Log",
                                             ui->tabNames() );

    if ( propertyDialog.exec() == QDialog::Accepted && !ui->properties().empty() )
    {
        executeCommand( *ui, result.files.toStdList() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogFeature::executeCommand( const RicCreateEnsembleWellLogUi& ui,
                                                      const std::list<QString>&         fileNames )
{
    caf::ProgressInfo progress( fileNames.size(), "Creating ensemble well log" );

    std::vector<std::pair<QString, RiaDefines::ResultCatType>> properties = ui.properties();

    RimWellLogPlotCollection* plotCollection = RimProject::current()->mainPlotCollection()->wellLogPlotCollection();

    RimWellPath* wellPath = nullptr;

    if ( ui.wellPathSource() == RicCreateEnsembleWellLogUi::WellPathSource::FILE )
    {
        if ( ui.wellPathFilePath().isEmpty() ) return;

        // Load well path from file
        QStringList wellPathFilePaths;
        wellPathFilePaths << ui.wellPathFilePath();
        bool                      importGrouped = false;
        QStringList               errorMessages;
        std::vector<RimWellPath*> wellPaths =
            RicImportWellPaths::importWellPaths( wellPathFilePaths, importGrouped, &errorMessages );
        if ( wellPaths.empty() ) return;

        wellPath = wellPaths[0];
    }
    else
    {
        CAF_ASSERT( ui.wellPathSource() == RicCreateEnsembleWellLogUi::WellPathSource::PROJECT_WELLS );
        wellPath = ui.wellPathFromProject();
    }

    if ( !wellPath ) return;

    std::vector<RimWellLogPlot*> tmpPlotsToDelete;
    QStringList                  allLasFileNames;
    for ( const auto& fileName : fileNames )
    {
        auto task = progress.task( QString( "Extracting well log for %1" ).arg( fileName ) );

        // Load eclipse case
        RimEclipseCase* eclipseCase = loadEclipseCase( fileName );
        if ( !eclipseCase )
        {
            RiaLogging::error( QString( "Failed to load model from file: " ).arg( fileName ) );
            return;
        }

        // Create the well log plot
        RimWellLogPlot* wellLogPlot =
            RimcWellLogPlotCollection_newWellLogPlot::createWellLogPlot( plotCollection, wellPath, eclipseCase );

        // Create well log track
        QString          title = "Track";
        RimWellLogTrack* wellLogTrack =
            RimcWellLogPlot_newWellLogTrack::createWellLogTrack( wellLogPlot, eclipseCase, wellPath, title );

        // Create a well log curve for each property
        for ( const auto& property : properties )
        {
            QString                   propertyName       = property.first;
            RiaDefines::ResultCatType resultCategoryType = property.second;
            int                       timeStep           = ui.timeStep();
            RimcWellLogTrack_addExtractionCurve::addExtractionCurve( wellLogTrack,
                                                                     eclipseCase,
                                                                     wellPath,
                                                                     propertyName,
                                                                     resultCategoryType,
                                                                     timeStep );
        }

        {
            // Create missing directories
            QString   wellLogExportDirName = "lasexport";
            QFileInfo fi( fileName );
            QString   exportFolder = fi.absoluteDir().absolutePath() + QString( "/%1/" ).arg( wellLogExportDirName );

            if ( !fi.absoluteDir().exists( wellLogExportDirName ) )
            {
                if ( !fi.absoluteDir().mkpath( wellLogExportDirName ) )
                {
                    RiaLogging::error( QString( "Unable to create directory for well log export: " ).arg( exportFolder ) );
                    return;
                }
            }

            // Export to las file
            QString filePrefix          = "";
            bool    exportTvdRkb        = false;
            bool    capitalizeFileNames = false;
            bool    alwaysOverwrite     = true;
            double  resampleInterval    = 0.0;
            bool    convertCurveUnits   = false;

            std::vector<QString> lasFiles = RicExportToLasFileFeature::exportToLasFiles( exportFolder,
                                                                                         filePrefix,
                                                                                         wellLogPlot,
                                                                                         exportTvdRkb,
                                                                                         capitalizeFileNames,
                                                                                         alwaysOverwrite,
                                                                                         resampleInterval,
                                                                                         convertCurveUnits );
            for ( const auto& lasFile : lasFiles )
                allLasFileNames << lasFile;
        }

        tmpPlotsToDelete.push_back( wellLogPlot );

        RicCloseCaseFeature::deleteEclipseCase( eclipseCase );
    }

    for ( auto wlp : tmpPlotsToDelete )
    {
        // Hide window to avoid flickering
        wlp->setShowWindow( false );
        wlp->updateMdiWindowVisibility();

        plotCollection->removePlot( wlp );
        delete wlp;
    }

    if ( ui.autoCreateEnsembleWellLogs() )
    {
        std::vector<RimEnsembleWellLogs*> ensembleWellLogs =
            RicImportEnsembleWellLogsFeature::createEnsembleWellLogsFromFiles( allLasFileNames );

        for ( auto ensembleWellLog : ensembleWellLogs )
        {
            if ( ensembleWellLog )
            {
                RimEclipseCase* eclipseCase = nullptr;

                // Create the well log plot
                RimWellLogPlot* wellLogPlot =
                    RimcWellLogPlotCollection_newWellLogPlot::createWellLogPlot( plotCollection, wellPath, eclipseCase );

                // Create a track per property
                for ( const auto& property : properties )
                {
                    // Create well log track
                    cvf::Color3f color = RiaColorTables::normalPaletteColors().cycledColor3f( wellLogPlot->plotCount() );
                    QString      title = QString( "Track %1" ).arg( wellLogPlot->plotCount() );
                    RimWellLogTrack* wellLogTrack =
                        RimcWellLogPlot_newWellLogTrack::createWellLogTrack( wellLogPlot, eclipseCase, wellPath, title );
                    auto* ensembleWellLogCurveSet = new RimEnsembleWellLogCurveSet();
                    ensembleWellLogCurveSet->setEnsembleWellLogs( ensembleWellLog );
                    ensembleWellLogCurveSet->setColor( color );
                    ensembleWellLogCurveSet->setWellLogChannelName( property.first );
                    wellLogTrack->setEnsembleWellLogCurveSet( ensembleWellLogCurveSet );
                    ensembleWellLogCurveSet->loadDataAndUpdate( true );
                }

                wellLogPlot->updateConnectedEditors();

                RiuPlotMainWindowTools::showPlotMainWindow();
                RiuPlotMainWindowTools::selectAsCurrentItem( wellLogPlot );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicCreateEnsembleWellLogFeature::loadEclipseCase( const QString& fileName )
{
    QString   absolutePath = fileName;
    QFileInfo projectPathInfo( absolutePath );
    if ( !projectPathInfo.exists() )
    {
        QDir startDir( RiaApplication::instance()->startDir() );
        absolutePath = startDir.absoluteFilePath( fileName );
    }
    bool createView = false;
    bool createPlot = false;
    auto openResult = RicImportGeneralDataFeature::openEclipseFilesFromFileNames( QStringList( { absolutePath } ),
                                                                                  createPlot,
                                                                                  createView );

    if ( !openResult.createdCaseIds.empty() )
    {
        return RimProject::current()->eclipseCaseFromCaseId( openResult.createdCaseIds.front() );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateEnsembleWellLogFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogFeature::onActionTriggered( bool isChecked )
{
    openDialogAndExecuteCommand();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateEnsembleWellLogFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Ensemble Well Log..." );
}
