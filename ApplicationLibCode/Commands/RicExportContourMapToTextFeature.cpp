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
#include "RicExportContourMapToTextFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RicExportContourMapToTextUi.h"

#include "RifTextDataTableFormatter.h"

#include "RimContourMapProjection.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechContourMapView.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "Riu3DMainWindowTools.h"
#include "RiuViewer.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileInfo>

#include <cmath>

RICF_SOURCE_INIT( RicExportContourMapToTextFeature, "RicExportContourMapToTextFeature", "exportContourMapToText" );

RicExportContourMapToTextFeature::RicExportContourMapToTextFeature()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_exportFileName, "exportFileName", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_exportLocalCoordinates, "exportLocalCoordinates", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_undefinedValueLabel, "undefinedValueLabel", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_excludeUndefinedValues, "excludeUndefinedValues", "" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View Id" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportContourMapToTextFeature::isCommandEnabled() const
{
    auto [existingEclipseContourMap, existingGeoMechContourMap] = findContourMapView();

    return existingEclipseContourMap || existingGeoMechContourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToTextFeature::onActionTriggered( bool isChecked )
{
    RimEclipseContourMapView* existingEclipseContourMap = nullptr;
    RimGeoMechContourMapView* existingGeoMechContourMap = nullptr;

    auto sourceViews          = findContourMapView();
    existingEclipseContourMap = sourceViews.first;
    existingGeoMechContourMap = sourceViews.second;

    CAF_ASSERT( existingEclipseContourMap || existingGeoMechContourMap );

    RimContourMapProjection* contourMapProjection = nullptr;
    QString                  contourMapName;
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
    QString startPath    = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "CONTOUR_EXPORT" );
    QString fileBaseName = caf::Utils::makeValidFileBasename( contourMapName );
    startPath            = startPath + "/" + fileBaseName + ".txt";

    RicExportContourMapToTextUi featureUi;
    featureUi.setExportFileName( startPath );

    caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                                 &featureUi,
                                                 "Export Contour Map to Text",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        QString fileName = featureUi.exportFileName();

        app->setLastUsedDialogDirectory( "CONTOUR_EXPORT", QFileInfo( fileName ).absolutePath() );
        m_exportFileName         = fileName;
        m_exportLocalCoordinates = featureUi.exportLocalCoordinates();
        m_undefinedValueLabel    = featureUi.undefinedValueLabel();
        m_excludeUndefinedValues = featureUi.excludeUndefinedValues();

        caf::PdmScriptResponse response = execute();
        QStringList            messages = response.messages();

        if ( !messages.empty() )
        {
            QString displayMessage = QString( "Problem exporting contour map:\n%2" ).arg( messages.join( "\n" ) );
            if ( response.status() == caf::PdmScriptResponse::COMMAND_ERROR )
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
void RicExportContourMapToTextFeature::writeMetaDataToStream( QTextStream&                   stream,
                                                              const RimContourMapProjection* contourMapProjection,
                                                              const QString&                 caseName,
                                                              bool                           exportLocalCoordinates )
{
    cvf::Vec2ui numVerticesIJ = contourMapProjection->numberOfVerticesIJ();
    stream << "# case name : " << contourMapProjection->caseName() << "\n";
    stream << "# sampling points : nx=" << numVerticesIJ.x() << " ny=" << numVerticesIJ.y() << "\n";
    stream << "# time and date : " << contourMapProjection->currentTimeStepName() << "\n";
    stream << "# property name : " << contourMapProjection->resultDescriptionText() << "\n";
    if ( exportLocalCoordinates )
    {
        stream << "# UTM offset : x=" << contourMapProjection->origin3d().x() << " y=" << contourMapProjection->origin3d().y() << "\n";
    }
    stream << "\n\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToTextFeature::writeContourMapToStream( QTextStream&                   stream,
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
    formatter.setDefaultMarker( undefinedValueLabel );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "x" ),
        RifTextDataTableColumn( "y" ),
        RifTextDataTableColumn( "value" ),
    };

    formatter.header( header );

    cvf::Vec2ui         numVerticesIJ    = contourMapProjection->numberOfVerticesIJ();
    std::vector<double> xVertexPositions = contourMapProjection->xVertexPositions();
    std::vector<double> yVertexPositions = contourMapProjection->yVertexPositions();

    // Undefined values are positive inf in contour map projection.
    double undefined = std::numeric_limits<double>::infinity();
    for ( unsigned int j = 0; j < numVerticesIJ.y(); j++ )
    {
        for ( unsigned int i = 0; i < numVerticesIJ.x(); i++ )
        {
            double value = contourMapProjection->valueAtVertex( i, j );
            if ( !( std::isinf( value ) && excludeUndefinedValues ) )
            {
                double x = xVertexPositions.at( i );
                double y = yVertexPositions.at( j );
                if ( !exportLocalCoordinates )
                {
                    x += contourMapProjection->origin3d().x();
                    y += contourMapProjection->origin3d().y();
                }

                formatter.add( x );
                formatter.add( y );
                formatter.addValueOrDefaultMarker( value, undefined );
                formatter.rowCompleted();
            }
        }
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimEclipseContourMapView*, RimGeoMechContourMapView*> RicExportContourMapToTextFeature::findContourMapView()
{
    RimEclipseContourMapView* existingEclipseContourMap = nullptr;
    RimGeoMechContourMapView* existingGeoMechContourMap = nullptr;

    auto contextMenuWidget = dynamic_cast<RiuViewer*>( caf::CmdFeatureManager::instance()->currentContextMenuTargetWidget() );

    if ( contextMenuWidget )
    {
        {
            auto candidate = dynamic_cast<RimEclipseContourMapView*>( contextMenuWidget->ownerReservoirView() );
            if ( candidate )
            {
                existingEclipseContourMap = candidate;
            }
        }
        {
            auto candidate = dynamic_cast<RimGeoMechContourMapView*>( contextMenuWidget->ownerReservoirView() );
            if ( candidate )
            {
                existingGeoMechContourMap = candidate;
            }
        }
    }

    if ( !existingEclipseContourMap && !existingGeoMechContourMap )
    {
        existingEclipseContourMap = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseContourMapView>();
        existingGeoMechContourMap = caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechContourMapView>();
    }

    auto pair = std::make_pair( existingEclipseContourMap, existingGeoMechContourMap );

    return pair;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportContourMapToTextFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Contour Map to Text" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicExportContourMapToTextFeature::execute()
{
    caf::PdmScriptResponse response;
    QStringList            errorMessages, warningMessages;

    RiaApplication* app = RiaApplication::instance();

    RimProject* proj = app->project();
    CAF_ASSERT( proj );

    Rim3dView* myView = nullptr;
    for ( auto view : proj->allViews() )
    {
        if ( m_viewId == view->id() )
        {
            myView = view;
        }
    }

    if ( !myView )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, "No contour map view found" );
        return response;
    }

    RimContourMapProjection*  contourMapProjection      = nullptr;
    RimEclipseContourMapView* existingEclipseContourMap = dynamic_cast<RimEclipseContourMapView*>( myView );
    RimGeoMechContourMapView* existingGeoMechContourMap = dynamic_cast<RimGeoMechContourMapView*>( myView );
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

    QFile exportFile( m_exportFileName );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        errorMessages << QString( "Export Contour Map to Text : Could not open the file: %1" ).arg( m_exportFileName );
    }
    else
    {
        QString     tableText;
        QTextStream stream( &exportFile );
        writeMetaDataToStream( stream, contourMapProjection, contourMapName, m_exportLocalCoordinates.value() );
        writeContourMapToStream( stream,
                                 contourMapProjection,
                                 m_exportLocalCoordinates.value(),
                                 m_undefinedValueLabel.value(),
                                 m_excludeUndefinedValues.value() );
    }

    for ( QString errorMessage : errorMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, errorMessage );
    }

    return response;
}
