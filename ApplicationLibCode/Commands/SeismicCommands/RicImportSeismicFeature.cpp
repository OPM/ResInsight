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

#include "RicNewSeismicViewFeature.h"

#include "RiaApplication.h"

#include "RimOilField.h"
#include "RimProcess.h"
#include "RimProject.h"
#include "RimSEGYConvertOptions.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicImportSeismicFeature, "RicImportSeismicFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSeismicFeature::onActionTriggered( bool isChecked )
{
#ifdef USE_OPENVDS
    QString filter = "Seismic volumes (*.zgy *.vds);;SEG-Y files (*.sgy *.segy);;All Files (*.*)";
#else
    QString filter = "Seismic volumes (*.zgy);;All Files (*.*)";
#endif

    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "SEISMIC_GRID" );
    QString fileName = RiuFileDialogTools::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(), "Import Seismic", defaultDir, filter );

#ifdef USE_OPENVDS
    QFileInfo fi( fileName );
    QString   ext = fi.suffix().toLower();
    if ( ( ext == "segy" ) || ( ext == "sgy" ) )
    {
        fileName = convertSEGYtoVDS( fileName );
    }
#endif

    if ( fileName.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "SEISMIC_GRID", QFileInfo( fileName ).absolutePath() );

    auto  proj     = RimProject::current();
    auto& seisColl = proj->activeOilField()->seismicDataCollection();

    if ( !seisColl ) return;

    if ( auto newData = seisColl->importSeismicFromFile( fileName ) )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( newData );

        RicNewSeismicViewFeature::createInitialViewIfNeeded( newData );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportSeismicFeature::convertSEGYtoVDS( QString filename )
{
    RimSEGYConvertOptions segyoptions;

    segyoptions.setInputFilename( filename );

    QFileInfo fi( filename );
    QString   outputFilename = fi.path() + "/" + fi.completeBaseName() + ".vds";
    segyoptions.setOutputFilename( outputFilename );

    caf::PdmUiPropertyViewDialog propDlg( nullptr,
                                          &segyoptions,
                                          "Convert SEG-Y to VDS file format",
                                          "",
                                          QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    propDlg.resize( QSize( 520, 420 ) );

    while ( true )
    {
        if ( propDlg.exec() != QDialog::Accepted )
        {
            outputFilename = "";
            break;
        }

        outputFilename = segyoptions.outputFilename();

        if ( QFile::exists( outputFilename ) )
        {
            QString question = QString( "File \"%1\" already exists. \n\nDo you want to overwrite it?" ).arg( outputFilename );
            auto    reply    = QMessageBox::question( nullptr, "Replace existing file?", question, QMessageBox::Yes, QMessageBox::No );
            if ( reply != QMessageBox::Yes ) continue;
        }

        if ( !segyoptions.headerFormatFilename().isEmpty() && !QFile::exists( segyoptions.headerFormatFilename() ) )
        {
            QString warning = QString( "Header Format Definition File \"%1\" could not be found." ).arg( segyoptions.headerFormatFilename() );
            QMessageBox::warning( nullptr, "File Not Found.", warning );
            continue;
        }

        auto [overrideSampleStart, sampleStartOffset] = segyoptions.sampleStartOverride();
        if ( overrideSampleStart && ( sampleStartOffset < 0.0 ) )
        {
            QString warning = "Depth (Z) Offset Override must be 0 or larger.";
            QMessageBox::warning( nullptr, "Invalid input.", warning );
            continue;
        }

        break;
    }

    if ( !outputFilename.isEmpty() )
    {
        if ( !runSEGYConversion( &segyoptions ) ) outputFilename = "";
    }

    return outputFilename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportSeismicFeature::runSEGYConversion( RimSEGYConvertOptions* options )
{
    QString     command    = options->convertCommand();
    QStringList parameters = options->convertCommandParameters();

    RimProcess process;

#ifdef WIN32
    command += ".exe";
#else
    process.addEnvironmentVariable( "LD_LIBRARY_PATH", options->programDirectory() );
#endif

    if ( !QFile::exists( command ) )
    {
        QString warning = QString( "The SEG-Y import utility \"%1\" could not be found!" ).arg( command );
        QMessageBox::critical( nullptr, "Missing Converter.", warning );
        return false;
    }

    process.setCommand( command );
    process.addParameters( parameters );

    bool showStdOut = false;
    bool showStdErr = true;

    if ( !process.execute( showStdOut, showStdErr ) )
    {
        QMessageBox::critical( nullptr,
                               "SEG-Y conversion failed.",
                               "Failed to convert the input SEG-Y file. Check log window for additional information." );
        return false;
    }

    return true;
}
