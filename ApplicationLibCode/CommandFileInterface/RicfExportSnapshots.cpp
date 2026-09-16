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
#include "RicfCommandForwarding.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimViewWindow.h"
#include "RimcProject.h"
#include "RimcViewWindow.h"

#include "cafPdmFieldScriptingCapability.h"

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
    CAF_PDM_InitScriptableField( &m_type, "type", RicfExportSnapshots::SnapshotsTypeEnum(), "Type" );
    CAF_PDM_InitScriptableField( &m_prefix, "prefix", QString(), "Prefix" );
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case Id" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View Id" );
    CAF_PDM_InitScriptableField( &m_exportFolder, "exportFolder", QString(), "Export Folder" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_plotOutputFormat, "plotOutputFormat", "Output Format" );
    CAF_PDM_InitScriptableField( &m_width, "width", -1, "Width" );
    CAF_PDM_InitScriptableField( &m_height, "height", -1, "Height" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportSnapshots::execute()
{
    const QString commandName = classKeyword();

    // Resolve the export folder from the command file executor state. The Rimc methods only accept explicit folders.
    QString exportFolder = m_exportFolder();
    if ( exportFolder.isEmpty() )
    {
        exportFolder = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::SNAPSHOTS );
    }

    const RiaDefines::SnapshotFileFormat plotFileFormat =
        m_plotOutputFormat() == PlotOutputFormat::PDF ? RiaDefines::SnapshotFileFormat::PDF : RiaDefines::SnapshotFileFormat::PNG;

    const bool exportViews = m_type() != SnapshotsType::PLOTS;
    const bool exportPlots = m_type() != SnapshotsType::VIEWS;

    // No case or view filtering: export everything of the requested type through the project method
    if ( m_caseId() == -1 && m_viewId() == -1 )
    {
        RimProject_exportSnapshots method( RimProject::current() );
        method.setContentType( m_type() == SnapshotsType::ALL ? RiaDefines::SnapshotContentType::ALL
                               : exportViews                  ? RiaDefines::SnapshotContentType::VIEWS
                                                              : RiaDefines::SnapshotContentType::PLOTS );
        method.setExportFolder( exportFolder );
        method.setPrefix( m_prefix() );
        method.setWidth( m_width() );
        method.setHeight( m_height() );
        method.setPlotFileFormat( plotFileFormat );

        return RicfForwarding::toScriptResponse( method.execute(), commandName );
    }

    // Collect the view windows matching the case and view filters
    std::vector<RimViewWindow*> viewWindows;

    if ( exportViews )
    {
        RimProject* project = RimProject::current();
        for ( RimCase* gridCase : project->allGridCases() )
        {
            if ( !gridCase ) continue;
            if ( m_caseId() != -1 && m_caseId() != gridCase->caseId() ) continue;

            for ( Rim3dView* view : gridCase->views() )
            {
                if ( view && view->viewer() && ( m_viewId() == -1 || m_viewId() == view->id() ) )
                {
                    viewWindows.push_back( view );
                }
            }
        }
    }

    if ( exportPlots )
    {
        // Plots are not associated with a case id, so only the view id filter applies
        for ( RimViewWindow* viewWindow : RimMainPlotCollection::current()->descendantsIncludingThisOfType<RimViewWindow>() )
        {
            if ( viewWindow->isMainDockedWindow() && viewWindow->viewWidget() && ( m_viewId() == -1 || m_viewId() == viewWindow->id() ) )
            {
                viewWindows.push_back( viewWindow );
            }
        }
    }

    for ( RimViewWindow* viewWindow : viewWindows )
    {
        RimViewWindow_exportSnapshot method( viewWindow );
        method.setExportFolder( exportFolder );
        method.setPrefix( m_prefix() );
        method.setWidth( m_width() );
        method.setHeight( m_height() );
        method.setFileFormat( plotFileFormat );

        auto result = method.execute();
        if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );
    }

    return caf::PdmScriptResponse();
}
