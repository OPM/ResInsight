////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicImportPolygonFileFeature.h"

#include "RiaApplication.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonFile.h"
#include "Polygons/RimPolygonTools.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportPolygonFileFeature, "RicImportPolygonFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportPolygonFileFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    auto fallbackPath = app->lastUsedDialogDirectory( "BINARY_GRID" );
    auto polygonPath  = app->lastUsedDialogDirectoryWithFallback( RimPolygonTools::polygonCacheName(), fallbackPath );

    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Polygons",
                                                                  polygonPath,
                                                                  "Polylines (*.csv *.dat *.pol);;Text Files (*.txt);;Polylines "
                                                                  "(*.dat);;Polylines (*.pol);;Polylines (*.csv);;All Files (*.*)" );

    if ( fileNames.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( RimPolygonTools::polygonCacheName(), QFileInfo( fileNames.last() ).absolutePath() );

    auto polygonCollection = RimTools::polygonCollection();

    RimPolygon* objectToSelect = nullptr;

    for ( const auto& filename : fileNames )
    {
        auto newPolygonFile = new RimPolygonFile();
        newPolygonFile->setFileName( filename );
        newPolygonFile->loadData();
        polygonCollection->addPolygonFile( newPolygonFile );

        if ( !newPolygonFile->polygons().empty() ) objectToSelect = newPolygonFile->polygons().front();
    }

    polygonCollection->uiCapability()->updateAllRequiredEditors();

    Riu3DMainWindowTools::setExpanded( objectToSelect );
    Riu3DMainWindowTools::selectAsCurrentItem( objectToSelect );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportPolygonFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Polygon" );
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
}
