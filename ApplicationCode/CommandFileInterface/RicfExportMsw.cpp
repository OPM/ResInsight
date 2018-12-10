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

#include "RicfExportMsw.h"

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimOilField.h"
#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"

#include "CompletionExportCommands/RicExportFishbonesWellSegmentsFeature.h"

CAF_PDM_SOURCE_INIT(RicfExportMsw, "exportMsw");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportMsw::RicfExportMsw()
{
    RICF_InitField(&m_caseId,       "caseId",   -1,        "Case ID", "", "", "");
    RICF_InitField(&m_wellPathName, "wellPath", QString(), "Well Path Name", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportMsw::execute()
{
    using TOOLS = RicfApplicationTools;

    RicCaseAndFileExportSettingsUi exportSettings;

    auto eclipseCase = TOOLS::caseFromId(m_caseId());
    if (!eclipseCase)
    {
        RiaLogging::error(QString("exportMsw: Could not find case with ID %1.").arg(m_caseId()));
        return;
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::COMPLETIONS);
    if (exportFolder.isNull())
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("completions");
    }
    exportSettings.folder = exportFolder;

    RimWellPath* wellPath = RiaApplication::instance()->project()->wellPathByName(m_wellPathName);
    if (!wellPath)
    {
        RiaLogging::error(QString("exportMsw: Could not find well path with name %1").arg(m_wellPathName()));
        return;
    }

    if (!wellPath->fishbonesCollection()->activeFishbonesSubs().empty())
    {
        RicExportFishbonesWellSegmentsFeature::exportWellSegments(wellPath, wellPath->fishbonesCollection()->activeFishbonesSubs(), exportSettings);
    }
}
