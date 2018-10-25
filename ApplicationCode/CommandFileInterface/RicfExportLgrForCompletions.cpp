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

#include "RicfExportLgrForCompletions.h"

#include "RicfCommandFileExecutor.h"
#include "RicfCreateMultipleFractures.h"

#include "ExportCommands/RicExportLgrFeature.h"
#include "ExportCommands/RicExportLgrUi.h"

#include "RimProject.h"
#include "RimDialogData.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimWellPath.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaWellNameComparer.h"

#include "cafCmdFeatureManager.h"


CAF_PDM_SOURCE_INIT(RicfExportLgrForCompletions, "exportLgrForCompletions");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportLgrForCompletions::RicfExportLgrForCompletions()
{
    RICF_InitField(&m_caseId,               "caseId",               -1,                     "Case ID",          "", "", "");
    RICF_InitField(&m_timeStep,             "timeStep",             -1,                     "Time Step Index",  "", "", "");
    RICF_InitField(&m_wellPathNames,        "wellPathNames",        std::vector<QString>(), "Well Path Names",  "", "", "");
    RICF_InitField(&m_refinementI,          "refinementI",          -1,                     "RefinementI",      "", "", "");
    RICF_InitField(&m_refinementJ,          "refinementJ",          -1,                     "RefinementJ",      "", "", "");
    RICF_InitField(&m_refinementK,          "refinementK",          -1,                     "RefinementK",      "", "", "");
    RICF_InitField(&m_splitType,            "splitType",            LgrSplitType(),         "SplitType", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportLgrForCompletions::execute()
{
    const auto wellPaths = RicfCreateMultipleFractures::wellPaths(m_wellPathNames);
    if (!wellPaths.empty())
    {
        QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::LGRS);
        if (exportFolder.isNull())
        {
            exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("LGR");
        }

        caf::CmdFeatureManager*                 commandManager = caf::CmdFeatureManager::instance();
        auto feature = dynamic_cast<RicExportLgrFeature*>(commandManager->getCommandFeature("RicExportLgrFeature"));

        RimEclipseCase* eclipseCase = nullptr;
        {
            for (RimEclipseCase* c : RiaApplication::instance()->project()->activeOilField()->analysisModels->cases())
            {
                if (c->caseId() == m_caseId())
                {
                    eclipseCase = c;
                    break;
                }
            }
            if (!eclipseCase)
            {
                RiaLogging::error(QString("exportLgrForCompletions: Could not find case with ID %1").arg(m_caseId()));
                return;
            }
        }

        caf::VecIjk lgrCellCounts(m_refinementI, m_refinementJ, m_refinementK);
        bool lgrIntersected = false;
        for (const auto wellPath : wellPaths)
        {
            if (wellPath)
            {
                try
                {
                    auto completionTypes = (RicExportLgrUi::CompletionType)(RicExportLgrUi::CT_PERFORATION | RicExportLgrUi::CT_FRACTURE | RicExportLgrUi::CT_FISHBONE);
                    feature->exportLgrsForWellPath(exportFolder, wellPath, eclipseCase, m_timeStep, lgrCellCounts, m_splitType(),
                                                   completionTypes);
                }
                catch(CreateLgrException e)
                {
                    lgrIntersected = true;
                }
            }
        }

        if (lgrIntersected)
        {
            RiaLogging::error("exportLgrForCompletions: At least one completion intersects with an LGR. No output for those completions produced");
        }
    }
}
