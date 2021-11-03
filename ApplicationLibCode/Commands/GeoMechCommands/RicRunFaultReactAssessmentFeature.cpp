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

#include "RicRunFaultReactAssessmentFeature.h"

#include "RiaApplication.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaPreferencesGeoMech.h"
#include "RiaResultNames.h"

#include "RifFaultRAJsonWriter.h"
#include "RifFaultRAXmlWriter.h"

#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimFaultRAPostprocSettings.h"
#include "RimFaultRAPreprocSettings.h"
#include "RimFaultRASettings.h"
#include "RimFileSurface.h"
#include "RimGeoMechCase.h"
#include "RimOilField.h"
#include "RimProcess.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRunFaultReactAssessmentFeature::RicRunFaultReactAssessmentFeature()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInViewCollection* RicRunFaultReactAssessmentFeature::faultCollection()
{
    RimFaultInViewCollection* faultColl =
        dynamic_cast<RimFaultInViewCollection*>( caf::SelectionManager::instance()->selectedItem() );

    if ( faultColl ) return faultColl;

    RimFaultInView* selObj = dynamic_cast<RimFaultInView*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        if ( !selObj->name().startsWith( RiaResultNames::faultReactAssessmentPrefix() ) ) return nullptr;
        selObj->firstAncestorOrThisOfType( faultColl );
    }

    return faultColl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicRunFaultReactAssessmentFeature::faultIDFromName( QString faultName ) const
{
    int retval = -1;

    QString lookFor = RiaResultNames::faultReactAssessmentPrefix();
    QString name    = faultName;
    if ( !name.startsWith( lookFor ) ) return retval;

    name = name.mid( lookFor.length() );
    if ( name.size() == 0 ) return retval;

    bool bOK;
    retval = name.toInt( &bOK );
    if ( !bOK ) retval = -1;

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicRunFaultReactAssessmentFeature::selectedFaultID()
{
    int             retval = -1;
    RimFaultInView* selObj = dynamic_cast<RimFaultInView*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        return faultIDFromName( selObj->name() );
    }

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunFaultReactAssessmentFeature::runPostProcessing( int faultID, RimFaultRASettings* settings )
{
    RimFaultRAPostprocSettings postproc_settings;
    postproc_settings.initFromSettings( settings );

    QString outErrorText;
    if ( !RifFaultRAJSonWriter::writeToPostprocFile( faultID, &postproc_settings, outErrorText ) )
    {
        QMessageBox::warning( nullptr,
                              "Fault Reactivation Assessment Processing",
                              "Unable to write postproc parameter file! " + outErrorText );
        return false;
    }

    QString     command    = RiaPreferencesGeoMech::current()->geomechFRAPostprocCommand();
    QStringList parameters = postproc_settings.postprocCommandParameters( faultID );

    RimProcess process;
    process.setCommand( command );
    process.setParameters( parameters );

    addParameterFileForCleanUp( postproc_settings.postprocParameterFilename( faultID ) );

    if ( !process.execute() )
    {
        QMessageBox::critical( nullptr,
                               "Fault Reactivation Assessment Processing",
                               "Failed to run post processing command. Check log window for additional "
                               "information." );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactAssessmentFeature::addParameterFileForCleanUp( QString filename )
{
    m_parameterFilesToCleanUp.push_back( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactAssessmentFeature::cleanUpParameterFiles()
{
    if ( !RiaPreferencesGeoMech::current()->keepTemporaryFiles() )
    {
        for ( auto& filename : m_parameterFilesToCleanUp )
        {
            removeFile( filename );
        }
    }
    m_parameterFilesToCleanUp.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactAssessmentFeature::removeFile( QString filename )
{
    if ( QFile::exists( filename ) ) QFile::remove( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection* RicRunFaultReactAssessmentFeature::surfaceCollection()
{
    RimProject*           proj     = RimProject::current();
    RimSurfaceCollection* surfColl = proj->activeOilField()->surfaceCollection();

    if ( surfColl )
    {
        for ( auto& subColl : surfColl->subCollections() )
        {
            if ( subColl->collectionName() == "FaultRA" )
            {
                return subColl;
            }
        }

        // No FaultRA collection found, make one
        RimSurfaceCollection* fraCollection = new RimSurfaceCollection();
        fraCollection->setCollectionName( "FaultRA" );
        surfColl->addSubCollection( fraCollection );
        return fraCollection;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactAssessmentFeature::reloadSurfaces( RimFaultRASettings* settings )
{
    RimSurfaceCollection* surfColl = surfaceCollection();
    if ( !surfColl ) return;

    // get rid of any files removed by the processing
    surfColl->removeMissingFileSurfaces();

    bool showLegendInView = false;

    // ask the collection to reload the existing files
    surfColl->reloadSurfaces( surfColl->surfaces(), showLegendInView );

    // get all the files in the folder, skip the ones we alreday have
    QStringList newFiles;

    QDirIterator tsurfIt( settings->tsurfOutputDirectory(), { "*.ts" }, QDir::Files );
    while ( tsurfIt.hasNext() )
    {
        QString filename = tsurfIt.next();
        if ( surfColl->containsFileSurface( filename ) ) continue;
        newFiles << filename;
    }

    // import the new surfaces
    surfColl->importSurfacesFromFiles( newFiles, showLegendInView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactAssessmentFeature::runBasicProcessing( int faultID )
{
    RimFaultInViewCollection* coll = faultCollection();
    if ( coll == nullptr ) return;

    RimFaultRASettings* fraSettings = coll->faultRASettings();
    if ( fraSettings == nullptr ) return;

    caf::ProgressInfo runProgress( 3, "Running Basic Fault RA processing, please wait..." );

    {
        runProgress.setProgressDescription( "Macris calculate command." );
        QString paramfilename = fraSettings->basicParameterXMLFilename( faultID );

        RifFaultRAXmlWriter xmlwriter( fraSettings );
        QString             outErrorText;
        if ( !xmlwriter.writeCalculateFile( paramfilename, faultID, outErrorText ) )
        {
            QMessageBox::warning( nullptr,
                                  "Fault Reactivation Assessment Processing",
                                  "Unable to write parameter file! " + outErrorText );
            return;
        }

        addParameterFileForCleanUp( paramfilename );

        // remove any existing database file
        removeFile( fraSettings->basicMacrisDatabase() );

        // run the java macris program in calculate mode
        QString     command    = RiaPreferencesGeoMech::current()->geomechFRAMacrisCommand();
        QStringList parameters = fraSettings->basicMacrisParameters( faultID );

        RimProcess process;
        process.setCommand( command );
        process.setParameters( parameters );
        if ( !process.execute() )
        {
            QMessageBox::critical( nullptr,
                                   "Basic Fault Reactivation Assessment Processing",
                                   "Failed to run Macris calculate command. Check log window for additional "
                                   "information." );
            cleanUpParameterFiles();
            return;
        }

        runProgress.incrementProgress();
    }

    runProgress.setProgressDescription( "Generating surface results." );

    if ( runPostProcessing( faultID, fraSettings ) )
    {
        runProgress.incrementProgress();

        runProgress.setProgressDescription( "Importing surface results." );

        // reload output surfaces
        reloadSurfaces( fraSettings );
    }
    // delete parameter files
    cleanUpParameterFiles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunFaultReactAssessmentFeature::runAdvancedProcessing( int faultID )
{
    RimFaultInViewCollection* coll = faultCollection();
    if ( coll == nullptr ) return;

    RimFaultRASettings* fraSettings = coll->faultRASettings();
    if ( fraSettings == nullptr ) return;

    caf::ProgressInfo runProgress( 3, "Running Advanced Fault RA processing, please wait..." );

    runProgress.setProgressDescription( "Macris calibrate command." );
    QString paramfilename = fraSettings->basicParameterXMLFilename( faultID );

    RifFaultRAXmlWriter xmlwriter( fraSettings );
    QString             outErrorText;
    if ( !xmlwriter.writeCalculateFile( paramfilename, faultID, outErrorText ) )
    {
        QMessageBox::warning( nullptr,
                              "Fault Reactivation Assessment Processing",
                              "Unable to write parameter file! " + outErrorText );
        return;
    }

    QString paramfilename2 = fraSettings->advancedParameterXMLFilename( faultID );
    if ( !xmlwriter.writeCalibrateFile( paramfilename2, faultID, outErrorText ) )
    {
        QMessageBox::warning( nullptr,
                              "Fault Reactivation Assessment Processing",
                              "Unable to write calibrate parameter file! " + outErrorText );
        return;
    }

    addParameterFileForCleanUp( paramfilename );
    addParameterFileForCleanUp( paramfilename2 );

    // remove any existing database file
    removeFile( fraSettings->advancedMacrisDatabase() );

    // run the java macris program in calibrate mode
    QString     command    = RiaPreferencesGeoMech::current()->geomechFRAMacrisCommand();
    QStringList parameters = fraSettings->advancedMacrisParameters( faultID );

    RimProcess process;
    process.setCommand( command );
    process.setParameters( parameters );
    if ( !process.execute() )
    {
        QMessageBox::critical( nullptr,
                               "Advanced Fault Reactivation Assessment Processing",
                               "Failed to run Macris calibrate command. Check log window for additional information." );
        cleanUpParameterFiles();
        return;
    }

    runProgress.incrementProgress();

    runProgress.setProgressDescription( "Generating surface results." );

    if ( runPostProcessing( faultID, fraSettings ) )
    {
        runProgress.incrementProgress();

        runProgress.setProgressDescription( "Importing surface results." );

        // reload output surfaces
        reloadSurfaces( fraSettings );
    }

    // delete parameter files
    cleanUpParameterFiles();
}
