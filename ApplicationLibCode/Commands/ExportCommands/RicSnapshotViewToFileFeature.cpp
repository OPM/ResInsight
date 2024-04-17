/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicSnapshotViewToFileFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPlotWindowRedrawScheduler.h"

#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimPlotWindow.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindow.h"

#include "RicSnapshotFilenameGenerator.h"

#include "cafUtils.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QFileInfo>
#include <QMdiSubWindow>
#include <QPageLayout>
#include <QPdfWriter>

CAF_CMD_SOURCE_INIT( RicSnapshotViewToFileFeature, "RicSnapshotViewToFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveSnapshotAs( const QString& fileName, RimViewWindow* viewWindow )
{
    auto* plotWindow = dynamic_cast<RimPlotWindow*>( viewWindow );
    if ( plotWindow && fileName.endsWith( ".pdf" ) )
    {
        savePlotPdfReportAs( fileName, plotWindow );
    }
    else if ( viewWindow )
    {
        QImage image = viewWindow->snapshotWindowContent();
        saveSnapshotAs( fileName, image );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveSnapshotAs( const QString& fileName, const QImage& image )
{
    if ( !image.isNull() )
    {
        if ( image.save( fileName ) )
        {
            RiaLogging::info( QString( "Exported snapshot image to %1" ).arg( fileName ) );
        }
        else
        {
            RiaLogging::error( QString( "Error when trying to export snapshot image to %1" ).arg( fileName ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::savePlotPdfReportAs( const QString& fileName, RimPlotWindow* plot )
{
    if ( !plot || !plot->viewWidget() ) return;

    auto viewWidget = plot->viewWidget();

    RiaPlotWindowRedrawScheduler::instance()->performScheduledUpdatesAndReplots();
    QCoreApplication::processEvents();
    QFile pdfFile( fileName );
    if ( pdfFile.open( QIODevice::WriteOnly ) )
    {
        int resolution = RiaGuiApplication::applicationResolution();

        QPdfWriter pdfPrinter( fileName );
        pdfPrinter.setPageLayout( plot->pageLayout() );
        pdfPrinter.setCreator( QCoreApplication::applicationName() );
        pdfPrinter.setResolution( resolution );
        QRect widgetRect = viewWidget->contentsRect();

        auto* multiPlot = dynamic_cast<RimMultiPlot*>( plot );
        if ( multiPlot && multiPlot->previewModeEnabled() )
        {
            QRect pageRect = pdfPrinter.pageLayout().fullRectPixels( resolution );
            viewWidget->resize( pageRect.size() );
            plot->renderWindowContent( &pdfPrinter );
            viewWidget->resize( widgetRect.size() );
        }
        else
        {
            QRect pageRect = pdfPrinter.pageLayout().paintRectPixels( resolution );
            viewWidget->resize( pageRect.size() );
            plot->renderWindowContent( &pdfPrinter );
            viewWidget->resize( widgetRect.size() );
        }
    }
    else
    {
        RiaLogging::error( QString( "Could not write PDF to %1" ).arg( fileName ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveViewWindowToFile( RimViewWindow* viewWindow, const QString& defaultFileBaseName /*= "image" */ )
{
    auto* plotWindow = dynamic_cast<RimPlotWindow*>( viewWindow );

    QString fileName = generateSaveFileName( defaultFileBaseName, plotWindow != nullptr );
    if ( !fileName.isEmpty() )
    {
        if ( plotWindow && fileName.endsWith( "PDF", Qt::CaseInsensitive ) )
        {
            savePlotPdfReportAs( fileName, plotWindow );
        }
        else
        {
            RicSnapshotViewToFileFeature::saveSnapshotAs( fileName, viewWindow->snapshotWindowContent() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveImageToFile( const QImage& image, const QString& defaultFileBaseName )
{
    QString fileName = generateSaveFileName( defaultFileBaseName, false );
    if ( !fileName.isEmpty() )
    {
        RicSnapshotViewToFileFeature::saveSnapshotAs( fileName, image );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSnapshotViewToFileFeature::generateSaveFileName( const QString& defaultFileBaseName, bool supportPDF, const QString& defaultExtension )
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    QString startPath;
    if ( !proj->fileName().isEmpty() )
    {
        QFileInfo fi( proj->fileName() );
        startPath = fi.absolutePath();
    }
    else
    {
        startPath = app->lastUsedDialogDirectory( "IMAGE_SNAPSHOT" );
    }

    QStringList imageFileExtensions;
    imageFileExtensions << "*.png"
                        << "*.jpg"
                        << "*.bmp"
                        << "*.pbm"
                        << "*.pgm";
    QString fileExtensionFilter = QString( "Images (%1)" ).arg( imageFileExtensions.join( " " ) );

    QString pdfFilter = "PDF report( *.pdf )";
    if ( supportPDF )
    {
        fileExtensionFilter += QString( ";;%1" ).arg( pdfFilter );
    }

    QString defaultAbsFileName = caf::Utils::constructFullFileName( startPath, defaultFileBaseName, "." + defaultExtension );

    QString selectedExtension;
    if ( supportPDF && defaultExtension.compare( "pdf", Qt::CaseInsensitive ) == 0 )
    {
        selectedExtension = pdfFilter;
    }
    QString fileName =
        RiuFileDialogTools::getSaveFileName( nullptr, tr( "Export to File" ), defaultAbsFileName, fileExtensionFilter, &selectedExtension );
    if ( !fileName.isEmpty() )
    {
        // Remember the directory to next time
        app->setLastUsedDialogDirectory( "IMAGE_SNAPSHOT", QFileInfo( fileName ).absolutePath() );
    }
    return fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIcon RicSnapshotViewToFileFeature::icon()
{
    return QIcon( ":/SnapShotSave.svg" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSnapshotViewToFileFeature::text()
{
    return "Snapshot To File";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::onActionTriggered( bool isChecked )
{
    // Get active view window before displaying the file selection dialog
    // If this is done after the file save dialog is displayed (and closed)
    // app->activeViewWindow() returns nullptr on Linux

    RimViewWindow* viewWindow = RiaGuiApplication::activeViewWindow();
    if ( !viewWindow )
    {
        RiaLogging::error( "No view window is available, nothing to do" );

        return;
    }

    saveViewWindowToFile( viewWindow, RicSnapshotFilenameGenerator::generateSnapshotFileName( viewWindow ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( text() );
    actionToSetup->setIcon( icon() );
}
