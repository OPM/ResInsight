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

#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
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
int RicRunFaultReactAssessmentFeature::selectedFaultID()
{
    int             retval = -1;
    RimFaultInView* selObj = dynamic_cast<RimFaultInView*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        QString lookFor = RiaResultNames::faultReactAssessmentPrefix();
        QString name    = selObj->name();
        if ( !name.startsWith( lookFor ) ) return retval;

        name = name.mid( lookFor.length() );
        if ( name.size() == 0 ) return retval;

        bool bOK;
        retval = name.toInt( &bOK );
        if ( !bOK ) retval = -1;
    }

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunFaultReactAssessmentFeature::runPostProcessing( int faultID, RimFaultRASettings* settings )
{
    QString outErrorText;
    if ( !RifFaultRAJSonWriter::writeToPostprocFile( faultID, settings, outErrorText ) )
    {
        QMessageBox::warning( nullptr,
                              "Fault Reactivation Assessment Processing",
                              "Unable to write postproc parameter file! " + outErrorText );
        return false;
    }

    QString     command    = RiaPreferencesGeoMech::current()->geomechFRAPostprocCommand();
    QStringList parameters = settings->postprocParameters( faultID );

    RimProcess process;
    process.setCommand( command );
    process.setParameters( parameters );

    addParameterFileForCleanUp( settings->postprocParameterFilename( faultID ) );

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
            if ( QFile::exists( filename ) ) QFile::remove( filename );
        }
    }
    m_parameterFilesToCleanUp.clear();
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

    // ask the collection to reload the existing files
    surfColl->reloadSurfaces( surfColl->surfaces() );

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
    bool showLegendInView = false;
    surfColl->importSurfacesFromFiles( newFiles, showLegendInView );
}
