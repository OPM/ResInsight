/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfExportSnapshots.h"

#include "RicfCommandFileExecutor.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaRegressionTestRunner.h"

#include "RiuMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RicfExportSnapshots, "exportSnapshots" );

namespace caf
{
template <>
void RicfExportSnapshots::PreferredOutputFormatEnum::setUp()
{
    addItem( RicfExportSnapshots::PlotOutputFormat::PNG, "PNG", "PNG" );
    addItem( RicfExportSnapshots::PlotOutputFormat::PDF, "PDF", "PDF" );
    setDefault( RicfExportSnapshots::PlotOutputFormat::PNG );
}

template <>
void RicfExportSnapshots::SnapshotsTypeEnum::setUp()
{
    addItem( RicfExportSnapshots::SnapshotsType::ALL, "ALL", "All" );
    addItem( RicfExportSnapshots::SnapshotsType::VIEWS, "VIEWS", "Views" );
    addItem( RicfExportSnapshots::SnapshotsType::PLOTS, "PLOTS", "Plots" );
    setDefault( RicfExportSnapshots::SnapshotsType::ALL );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportSnapshots::RicfExportSnapshots()
{
    CAF_PDM_InitScriptableField( &m_type, "type", RicfExportSnapshots::SnapshotsTypeEnum(), "Type", "", "", "" );
    CAF_PDM_InitScriptableField( &m_prefix, "prefix", QString(), "Prefix", "", "", "" );
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case Id", "", "", "" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View Id", "", "", "" );
    CAF_PDM_InitScriptableField( &m_exportFolder, "exportFolder", QString(), "Export Folder", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_plotOutputFormat, "plotOutputFormat", "Output Format", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportSnapshots::execute()
{
    if ( !RiaGuiApplication::isRunning() )
    {
        QString error( "RicfExportSnapshot: Command cannot run without a GUI" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    CVF_ASSERT( mainWnd );
    mainWnd->hideAllDockWidgets();
    RiaGuiApplication::instance()->processEvents();

    QString absolutePathToSnapshotDir =
        RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::SNAPSHOTS );

    if ( !m_exportFolder().isEmpty() )
    {
        absolutePathToSnapshotDir = m_exportFolder;
    }
    if ( absolutePathToSnapshotDir.isNull() )
    {
        absolutePathToSnapshotDir =
            RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "snapshots" );
    }
    if ( m_type == RicfExportSnapshots::SnapshotsType::VIEWS || m_type == RicfExportSnapshots::SnapshotsType::ALL )
    {
        if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
        {
            RiaRegressionTestRunner::setDefaultSnapshotSizeFor3dViews();

            QApplication::processEvents();
        }

        RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( absolutePathToSnapshotDir,
                                                                           m_prefix,
                                                                           m_caseId(),
                                                                           m_viewId() );
    }
    if ( m_type == RicfExportSnapshots::SnapshotsType::PLOTS || m_type == RicfExportSnapshots::SnapshotsType::ALL )
    {
        bool activateWidget = false;
        if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
        {
            RiaRegressionTestRunner::setDefaultSnapshotSizeForPlotWindows();

            QApplication::processEvents();
        }
        else
        {
            activateWidget = true;
        }

        QString fileSuffix = ".png";
        if ( m_plotOutputFormat == PlotOutputFormat::PDF ) fileSuffix = ".pdf";
        RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlotsIntoFolder( absolutePathToSnapshotDir,
                                                                           activateWidget,
                                                                           m_prefix,
                                                                           m_viewId(),
                                                                           fileSuffix );
    }

    mainWnd->loadWinGeoAndDockToolBarLayout();
    RiaGuiApplication::instance()->processEvents();

    return caf::PdmScriptResponse();
}
