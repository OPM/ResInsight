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

#include "RicfExportVisibleCells.h"

#include "RiaApplication.h"

#include "ExportCommands/RicSaveEclipseInputVisibleCellsUi.h"
#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"
#include "RicfCommandForwarding.h"

#include "RimEclipseView.h"
#include "RimcEclipseView.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfExportVisibleCells, "exportVisibleCells" );

namespace caf
{
template <>
void AppEnum<RicfExportVisibleCells::ExportKeyword>::setUp()
{
    addItem( RicfExportVisibleCells::ExportKeyword::FLUXNUM, "FLUXNUM", "FLUXNUM" );
    addItem( RicfExportVisibleCells::ExportKeyword::MULTNUM, "MULTNUM", "MULTNUM" );
    addItem( RicfExportVisibleCells::ExportKeyword::ACTNUM, "ACTNUM", "ACTNUM" );

    setDefault( RicfExportVisibleCells::ExportKeyword::FLUXNUM );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportVisibleCells::RicfExportVisibleCells()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View ID" );
    CAF_PDM_InitScriptableField( &m_viewName, "viewName", QString(), "View Name" );
    CAF_PDM_InitScriptableField( &m_exportKeyword, "exportKeyword", caf::AppEnum<RicfExportVisibleCells::ExportKeyword>(), "Export Keyword" );
    CAF_PDM_InitScriptableField( &m_visibleActiveCellsValue, "visibleActiveCellsValue", 1, "Visible Active Cells Value" );
    CAF_PDM_InitScriptableField( &m_hiddenActiveCellsValue, "hiddenActiveCellsValue", 0, "Hidden Active Cells Value" );
    CAF_PDM_InitScriptableField( &m_inactiveCellsValue, "inactiveCellsValue", 0, "Inactive Cells Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportVisibleCells::execute()
{
    const QString commandName = classKeyword();

    if ( m_caseId < 0 || ( m_viewName().isEmpty() && m_viewId() < 0 ) )
    {
        return RicfForwarding::errorResponse( "CaseId or view name or view id not specified", commandName );
    }

    RimEclipseView* eclipseView = nullptr;
    if ( m_viewId() >= 0 )
    {
        eclipseView = RicfApplicationTools::viewFromCaseIdAndViewId( m_caseId, m_viewId() );
    }
    else
    {
        eclipseView = RicfApplicationTools::viewFromCaseIdAndViewName( m_caseId, m_viewName );
    }
    if ( !eclipseView )
    {
        return RicfForwarding::errorResponse( QString( "Could not find view of id %1 or named '%2' in case ID %3" )
                                                  .arg( m_viewId() )
                                                  .arg( m_viewName() )
                                                  .arg( m_caseId() ),
                                              commandName );
    }

    // Resolve the default export folder from the command file executor state. The Rimc method requires an explicit file.
    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::CELLS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->currentProjectPath();
    }
    QDir baseDir( exportFolder );

    RicSaveEclipseInputVisibleCellsUi::ExportKeyword exportKeyword = RicSaveEclipseInputVisibleCellsUi::FLUXNUM;
    if ( m_exportKeyword == ExportKeyword::MULTNUM )
        exportKeyword = RicSaveEclipseInputVisibleCellsUi::MULTNUM;
    else if ( m_exportKeyword == ExportKeyword::ACTNUM )
        exportKeyword = RicSaveEclipseInputVisibleCellsUi::ACTNUM;

    RimEclipseView_exportVisibleCells method( eclipseView );
    method.setExportFile( baseDir.absoluteFilePath( QString( "%1.grdecl" ).arg( m_exportKeyword().text() ) ) );
    method.setExportKeyword( exportKeyword );
    method.setVisibleActiveCellsValue( m_visibleActiveCellsValue() );
    method.setHiddenActiveCellsValue( m_hiddenActiveCellsValue() );
    method.setInactiveCellsValue( m_inactiveCellsValue() );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
