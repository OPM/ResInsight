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

#include "RiaFilePathTools.h"
#include "RiaViewRedrawScheduler.h"

#include "ExportCommands/RicSaveEclipseInputVisibleCellsFeature.h"
#include "ExportCommands/RicSaveEclipseInputVisibleCellsUi.h"
#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RifEclipseInputFileTools.h"

#include <QDir>

#include <cafUtils.h>

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfExportVisibleCells, "exportVisibleCells" );

namespace caf
{
template <>
void AppEnum<RicfExportVisibleCells::ExportKeyword>::setUp()
{
    addItem( RicfExportVisibleCells::FLUXNUM, "FLUXNUM", "FLUXNUM" );
    addItem( RicfExportVisibleCells::MULTNUM, "MULTNUM", "MULTNUM" );

    setDefault( RicfExportVisibleCells::FLUXNUM );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportVisibleCells::RicfExportVisibleCells()
{
    RICF_InitField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    RICF_InitField( &m_viewId, "viewId", -1, "View ID", "", "", "" );
    RICF_InitField( &m_viewName, "viewName", QString(), "View Name", "", "", "" );
    RICF_InitField( &m_exportKeyword,
                    "exportKeyword",
                    caf::AppEnum<RicfExportVisibleCells::ExportKeyword>(),
                    "Export Keyword",
                    "",
                    "",
                    "" );
    RICF_InitField( &m_visibleActiveCellsValue, "visibleActiveCellsValue", 1, "Visible Active Cells Value", "", "", "" );
    RICF_InitField( &m_hiddenActiveCellsValue, "hiddenActiveCellsValue", 0, "Hidden Active Cells Value", "", "", "" );
    RICF_InitField( &m_inactiveCellsValue, "inactiveCellsValue", 0, "Inactive Cells Value", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfExportVisibleCells::execute()
{
    if ( m_caseId < 0 || ( m_viewName().isEmpty() && m_viewId() < 0 ) )
    {
        QString error( "exportVisibleCells: CaseId or view name or view id not specified" );
        RiaLogging::error( error );
        return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
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
        QString error( QString( "exportVisibleCells: Could not find view of id %1 or named '%2' in case ID %3" )
                           .arg( m_viewId )
                           .arg( m_viewName )
                           .arg( m_caseId ) );
        RiaLogging::error( error );
        return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::CELLS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "visibleCells" );
    }

    RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();

    RicSaveEclipseInputVisibleCellsUi exportSettings;
    buildExportSettings( exportFolder, &exportSettings );
    RicSaveEclipseInputVisibleCellsFeature::executeCommand( eclipseView, exportSettings, "exportVisibleCells" );

    return RicfCommandResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfExportVisibleCells::buildExportSettings( const QString&                     exportFolder,
                                                  RicSaveEclipseInputVisibleCellsUi* exportSettings )
{
    QDir baseDir( exportFolder );
    exportSettings->exportFilename = baseDir.absoluteFilePath( QString( "%1.grdecl" ).arg( m_exportKeyword().text() ) );

    if ( m_exportKeyword == ExportKeyword::FLUXNUM )
        exportSettings->exportKeyword = RicSaveEclipseInputVisibleCellsUi::FLUXNUM;
    else if ( m_exportKeyword == ExportKeyword::MULTNUM )
        exportSettings->exportKeyword = RicSaveEclipseInputVisibleCellsUi::MULTNUM;

    exportSettings->visibleActiveCellsValue = m_visibleActiveCellsValue;
    exportSettings->hiddenActiveCellsValue  = m_hiddenActiveCellsValue;
    exportSettings->inactiveCellsValue      = m_inactiveCellsValue;
}
