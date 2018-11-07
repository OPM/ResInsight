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

#include "RicfCreateLgrForCompletions.h"

#include "RicfCommandFileExecutor.h"
#include "RicfCreateMultipleFractures.h"

#include "RicDeleteTemporaryLgrsFeature.h"
#include "RicCreateTemporaryLgrFeature.h"
#include "ExportCommands/RicExportLgrFeature.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaWellNameComparer.h"

#include "cafCmdFeatureManager.h"
#include <QStringList>

CAF_PDM_SOURCE_INIT(RicfCreateLgrForCompletions, "createLgrForCompletions");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateLgrForCompletions::RicfCreateLgrForCompletions()
{
    RICF_InitField(&m_caseId, "caseId", -1, "Case ID", "", "", "");
    RICF_InitField(&m_timeStep, "timeStep", 0, "Time Step Index", "", "", "");
    RICF_InitField(&m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names", "", "", "");
    RICF_InitField(&m_refinementI, "refinementI", -1, "RefinementI", "", "", "");
    RICF_InitField(&m_refinementJ, "refinementJ", -1, "RefinementJ", "", "", "");
    RICF_InitField(&m_refinementK, "refinementK", -1, "RefinementK", "", "", "");
    RICF_InitField(&m_splitType, "splitType", Lgr::SplitTypeEnum(), "SplitType", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfCreateLgrForCompletions::execute()
{
    const auto wellPaths = RicfCreateMultipleFractures::wellPaths(m_wellPathNames);
    if (!wellPaths.empty())
    {
        caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
        auto feature = dynamic_cast<RicCreateTemporaryLgrFeature*>(commandManager->getCommandFeature("RicCreateTemporaryLgrFeature"));

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
                RiaLogging::error(QString("createLgrForCompletions: Could not find case with ID %1").arg(m_caseId()));
                return;
            }
        }

        RicDeleteTemporaryLgrsFeature::deleteAllTemporaryLgrs(eclipseCase);

        caf::VecIjk lgrCellCounts(m_refinementI, m_refinementJ, m_refinementK);
        QStringList wellsWithIntersectingLgrs;
        for (const auto wellPath : wellPaths)
        {
            if (wellPath)
            {
                bool intersectingLgrs = false;
                feature->createLgrsForWellPath(
                    wellPath,
                    eclipseCase,
                    m_timeStep,
                    lgrCellCounts,
                    m_splitType(),
                    {RigCompletionData::PERFORATION, RigCompletionData::FRACTURE, RigCompletionData::FISHBONES},
                    &intersectingLgrs);

                if (intersectingLgrs) wellsWithIntersectingLgrs.push_back(wellPath->name());
            }
        }

        feature->updateViews(eclipseCase);

        if (!wellsWithIntersectingLgrs.empty())
        {
            auto wellsList = wellsWithIntersectingLgrs.join(", ");
            RiaLogging::error(
                "createLgrForCompletions: No LGRs created for some wells due to existing intersecting LGR(s).Affected wells : " +
                wellsList);
        }
    }
}
