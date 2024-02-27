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

#include "RicNewPolygonFileFeature.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonFile.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindowTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicNewPolygonFileFeature, "RicNewPolygonFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFileFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );
    QStringList     fileNames =
        RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                              "Import Polygons",
                                              defaultDir,
                                              "Text Files (*.txt);;Polylines (*.dat);;Polylines (*.pol);;All Files (*.*)" );

    if ( fileNames.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "BINARY_GRID", QFileInfo( fileNames.last() ).absolutePath() );

    auto proj              = RimProject::current();
    auto polygonCollection = proj->activeOilField()->polygonCollection();

    RimPolygonFile* objectToSelect = nullptr;

    for ( const auto& filename : fileNames )
    {
        auto newPolygonFile = new RimPolygonFile();
        newPolygonFile->setFileName( filename );
        newPolygonFile->setName( "File Polygon " + QString::number( polygonCollection->polygonFiles().size() + 1 ) );
        newPolygonFile->loadData();
        polygonCollection->addPolygonFile( newPolygonFile );
        objectToSelect = newPolygonFile;
    }
    polygonCollection->uiCapability()->updateAllRequiredEditors();

    RiuPlotMainWindowTools::setExpanded( objectToSelect );
    RiuPlotMainWindowTools::selectAsCurrentItem( objectToSelect );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New File Polygon" );
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
}
