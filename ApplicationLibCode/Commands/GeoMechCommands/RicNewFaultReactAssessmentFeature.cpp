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
    // get the geomech case we should be working with
    std::vector<RimGeoMechCase*> cases = caf::selectedObjectsByTypeStrict<RimGeoMechCase*>();
    if ( cases.empty() ) return;

    // step 1 - get base directory for our work, should be a new, empty folder somewhere
    QString defaultDir =
        RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( "FAULT_REACT_ASSESSMENT" );
    QString baseDir = RiuFileDialogTools::getExistingDirectory( nullptr, tr( "Select Base Directory" ), defaultDir );
    if ( baseDir.isNull() ) return;
    RiaApplication::instance()->setLastUsedDialogDirectory( "FAULT_REACT_ASSESSMENT", baseDir );

    // step 2 - ask the user for the options we need in the preproc step
    RimFaultRAPreprocSettings fraSettings;
    fraSettings.setGeoMechCase( cases[0] );
    fraSettings.setOutputBaseDirectory( baseDir );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                 &fraSettings,
                                                 "Fault Reactivation Assessment Preprocessing",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    propertyDialog.resize( QSize( 300, 320 ) );
    if ( propertyDialog.exec() != QDialog::Accepted ) return;

    // step 3 - prepare output folder, if needed
    prepareDirectory( fraSettings.outputBaseDirectory(), fraSettings.cleanBaseDirectory() );

    QString errorText;

    // step 4 - write the preprocessing parameter file
    if ( !RifFaultRAJSonWriter::writeToFile( fraSettings, errorText ) )
    {
    }

    // test - read xml file
    RifFaultRAXmlReader reader( "D:/Data/FRA/default.xml" );

    if ( reader.parseFile( errorText ) )
    {
        for ( auto p : reader.parameters() )
        {
            qDebug() << p.first << " = " << p.second;
        }
    }
    else
    {
        qDebug() << errorText;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactAssessmentFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "Create Fault Reactivation Assessment" );
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
