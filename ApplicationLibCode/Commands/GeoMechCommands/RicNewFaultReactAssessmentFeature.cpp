/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include "RicNewFaultReactAssessmentFeature.h"

#include "RiaApplication.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaPreferencesGeoMech.h"

#include "RifFaultRAJsonWriter.h"

#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimFaultRAPreprocSettings.h"
#include "RimFaultRASettings.h"
#include "RimGeoMechCase.h"
#include "RimProcess.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicNewFaultReactAssessmentFeature, "RicNewFaultReactAssessmentFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFaultReactAssessmentFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactAssessmentFeature::onActionTriggered( bool isChecked )
{
    RimFaultRAPreprocSettings frapSettings;

    // make sure the user has set up geomech/FRA things in preferences
    if ( !RiaPreferencesGeoMech::current()->validateFRASettings() )
    {
        QMessageBox::critical( nullptr,
                               "Fault Reactivation Assessment",
                               "Fault Reactivation Assessment has not been properly set up.\nPlease go to ResInsight "
                               "preferences and set/check the GeoMechanical settings." );
        return;
    }

    // ask user for preprocessing settings
    if ( !showSettingsGUI( frapSettings ) ) return;

    if ( frapSettings.cleanBaseDirectory() )
    {
        auto reply = QMessageBox::question( nullptr,
                                            QString( "Clean output directory" ),
                                            QString( "Are you sure you want to delete all files and subfolders in the "
                                                     "selected output directory?\n%1 " )
                                                .arg( frapSettings.outputBaseDirectory() ),
                                            QMessageBox::Yes | QMessageBox::No,
                                            QMessageBox::No );
        if ( reply == QMessageBox::No )
        {
            frapSettings.setCleanBaseDirectory( false );
        }
    }

    // make sure our work dir is there
    prepareDirectory( frapSettings.outputBaseDirectory(), frapSettings.cleanBaseDirectory() );

    // run the preproc steps needed
    if ( !runPreProc( frapSettings ) ) return;

    QStringList gridList;
    gridList << frapSettings.outputEclipseFilename();

    // load the new grid
    bool createView = true;
    int  caseId     = RiaImportEclipseCaseTools::openEclipseInputCaseFromFileNames( gridList, createView );
    if ( caseId < 0 )
    {
        QMessageBox::critical( nullptr, "Fault Reactivation Assessment", "Unable to load generated Eclipse grid." );
        return;
    }

    RimProject*          project = RiaApplication::instance()->project();
    RimEclipseInputCase* fraCase = dynamic_cast<RimEclipseInputCase*>( project->eclipseCaseFromCaseId( caseId ) );
    if ( fraCase == nullptr )
    {
        QMessageBox::critical( nullptr, "Fault Reactivation Assessment", "Unable to find generated Eclipse grid." );
        return;
    }

    RimEclipseView* view = getView( fraCase );
    if ( view == nullptr )
    {
        QMessageBox::critical( nullptr, "Fault Reactivation Assessment", "Unable to find view for generated Eclipse grid." );
        return;
    }

    if ( view->faultCollection() )
    {
        view->faultCollection()->enableFaultRA( true );
        view->faultCollection()->faultRASettings()->initFromPreprocSettings( &frapSettings, fraCase );
    }

    cleanUpParameterFiles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactAssessmentFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "New Fault Reactivation Assessment" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactAssessmentFeature::prepareDirectory( QString dirname, bool deleteExistingContent ) const
{
    QDir dir( dirname );

    if ( deleteExistingContent && dir.exists() )
    {
        dir.setFilter( QDir::Files | QDir::Dirs | QDir::NoSymLinks );

        for ( auto& entry : dir.entryInfoList() )
        {
            if ( entry.isDir() && entry.fileName() != "." && entry.fileName() != ".." )
                entry.dir().removeRecursively();
            else if ( entry.isFile() )
                QFile::remove( entry.absoluteFilePath() );
        }
    }

    dir.mkpath( "." );
    dir.mkpath( "Eclipse" );
    dir.mkpath( "Abaqus" );
    dir.mkpath( "tmp" );
    dir.mkpath( "tsurf" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFaultReactAssessmentFeature::showSettingsGUI( RimFaultRAPreprocSettings& settings )
{
    // get the case we should be working with
    std::vector<RimGeoMechCase*>       geomechCases = caf::selectedObjectsByTypeStrict<RimGeoMechCase*>();
    std::vector<RimEclipseResultCase*> eclipseCases = caf::selectedObjectsByTypeStrict<RimEclipseResultCase*>();
    if ( geomechCases.empty() && eclipseCases.empty() ) return false;

    // get base directory for our work, should be a new, empty folder somewhere
    QString defaultDir =
        RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( "FAULT_REACT_ASSESSMENT" );
    QString baseDir = RiuFileDialogTools::getExistingDirectory( nullptr, tr( "Select Working Directory" ), defaultDir );
    if ( baseDir.isNull() ) return false;
    RiaApplication::instance()->setLastUsedDialogDirectory( "FAULT_REACT_ASSESSMENT", baseDir );

    // ask the user for the options we need in the preproc step
    if ( !geomechCases.empty() ) settings.setGeoMechCase( geomechCases[0] );
    if ( !eclipseCases.empty() ) settings.setEclipseCase( eclipseCases[0] );
    settings.setOutputBaseDirectory( baseDir );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                 &settings,
                                                 "Fault Reactivation Assessment Preprocessing",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    if ( settings.geoMechSelected() )
        propertyDialog.resize( QSize( 520, 520 ) );
    else
        propertyDialog.resize( QSize( 520, 420 ) );

    // make sure we always have an eclipse case selected
    while ( true )
    {
        if ( propertyDialog.exec() != QDialog::Accepted ) break;
        if ( settings.eclipseCase() != nullptr ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFaultReactAssessmentFeature::runPreProc( RimFaultRAPreprocSettings& settings )
{
    caf::ProgressInfo runProgress( 2, "Running preprocessing, please wait..." );

    // is geomech enabled? If so, run preproc script to generate rpt files
    if ( settings.geoMechSelected() )
    {
        QString errorText;
        if ( !RifFaultRAJSonWriter::writeToPreprocFile( settings, errorText ) )
        {
            QMessageBox::warning( nullptr, "Fault Reactivation Assessment Preprocessing", errorText );
            return false;
        }

        runProgress.setProgressDescription( "Preproc script." );

        // run the python preprocessing script
        QString     command    = RiaPreferencesGeoMech::current()->geomechFRAPreprocCommand();
        QStringList parameters = settings.preprocParameterList();

        addParameterFileForCleanUp( settings.preprocParameterFilename() );

        RimProcess process;
        process.setCommand( command );
        process.setParameters( parameters );
        if ( !process.execute() )
        {
            QMessageBox::critical( nullptr,
                                   "Fault Reactivation Assessment Preprocessing",
                                   "Failed to run preprocessing script. Check log window for additional information." );
            return false;
        }
    }

    runProgress.incrementProgress();
    runProgress.setProgressDescription( "Macris prepare command." );

    // run the java macris program in prepare mode
    QString     command    = RiaPreferencesGeoMech::current()->geomechFRAMacrisCommand();
    QStringList parameters = settings.macrisPrepareParameterList();

    RimProcess process;
    process.setCommand( command );
    process.setParameters( parameters );
    if ( !process.execute() )
    {
        QMessageBox::critical( nullptr,
                               "Fault Reactivation Assessment Preprocessing",
                               "Failed to run Macrix prepare command. Check log window for additional information." );
        return false;
    }

    runProgress.incrementProgress();

    return true;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicNewFaultReactAssessmentFeature::getView( RimEclipseInputCase* eCase )
{
    std::vector<RimEclipseView*> views;
    eCase->descendantsOfType( views );
    if ( views.size() > 0 ) return views[0];

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactAssessmentFeature::addParameterFileForCleanUp( QString filename )
{
    m_parameterFilesToCleanUp.push_back( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactAssessmentFeature::cleanUpParameterFiles()
{
    if ( !RiaPreferencesGeoMech::current()->keepTemporaryFiles() )
    {
        for ( auto& filename : m_parameterFilesToCleanUp )
        {
            if ( QFile::exists( filename ) ) QFile::remove( filename );
        }
    }
    m_parameterFilesToCleanUp.clear();
}
