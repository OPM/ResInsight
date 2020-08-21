/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportFormationNamesFeature.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimGeoMechCase.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RigEclipseCaseData.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportFormationNamesFeature, "RicImportFormationNamesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RicImportFormationNamesFeature::importFormationFiles( const QStringList& fileNames )
{
    RimProject*                  proj        = RiaApplication::instance()->project();
    RimFormationNamesCollection* fomNameColl = proj->activeOilField()->formationNamesCollection();
    if ( !fomNameColl )
    {
        fomNameColl                                      = new RimFormationNamesCollection;
        proj->activeOilField()->formationNamesCollection = fomNameColl;
    }

    // For each file, find existing Formation names item, or create new
    RimFormationNames* formationNames = fomNameColl->importFiles( fileNames );
    fomNameColl->updateConnectedEditors();

    return formationNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFormationNamesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );

    QString filterText =
        QString( "Formation Names description File (*.lyr);;FMU Layer Zone Table(%1);;All Files (*.*)" )
            .arg( RimFormationNames::layerZoneTableFileName() );

    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Formation Names",
                                                                  defaultDir,
                                                                  filterText );

    if ( fileNames.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "BINARY_GRID", QFileInfo( fileNames.last() ).absolutePath() );

    // Find or create the FomationNamesCollection
    RimFormationNames* formationName = importFormationFiles( fileNames );

    if ( fileNames.size() > 1 ) return;

    if ( formationName )
    {
        RimProject* proj = RiaApplication::instance()->project();

        std::vector<RimCase*> cases;
        proj->allCases( cases );

        if ( !cases.empty() )
        {
            Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
            if ( activeView )
            {
                RimCase* ownerCase = activeView->ownerCase();
                if ( ownerCase )
                {
                    ownerCase->setFormationNames( formationName );
                    ownerCase->updateFormationNamesData();
                    ownerCase->updateConnectedEditors();
                }
            }
        }

        if ( formationName )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( formationName );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FormationCollection16x16.png" ) );
    actionToSetup->setText( "Import Formation Names" );
}
