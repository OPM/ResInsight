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

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"
#include "ExportCommands/RicSaveEclipseInputVisibleCellsFeature.h"
#include "ExportCommands/RicSaveEclipseInputVisibleCellsUi.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimOilField.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"

#include "RifEclipseInputFileTools.h"

#include <QDir>

#include <cafUtils.h>

#include <QDir>

CAF_PDM_SOURCE_INIT(RicfExportVisibleCells, "exportVisibleCells");

namespace caf
{
    template<>
    void AppEnum< RicfExportVisibleCells::ExportKeyword >::setUp()
    {
        addItem(RicfExportVisibleCells::FLUXNUM, "FLUXNUM", "FLUXNUM");
        addItem(RicfExportVisibleCells::MULTNUM, "MULTNUM", "MULTNUM");

        setDefault(RicfExportVisibleCells::FLUXNUM);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportVisibleCells::RicfExportVisibleCells()
{
    RICF_InitField(&m_caseId, "caseId", -1, "Case ID", "", "", "");
    RICF_InitField(&m_viewName, "viewName", QString(), "View Name", "", "", "");
    RICF_InitField(&m_exportKeyword, "exportKeyword", caf::AppEnum<RicfExportVisibleCells::ExportKeyword>(), "Export Keyword", "", "", "");
    RICF_InitField(&m_visibleActiveCellsValue, "visibleActiveCellsValue", 1, "Visible Active Cells Value", "", "", "");
    RICF_InitField(&m_hiddenActiveCellsValue, "hiddenActiveCellsValue", 0, "Hidden Active Cells Value", "", "", "");
    RICF_InitField(&m_inactiveCellsValue, "inactiveCellsValue", 0, "Inactive Cells Value", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportVisibleCells::execute()
{
    if (m_caseId < 0 || m_viewName().isEmpty())
    {
        RiaLogging::error("exportVisibleCells: CaseId or view name not specified");
        return;
    }

    auto eclipseView = RicfApplicationTools::viewFromCaseIdAndViewName(m_caseId, m_viewName);
    if (!eclipseView)
    {
        RiaLogging::error(QString("exportVisibleCells: Could not find view '%1' in case ID %2").arg(m_viewName).arg(m_caseId));
        return;
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::CELLS);
    if (exportFolder.isNull())
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("visibleCells");
    }

    RicSaveEclipseInputVisibleCellsUi exportSettings;
    buildExportSettings(exportFolder, &exportSettings);
    RicSaveEclipseInputVisibleCellsFeature::executeCommand(eclipseView, exportSettings, "exportVisibleCells");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportVisibleCells::buildExportSettings(const QString& exportFolder, RicSaveEclipseInputVisibleCellsUi* exportSettings)
{
    QDir baseDir(exportFolder);
    exportSettings->exportFilename = baseDir.absoluteFilePath(QString("%1.grdecl").arg(m_exportKeyword().text()));

    if (m_exportKeyword == ExportKeyword::FLUXNUM) exportSettings->exportKeyword = RicSaveEclipseInputVisibleCellsUi::FLUXNUM;
    else if (m_exportKeyword == ExportKeyword::MULTNUM) exportSettings->exportKeyword = RicSaveEclipseInputVisibleCellsUi::MULTNUM;

    exportSettings->visibleActiveCellsValue = m_visibleActiveCellsValue;
    exportSettings->hiddenActiveCellsValue = m_hiddenActiveCellsValue;
    exportSettings->inactiveCellsValue = m_inactiveCellsValue;
}
