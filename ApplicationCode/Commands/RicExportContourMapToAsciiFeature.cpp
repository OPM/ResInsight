/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RicExportContourMapToAsciiFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RifTextDataTableFormatter.h"
#include "RimContourMapProjection.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechContourMapView.h"

#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicExportContourMapToAsciiFeature, "RicExportContourMapToAsciiFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportContourMapToAsciiFeature::isCommandEnabled()
{
    RimEclipseContourMapView* existingEclipseContourMap = caf::SelectionManager::instance()
                                                              ->selectedItemOfType<RimEclipseContourMapView>();
    RimGeoMechContourMapView* existingGeoMechContourMap = caf::SelectionManager::instance()
                                                              ->selectedItemOfType<RimGeoMechContourMapView>();
    return existingEclipseContourMap || existingGeoMechContourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToAsciiFeature::onActionTriggered( bool isChecked )
{
    RimContourMapProjection*  contourMapProjection      = nullptr;
    RimEclipseContourMapView* existingEclipseContourMap = caf::SelectionManager::instance()
                                                              ->selectedItemOfType<RimEclipseContourMapView>();
    RimGeoMechContourMapView* existingGeoMechContourMap = caf::SelectionManager::instance()
                                                              ->selectedItemOfType<RimGeoMechContourMapView>();
    CAF_ASSERT( existingEclipseContourMap || existingGeoMechContourMap );

    QString contourMapName;
    if ( existingEclipseContourMap )
    {
        contourMapProjection = existingEclipseContourMap->contourMapProjection();
        contourMapName       = existingEclipseContourMap->createAutoName();
    }
    else if ( existingGeoMechContourMap )
    {
        contourMapProjection = existingGeoMechContourMap->contourMapProjection();
        contourMapName       = existingGeoMechContourMap->createAutoName();
    }

    CAF_ASSERT( contourMapProjection );

    RiaGuiApplication* app = RiaGuiApplication::instance();
    CAF_ASSERT( app && "Must be gui mode" );
    QString startPath = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "CONTOUR_EXPORT" );

    QString fileBaseName = caf::Utils::makeValidFileBasename( contourMapName );

    startPath = startPath + "/" + fileBaseName + ".txt";

    QString fileName = QFileDialog::getSaveFileName( nullptr,
                                                     tr( "Export Contour Map To Text File" ),
                                                     startPath,
                                                     tr( "Text file (*.txt);;All files(*.*)" ) );

    if ( fileName.isEmpty() )
    {
        return;
    }

    QFile exportFile( fileName );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        RiaLogging::error( QString( "Export Contour Map as Text : Could not open the file: %1" ).arg( fileName ) );
        return;
    }

    cvf::Vec2ui numVerticesIJ = contourMapProjection->numberOfVerticesIJ();

    QString                   tableText;
    QTextStream               stream( &exportFile );
    RifTextDataTableFormatter formatter( stream );
    formatter.setTableRowLineAppendText( "" );
    formatter.setTableRowPrependText( "" );
    formatter.setCommentPrefix( "#" );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "x" ),
        RifTextDataTableColumn( "y" ),
        RifTextDataTableColumn( "value" ),
    };

    formatter.header( header );

    for ( unsigned int j = 0; j < numVerticesIJ.y(); j++ )
    {
        for ( unsigned int i = 0; i < numVerticesIJ.x(); i++ )
        {
            formatter.add( static_cast<int>( i ) );
            formatter.add( static_cast<int>( j ) );
            formatter.add( contourMapProjection->valueAtVertex( i, j ) );
            formatter.rowCompleted();
        }
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToAsciiFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Contour Map to Ascii" );
}
