/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicImportFaciesFeature.h"

#include "RiaApplication.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "RicFaciesPropertiesImportTools.h"

#include "RimFractureModel.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportFaciesFeature, "RicImportFaciesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFaciesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesFeature::onActionTriggered( bool isChecked )
{
    RimFractureModel* fractureModel = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimFractureModel>();
    if ( !fractureModel ) return;

    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "STIMPLAN_DIR" );

    QString filterText = QString( "Roff ascii file (*.roff);;All Files (*.*)" );

    QString fileName = RiuFileDialogTools::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(),
                                                            "Import Facies",
                                                            defaultDir,
                                                            filterText );

    if ( fileName.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "STIMPLAN_DIR", QFileInfo( fileName ).absolutePath() );

    bool createColorLegend = true;
    RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( fileName, fractureModel, createColorLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesFeature::setupActionLook( QAction* actionToSetup )
{
    // TODO: add icon?
    // actionToSetup->setIcon( QIcon( ":/Formations16x16.png" ) );
    actionToSetup->setText( "Import Facies" );
}
