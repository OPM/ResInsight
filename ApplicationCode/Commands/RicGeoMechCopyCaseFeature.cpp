/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicGeoMechCopyCaseFeature.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiaApplication.h"
#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicGeoMechCopyCaseFeature, "RicGeoMechCopyCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicGeoMechCopyCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechCopyCaseFeature::onActionTriggered( bool isChecked )
{
    RimGeoMechModels* coll = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGeoMechModels>();
    if ( coll )
    {
        // get the cases
        std::vector<RimGeoMechCase*> cases = caf::selectedObjectsByTypeStrict<RimGeoMechCase*>();

        RimGeoMechCase* caseToSelect = nullptr;
        RiaApplication* app          = RiaApplication::instance();
        QString         defaultDir   = app->lastUsedDialogDirectory( "GEOMECH_MODEL" );

        for ( RimGeoMechCase* gmc : cases )
        {
            QString fileName = RiuFileDialogTools::getOpenFileName( nullptr,
                                                                    "Import Geo-Mechanical Model",
                                                                    defaultDir,
                                                                    "Abaqus results (*.odb)" );
            if ( fileName.isEmpty() ) break;

            defaultDir = QFileInfo( fileName ).absolutePath();
            app->setLastUsedDialogDirectory( "GEOMECH_MODEL", defaultDir );

            caseToSelect = coll->copyCase( gmc, fileName );
        }

        if ( caseToSelect )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( caseToSelect );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechCopyCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Copy.png" ) );
    actionToSetup->setText( "Copy and Replace Input" );
}
