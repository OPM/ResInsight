/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimcViewWindow.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaRegressionTestRunner.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"

#include "Rim3dView.h"
#include "RimPlotWindow.h"
#include "RimViewWindow.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimViewWindow, RimViewWindow_exportSnapshot, "exportSnapshot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow_exportSnapshot::RimViewWindow_exportSnapshot( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Snapshot", "", "", "Export a snapshot of the view or plot to a file" );

    CAF_PDM_InitScriptableField( &m_exportFolder,
                                 "ExportFolder",
                                 QString(),
                                 "Export Folder",
                                 "",
                                 "",
                                 "Folder to export to. Defaults to the 'snapshots' folder next to the project file." );
    CAF_PDM_InitScriptableField( &m_prefix, "Prefix", QString(), "Prefix", "", "", "Prefix for the generated file name" );
    CAF_PDM_InitScriptableField( &m_width, "Width", -1, "Width", "", "", "Image width in pixels. Use -1 for the current size." );
    CAF_PDM_InitScriptableField( &m_height, "Height", -1, "Height", "", "", "Image height in pixels. Use -1 for the current size." );
    CAF_PDM_InitScriptableField( &m_fileFormat,
                                 "FileFormat",
                                 RiaDefines::SnapshotFileFormat::PNG,
                                 "File Format",
                                 "",
                                 "",
                                 "Output file format. PDF is only supported for plots." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow_exportSnapshot::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow_exportSnapshot::setPrefix( const QString& prefix )
{
    m_prefix = prefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow_exportSnapshot::setWidth( int width )
{
    m_width = width;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow_exportSnapshot::setHeight( int height )
{
    m_height = height;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow_exportSnapshot::setFileFormat( RiaDefines::SnapshotFileFormat fileFormat )
{
    m_fileFormat = fileFormat;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimViewWindow_exportSnapshot::execute()
{
    if ( !RiaGuiApplication::isRunning() ) return std::unexpected( "Snapshots cannot be exported without a GUI." );

    auto* viewWindow = self<RimViewWindow>();
    if ( !viewWindow ) return std::unexpected( "No view window is available." );

    const QString exportFolder = resolveExportFolder( m_exportFolder() );

    int width  = m_width();
    int height = m_height();
    if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        QSize defaultSize = RiaRegressionTestRunner::regressionDefaultImageSize();
        width             = defaultSize.width();
        height            = defaultSize.height();
    }

    if ( auto* view3d = dynamic_cast<Rim3dView*>( viewWindow ) )
    {
        if ( !view3d->viewer() ) return std::unexpected( QString( "View '%1' has no viewer." ).arg( view3d->name() ) );

        RicSnapshotAllViewsToFileFeature::exportSnapshotOfView( view3d, exportFolder, width, height, m_prefix() );
        return nullptr;
    }

    if ( !viewWindow->viewWidget() ) return std::unexpected( "The plot has no widget to export." );

    const bool    activateWidget = !RiaRegressionTestRunner::instance()->isRunningRegressionTests();
    const QString fileSuffix     = m_fileFormat() == RiaDefines::SnapshotFileFormat::PDF ? ".pdf" : ".png";

    RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlot( viewWindow, exportFolder, width, height, activateWidget, m_prefix(), fileSuffix );
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimViewWindow_exportSnapshot::resolveExportFolder( const QString& exportFolder )
{
    if ( !exportFolder.isEmpty() ) return exportFolder;

    return RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "snapshots" );
}
