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

#include "RiaApplication.h"

#include "RiuMainWindow.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RicfExportSnapshots, "exportSnapshots");

namespace caf
{
template<>
void RicfExportSnapshots::SnapshotsTypeEnum::setUp()
{
    addItem(RicfExportSnapshots::ALL, "ALL", "All");
    addItem(RicfExportSnapshots::VIEWS, "VIEWS", "Views");
    addItem(RicfExportSnapshots::PLOTS, "PLOTS", "Plots");
    setDefault(RicfExportSnapshots::ALL);
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportSnapshots::RicfExportSnapshots()
{
    RICF_InitField(&m_type, "type", RicfExportSnapshots::SnapshotsTypeEnum(), "Type", "", "", "");
    RICF_InitField(&m_prefix, "prefix", QString(), "Prefix", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfExportSnapshots::execute()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    CVF_ASSERT(mainWnd);
    mainWnd->hideAllDockWindows();
    RiaApplication::instance()->processEvents();

    QString absolutePathToSnapshotDir = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::SNAPSHOTS);
    if (absolutePathToSnapshotDir.isNull())
    {
        absolutePathToSnapshotDir = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("snapshots");
    }
    if (m_type == RicfExportSnapshots::VIEWS || m_type == RicfExportSnapshots::ALL)
    {
        RicSnapshotAllViewsToFileFeature::exportSnapshotOfAllViewsIntoFolder(absolutePathToSnapshotDir, m_prefix);
    }
    if (m_type == RicfExportSnapshots::PLOTS || m_type == RicfExportSnapshots::ALL)
    {
        RicSnapshotAllPlotsToFileFeature::exportSnapshotOfAllPlotsIntoFolder(absolutePathToSnapshotDir, m_prefix);
    }

    mainWnd->loadWinGeoAndDockToolBarLayout();
    RiaApplication::instance()->processEvents();
}
