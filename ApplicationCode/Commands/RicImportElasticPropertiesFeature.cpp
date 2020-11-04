/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RicImportElasticPropertiesFeature.h"

#include "RiaApplication.h"

#include "RicElasticPropertiesImportTools.h"

#include "RimStimPlanModelTemplate.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportElasticPropertiesFeature, "RicImportElasticPropertiesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportElasticPropertiesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportElasticPropertiesFeature::onActionTriggered( bool isChecked )
{
    RimStimPlanModelTemplate* stimPlanModelTemplate =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimStimPlanModelTemplate>();
    if ( !stimPlanModelTemplate ) return;

    // Open dialog box to select files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "STIMPLAN_DIR" );
    QString         filePath   = RiuFileDialogTools::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(),
                                                            "Import Elastic Properties",
                                                            defaultDir,
                                                            "Elastic Properties (*.csv);;All Files (*.*)" );

    if ( filePath.isNull() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "STIMPLAN_DIR", QFileInfo( filePath ).absolutePath() );

    RicElasticPropertiesImportTools::importElasticPropertiesFromFile( filePath, stimPlanModelTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportElasticPropertiesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Elastic Properties" );
    // TODO: add icon?
    // actionToSetup->setIcon( QIcon( ":/ElasticProperties16x16.png" ) );
}
