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

#include "CompletionCommands/RicExportFishbonesWellSegmentsFeature.h"

CAF_PDM_SOURCE_INIT(RicfExportMsw, "exportMsw");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportMsw::RicfExportMsw()
{
    RICF_InitField(&m_caseId,       "case",     -1,        "Case", "", "", "");
    RICF_InitField(&m_wellPathName, "wellPath", QString(), "Case", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportMsw::execute()
{
    RicCaseAndFileExportSettingsUi exportSettings;

    {
        bool foundCase = false;
        for (RimEclipseCase* c : RiaApplication::instance()->project()->activeOilField()->analysisModels->cases())
        {
            if (c->caseId() == m_caseId())
            {
                exportSettings.caseToApply = c;
                foundCase = true;
                break;
            }
        }

        if (!foundCase)
        {
            RiaLogging::error(QString("exportMsw: Could not find case with ID %1.").arg(m_caseId()));
            return;
        }
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::COMPLETIONS);
    if (exportFolder.isNull())
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("completions");
    }
    exportSettings.folder = exportFolder;

    RimWellPath* wellPath = RiaApplication::instance()->project()->activeOilField()->wellPathCollection->wellPathByName(m_wellPathName);
    if (!wellPath)
    {
        RiaLogging::error(QString("exportMsw: Could not find well path with name %1").arg(m_wellPathName()));
        return;
    }

    std::vector<RimFishbonesMultipleSubs*> fishbonesSubs;
    
    for (RimFishbonesMultipleSubs* fishbones : wellPath->fishbonesCollection()->fishbonesSubs())
    {
        fishbonesSubs.push_back(fishbones);
    }

    RicExportFishbonesWellSegmentsFeature::exportWellSegments(wellPath, fishbonesSubs, exportSettings);
}
