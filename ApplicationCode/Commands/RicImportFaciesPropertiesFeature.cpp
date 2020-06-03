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

#include "RicImportFaciesPropertiesFeature.h"

#include "RiaApplication.h"

#include "RicFaciesPropertiesImportTools.h"

#include "RimFractureModel.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicImportFaciesPropertiesFeature, "RicImportFaciesPropertiesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFaciesPropertiesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesPropertiesFeature::onActionTriggered( bool isChecked )
{
    RimFractureModel* fractureModel = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimFractureModel>();
    if ( !fractureModel ) return;

    // Open dialog box to select files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "FACIES_DIR" );
    QString         filePath   = QFileDialog::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(),
                                                     "Import Facies Properties",
                                                     defaultDir,
                                                     "Facies Properties (*.csv);;All Files (*.*)" );

    if ( filePath.isNull() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "FACIES_DIR", QFileInfo( filePath ).absolutePath() );

    RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( filePath, fractureModel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesPropertiesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Facies Properties" );
    // TODO: add icon?
    // actionToSetup->setIcon( QIcon( ":/FaciesProperties16x16.png" ) );
}
