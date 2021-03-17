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

#include "RimEclipseResultCase.h"
#include "RimFaultRAPreprocSettings.h"
#include "RimGeoMechCase.h"

#include "RiaApplication.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "RifFaultRAJsonWriter.h"
#include "RifFaultRAXmlReader.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QDir>

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
    // get the case we should be working with
    std::vector<RimGeoMechCase*>       geomechCases = caf::selectedObjectsByTypeStrict<RimGeoMechCase*>();
    std::vector<RimEclipseResultCase*> eclipseCases = caf::selectedObjectsByTypeStrict<RimEclipseResultCase*>();
    if ( geomechCases.empty() && eclipseCases.empty() ) return;

    // step 1 - get base directory for our work, should be a new, empty folder somewhere
    QString defaultDir =
        RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( "FAULT_REACT_ASSESSMENT" );
    QString baseDir = RiuFileDialogTools::getExistingDirectory( nullptr, tr( "Select Base Directory" ), defaultDir );
    if ( baseDir.isNull() ) return;
    RiaApplication::instance()->setLastUsedDialogDirectory( "FAULT_REACT_ASSESSMENT", baseDir );

    // step 2 - ask the user for the options we need in the preproc step
    RimFaultRAPreprocSettings frapSettings;
    if ( !geomechCases.empty() ) frapSettings.setGeoMechCase( geomechCases[0] );
    if ( !eclipseCases.empty() ) frapSettings.setEclipseCase( eclipseCases[0] );
    frapSettings.setOutputBaseDirectory( baseDir );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                 &frapSettings,
                                                 "Fault Reactivation Assessment Preprocessing",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    propertyDialog.resize( QSize( 400, 420 ) );
    if ( propertyDialog.exec() != QDialog::Accepted ) return;

    // step 3 - prepare output folder, if needed
    prepareDirectory( frapSettings.outputBaseDirectory(), frapSettings.cleanBaseDirectory() );

    QString errorText;

    // step 4 - write the preprocessing parameter file
    if ( !RifFaultRAJSonWriter::writeToFile( frapSettings, errorText ) )
    {
    }
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

    if ( !dir.exists() )
    {
        dir.mkpath( "." );
    }
    else if ( deleteExistingContent )
    {
        dir.setFilter( QDir::Files | QDir::Dirs | QDir::NoSymLinks );

        for ( auto& entry : dir.entryInfoList() )
        {
            qDebug() << entry.fileName();
            if ( entry.isDir() && entry.fileName() != "." && entry.fileName() != ".." )
                entry.dir().removeRecursively();
            else if ( entry.isFile() )
                QFile::remove( entry.absoluteFilePath() );
        }
        dir.mkpath( "." );
    }
}
