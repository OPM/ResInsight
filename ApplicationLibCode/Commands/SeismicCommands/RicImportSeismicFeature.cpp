/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicImportSeismicFeature.h"

#include "RiaApplication.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportSeismicFeature, "RicImportSeismicFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportSeismicFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSeismicFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "SEISMIC_GRID" );
    QString         fileName   = RiuFileDialogTools::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(),
                                                            "Import Seismic",
                                                            defaultDir,
                                                            "Seismic files (*.zgy);;All Files (*.*)" );

    if ( fileName.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "SEISMIC_GRID", QFileInfo( fileName ).absolutePath() );

    auto  proj     = RimProject::current();
    auto& seisColl = proj->activeOilField()->seismicCollection();

    if ( !seisColl ) return;

    RimSeismicData* newData = seisColl->importSeismicFromFile( fileName );

    if ( newData )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( newData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSeismicFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Seismic16x16.png" ) );
    actionToSetup->setText( "Import Seismic" );
}
