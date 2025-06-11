/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicNewOpmFlowJobFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesOpm.h"

#include "RimEclipseCase.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "Jobs/RimJobCollection.h"
#include "Jobs/RimOpmFlowJob.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QIcon>

CAF_CMD_SOURCE_INIT( RicNewOpmFlowJobFeature, "RicNewOpmFlowJobFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewOpmFlowJobFeature::isCommandEnabled() const
{
    return RiaPreferencesOpm::current()->validateFlowSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOpmFlowJobFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimEclipseCase*> selectedEclipseCases = caf::SelectionManager::instance()->objectsByType<RimEclipseCase>();

    QString inDataFile;
    if ( selectedEclipseCases.empty() )
    {
        inDataFile = inputDataFile();
        if ( inDataFile.isEmpty() ) return;
    }

    QString workDir = workingFolder();
    if ( workDir.isEmpty() ) return;

    auto job = new RimOpmFlowJob();

    if ( !inDataFile.isEmpty() )
    {
        job->setInputDataFile( inDataFile );
        job->setName( job->deckName() + " - Opm Flow Simulation" );
    }
    else
    {
        job->setEclipseCase( selectedEclipseCases[0] );
        job->setName( selectedEclipseCases[0]->caseUserDescription() + " - Opm Flow Simulation" );
    }

    job->setWorkingDirectory( workDir );

    auto jobColl = RimTools::jobCollection();
    jobColl->addNewJob( job );

    Riu3DMainWindowTools::selectAsCurrentItem( job );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOpmFlowJobFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/opm.png" ) );
    actionToSetup->setText( "New Opm Flow Simulation... " + RiaDefines::betaFeaturePostfix() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewOpmFlowJobFeature::workingFolder()
{
    // get base directory for our work, should be a new, empty folder somewhere
    const QString defaultDirName = "OPM_FLOW_MODELING";
    QString       defaultDir     = RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( defaultDirName );
    QString       baseDir =
        RiuFileDialogTools::getExistingDirectory( Riu3DMainWindowTools::mainWindowWidget(), "Select Simulation Output Directory", defaultDir );
    if ( baseDir.isNull() || baseDir.isEmpty() ) return "";
    RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirName, baseDir );

    return baseDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewOpmFlowJobFeature::inputDataFile()
{
    const QString defaultDirName = "OPM_FLOW_INPUT";
    QString       defaultDir     = RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( defaultDirName );

    QString filterText = QString( "Simulation Input Files (*.DATA);;All Files (*.*)" );

    QString fileName =
        RiuFileDialogTools::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(), "Select Input Data File", defaultDir, filterText );

    if ( fileName.isEmpty() ) return "";

    // Remember the path to next time
    RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirName, QFileInfo( fileName ).absolutePath() );

    return fileName;
}
