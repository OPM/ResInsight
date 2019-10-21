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
#include "RicExportContourMapToAsciiUi.h"
#include "RifTextDataTableFormatter.h"
#include "RimContourMapProjection.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechContourMapView.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>

#include <cmath>

RICF_SOURCE_INIT( RicExportContourMapToAsciiFeature, "RicExportContourMapToAsciiFeature", "exportContourMapToText" );

RicExportContourMapToAsciiFeature::RicExportContourMapToAsciiFeature()
{
    RICF_InitFieldNoDefault( &m_exportFileName, "exportFileName", "", "", "", "" );
    RICF_InitFieldNoDefault( &m_exportLocalCoordinates, "exportLocalCoordinates", "", "", "", "" );
    RICF_InitFieldNoDefault( &m_undefinedValueLabel, "undefinedValueLabel", "", "", "", "" );
    RICF_InitFieldNoDefault( &m_excludeUndefinedValues, "excludeUndefinedValues", "", "", "", "" );
    RICF_InitField( &m_viewId, "viewId", -1, "View Id", "", "", "" );
}

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
        m_viewId             = existingEclipseContourMap->id();
        contourMapProjection = existingEclipseContourMap->contourMapProjection();
        contourMapName       = existingEclipseContourMap->createAutoName();
    }
    else if ( existingGeoMechContourMap )
    {
        m_viewId             = existingGeoMechContourMap->id();
        contourMapProjection = existingGeoMechContourMap->contourMapProjection();
        contourMapName       = existingGeoMechContourMap->createAutoName();
    }

    CAF_ASSERT( contourMapProjection );

    RiaGuiApplication* app = RiaGuiApplication::instance();
    CAF_ASSERT( app && "Must be gui mode" );

    QString startPath = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "CONTOUR_EXPORT" );

    QString fileBaseName = caf::Utils::makeValidFileBasename( contourMapName );

    startPath = startPath + "/" + fileBaseName + ".txt";

    RicExportContourMapToAsciiUi featureUi;
    featureUi.setExportFileName( startPath );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                 &featureUi,
                                                 "Export Contour Map as Text",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        QString fileName = featureUi.exportFileName();

        app->setLastUsedDialogDirectory( "CONTOUR_EXPORT", fileName );
        m_exportFileName         = fileName;
        m_exportLocalCoordinates = featureUi.exportLocalCoordinates();
        m_undefinedValueLabel    = featureUi.undefinedValueLabel();
        m_excludeUndefinedValues = featureUi.excludeUndefinedValues();

        RicfCommandResponse response = execute();
        QStringList         messages = response.messages();

        if ( !messages.empty() )
        {
            QString displayMessage = QString( "Problem exporting contour map:\n%2" ).arg( messages.join( "\n" ) );
            if ( response.status() == RicfCommandResponse::COMMAND_ERROR )
            {
                RiaLogging::error( displayMessage );
            }
            else
            {
                RiaLogging::warning( displayMessage );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToAsciiFeature::writeContourMapToStream( QTextStream&                   stream,
                                                                 const RimContourMapProjection* contourMapProjection,
                                                                 bool                           exportLocalCoordinates,
                                                                 const QString&                 undefinedValueLabel,
                                                                 bool                           excludeUndefinedValues )
{
    RifTextDataTableFormatter formatter( stream );
    formatter.setTableRowLineAppendText( "" );
    formatter.setTableRowPrependText( "" );
    formatter.setCommentPrefix( "#" );
    formatter.setHeaderPrefix( "" );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "x" ),
        RifTextDataTableColumn( "y" ),
        RifTextDataTableColumn( "value" ),
    };

    formatter.header( header );

    cvf::Vec2ui numVerticesIJ = contourMapProjection->numberOfVerticesIJ();
    for ( unsigned int j = 0; j < numVerticesIJ.y(); j++ )
    {
        for ( unsigned int i = 0; i < numVerticesIJ.x(); i++ )
        {
            double value = contourMapProjection->valueAtVertex( i, j );
            if ( !( std::isinf( value ) && excludeUndefinedValues ) )
            {
                formatter.add( static_cast<int>( i ) );
                formatter.add( static_cast<int>( j ) );
                formatter.add( value );
                formatter.rowCompleted();
            }
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

RicfCommandResponse RicExportContourMapToAsciiFeature::execute()
{
    RicfCommandResponse response;
    QStringList         errorMessages, warningMessages;

    RiaApplication* app = RiaApplication::instance();

    RimProject* proj = app->project();
    CAF_ASSERT( proj );

    std::vector<Rim3dView*> allViews;
    proj->allViews( allViews );

    Rim3dView* myView = nullptr;
    for ( auto view : allViews )
    {
        if ( m_viewId == view->id() )
        {
            myView = view;
        }
    }

    RimContourMapProjection*  contourMapProjection      = nullptr;
    RimEclipseContourMapView* existingEclipseContourMap = dynamic_cast<RimEclipseContourMapView*>( myView );
    RimGeoMechContourMapView* existingGeoMechContourMap = dynamic_cast<RimGeoMechContourMapView*>( myView );
    CAF_ASSERT( existingEclipseContourMap || existingGeoMechContourMap );

    QString contourMapName;
    if ( existingEclipseContourMap )
    {
        contourMapProjection = existingEclipseContourMap->contourMapProjection();
    }
    else if ( existingGeoMechContourMap )
    {
        contourMapProjection = existingGeoMechContourMap->contourMapProjection();
    }

    CAF_ASSERT( contourMapProjection );

    QFile exportFile( m_exportFileName );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        errorMessages << QString( "Export Contour Map as Text : Could not open the file: %1" ).arg( m_exportFileName );
    }
    else
    {
        QString     tableText;
        QTextStream stream( &exportFile );
        writeContourMapToStream( stream,
                                 contourMapProjection,
                                 m_exportLocalCoordinates.value(),
                                 m_undefinedValueLabel.value(),
                                 m_excludeUndefinedValues.value() );
    }

    for ( QString errorMessage : errorMessages )
    {
        response.updateStatus( RicfCommandResponse::COMMAND_ERROR, errorMessage );
    }

    return response;
}
