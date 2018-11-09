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

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"

#include "ExportCommands/RicExportLgrFeature.h"

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

#include <QStringList>
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
    RICF_InitField(&m_splitType,            "splitType",            Lgr::SplitTypeEnum(),   "SplitType",        "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportLgrForCompletions::execute()
{
    using TOOLS = RicfApplicationTools;

    std::vector<RimWellPath*> wellPaths;

    // Find well paths
    {
        QStringList wellsNotFound;
        wellPaths = TOOLS::wellPathsFromNames(TOOLS::toQStringList(m_wellPathNames), &wellsNotFound);
        if (!wellsNotFound.empty())
        {
            RiaLogging::error(QString("exportLgrForCompletions: These well paths were not found: ") + wellsNotFound.join(", "));
        }
    }

    if (!wellPaths.empty())
    {
        QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::LGRS);
        if (exportFolder.isNull())
        {
            exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("LGR");
        }

        caf::CmdFeatureManager*                 commandManager = caf::CmdFeatureManager::instance();
        auto feature = dynamic_cast<RicExportLgrFeature*>(commandManager->getCommandFeature("RicExportLgrFeature"));

        RimEclipseCase* eclipseCase = TOOLS::caseFromId(m_caseId());
        if (!eclipseCase)
        {
            RiaLogging::error(QString("exportLgrForCompletions: Could not find case with ID %1").arg(m_caseId()));
            return;
        }

        caf::VecIjk lgrCellCounts(m_refinementI, m_refinementJ, m_refinementK);
        QStringList wellsIntersectingOtherLgrs;

        feature->exportLgrsForWellPaths(exportFolder, wellPaths, eclipseCase, m_timeStep, lgrCellCounts, m_splitType(),
            {RigCompletionData::PERFORATION, RigCompletionData::FRACTURE, RigCompletionData::FISHBONES}, &wellsIntersectingOtherLgrs);

        if (!wellsIntersectingOtherLgrs.empty())
        {
            auto wellsList = wellsIntersectingOtherLgrs.join(", ");
            RiaLogging::error("exportLgrForCompletions: No export for some wells due to existing intersecting LGR(s).Affected wells : " + wellsList);
        }
    }
}
