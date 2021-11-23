/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicExportSelectedWellPathsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicExportWellPathsUi.h"

#include "RifTextDataTableFormatter.h"

#include "RigWellPath.h"
#include "RigWellPathGeometryExporter.h"

#include "RimDialogData.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

#include <memory>

CAF_CMD_SOURCE_INIT( RicExportSelectedWellPathsFeature, "RicExportSelectedWellPathsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::exportWellPath( gsl::not_null<const RimWellPath*> wellPath,
                                                        double                            mdStepSize,
                                                        const QString&                    folder,
                                                        bool                              writeProjectInfo )
{
    auto fileName = caf::Utils::makeValidFileBasename( wellPath->name() );
    fileName += ".dev";

    auto filePtr = openFileForExport( folder, fileName );
    auto stream  = createOutputFileStream( *filePtr );

    std::vector<double> xValues;
    std::vector<double> yValues;
    std::vector<double> tvdValues;
    std::vector<double> mdValues;

    bool showTextMdRkb = false;
    RigWellPathGeometryExporter::computeWellPathDataForExport( wellPath,
                                                               mdStepSize,
                                                               xValues,
                                                               yValues,
                                                               tvdValues,
                                                               mdValues,
                                                               showTextMdRkb );

    writeWellPathGeometryToStream( *stream, wellPath->name(), xValues, yValues, tvdValues, mdValues, showTextMdRkb, writeProjectInfo );
    filePtr->close();

    RiaLogging::info( QString( "Exported well geometry to %1" ).arg( filePtr->fileName() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::writeWellPathGeometryToStream( QTextStream&       stream,
                                                                       const RigWellPath& wellPathGeom,
                                                                       const QString&     exportName,
                                                                       double             mdStepSize,
                                                                       bool               showTextMdRkb,
                                                                       double             rkbOffset,
                                                                       bool               writeProjectInfo )
{
    std::vector<double> xValues;
    std::vector<double> yValues;
    std::vector<double> tvdValues;
    std::vector<double> mdValues;

    RigWellPathGeometryExporter::computeWellPathDataForExport( wellPathGeom,
                                                               mdStepSize,
                                                               rkbOffset,
                                                               xValues,
                                                               yValues,
                                                               tvdValues,
                                                               mdValues );
    writeWellPathGeometryToStream( stream, exportName, xValues, yValues, tvdValues, mdValues, showTextMdRkb, writeProjectInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::writeWellPathGeometryToStream( QTextStream&               stream,
                                                                       const QString&             exportName,
                                                                       const std::vector<double>& xValues,
                                                                       const std::vector<double>& yValues,
                                                                       const std::vector<double>& tvdValues,
                                                                       const std::vector<double>& mdValues,
                                                                       bool                       showTextMdRkb,
                                                                       bool                       writeProjectInfo )
{
    RifTextDataTableFormatter formatter( stream );
    formatter.setHeaderPrefix( "# " );
    formatter.setCommentPrefix( "# " );
    formatter.setTableRowPrependText( "  " );

    if ( writeProjectInfo )
    {
        formatter.comment( "Project: " + RimProject::current()->fileName );
        stream << "\n";
    }

    stream << "WELLNAME: '" << caf::Utils::makeValidFileBasename( exportName ) << "'"
           << "\n";

    auto numberFormat = RifTextDataTableDoubleFormatting( RIF_FLOAT, 2 );
    formatter.header( { { "X", numberFormat, RIGHT },
                        { "Y", numberFormat, RIGHT },
                        { "TVDMSL", numberFormat, RIGHT },
                        { showTextMdRkb ? "MDRKB" : "MDMSL", numberFormat, RIGHT } } );

    for ( size_t i = 0; i < xValues.size(); i++ )
    {
        // Write to file
        formatter.add( xValues[i] );
        formatter.add( yValues[i] );
        formatter.add( tvdValues[i] );
        formatter.add( mdValues[i] );
        formatter.rowCompleted( "" );
    }
    formatter.tableCompleted( "", false );

    stream << -999 << "\n"
           << "\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFilePtr RicExportSelectedWellPathsFeature::openFileForExport( const QString& folderName, const QString& fileName )
{
    QDir exportFolder = QDir( folderName );
    if ( !exportFolder.exists() )
    {
        bool createdPath = exportFolder.mkpath( "." );
        if ( createdPath ) RiaLogging::info( "Created export folder " + folderName );
    }

    QString  filePath = exportFolder.filePath( fileName );
    QFilePtr exportFile( new QFile( filePath ) );
    if ( !exportFile->open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        auto errorMessage = QString( "Export Well Path: Could not open the file: %1" ).arg( filePath );
        RiaLogging::error( errorMessage );
    }
    return exportFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStreamPtr RicExportSelectedWellPathsFeature::createOutputFileStream( QFile& file )
{
    auto stream = QTextStreamPtr( new QTextStream( &file ) );
    stream->setRealNumberNotation( QTextStream::FixedNotation );
    stream->setRealNumberPrecision( 2 );
    return stream;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::exportWellPathsToFile( const std::vector<RimWellPath*>& wellPaths )
{
    if ( !wellPaths.empty() )
    {
        auto dialogData = openDialog();
        if ( dialogData )
        {
            auto folder     = dialogData->exportFolder();
            auto mdStepSize = dialogData->mdStepSize();
            if ( folder.isEmpty() )
            {
                return;
            }

            for ( auto wellPath : wellPaths )
            {
                exportWellPath( wellPath, mdStepSize, folder );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSelectedWellPathsFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    return !wellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellPath*> wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    exportWellPathsToFile( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Selected Well Paths" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportWellPathsUi* RicExportSelectedWellPathsFeature::openDialog()
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    QString startPath = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "WELL_PATH_EXPORT_DIR" );
    if ( startPath.isEmpty() )
    {
        QFileInfo fi( proj->fileName() );
        startPath = fi.absolutePath();
    }

    RicExportWellPathsUi* featureUi = app->project()->dialogData()->wellPathsExportData();
    if ( featureUi->exportFolder().isEmpty() )
    {
        featureUi->setExportFolder( startPath );
    }

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                 featureUi,
                                                 "Export Well Paths",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    propertyDialog.resize( QSize( 600, 60 ) );

    if ( propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty() )
    {
        auto dialogData = app->project()->dialogData()->wellPathsExportData();
        app->setLastUsedDialogDirectory( "WELL_PATH_EXPORT_DIR", dialogData->exportFolder() );
        return dialogData;
    }
    return nullptr;
}
