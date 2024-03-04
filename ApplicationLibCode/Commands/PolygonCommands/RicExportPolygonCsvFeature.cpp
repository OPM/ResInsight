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

#include "RicExportPolygonCsvFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"
#include "Polygons/RimPolygonTools.h"

#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicExportPolygonCsvFeature, "RicExportPolygonCsvFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportPolygonCsvFeature::onActionTriggered( bool isChecked )
{
    auto sourcePolygon = caf::SelectionManager::instance()->selectedItemOfType<RimPolygon>();
    if ( !sourcePolygon )
    {
        auto sourcePolygonInView = caf::SelectionManager::instance()->selectedItemOfType<RimPolygonInView>();
        if ( sourcePolygonInView )
        {
            sourcePolygon = sourcePolygonInView->polygon();
        }
    }

    if ( !sourcePolygon ) return;

    auto app             = RiaGuiApplication::instance();
    auto fallbackPath    = app->lastUsedDialogDirectory( "BINARY_GRID" );
    auto polygonPath     = app->lastUsedDialogDirectoryWithFallback( RimPolygonTools::polygonCacheName(), fallbackPath );
    auto polygonFileName = polygonPath + "/" + sourcePolygon->name() + ".csv";

    auto fileName = RiuFileDialogTools::getSaveFileName( nullptr,
                                                         "Select File for Polygon Export to CSV",
                                                         polygonFileName,
                                                         "CSV Files (*.csv);;All files(*.*)" );

    if ( !RimPolygonTools::exportPolygonCsv( sourcePolygon, fileName ) )
    {
        RiaLogging::error( "Failed to export polygon to " + fileName );
    }
    else
    {
        RiaLogging::info( "Completed polygon export to " + fileName );
        app->setLastUsedDialogDirectory( RimPolygonTools::polygonCacheName(), QFileInfo( fileName ).absolutePath() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportPolygonCsvFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Polygon CSV" );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );
}
