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

#include "RicfExportSimWellCompletions.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

CAF_PDM_SOURCE_INIT(RicfExportSimWellCompletions, "exportSimWellCompletions");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportSimWellCompletions::RicfExportSimWellCompletions()
{
    RICF_InitField(&m_caseId,                      "case",                        -1,                                                     "Case ID",  "", "", "");
    RICF_InitField(&m_timeStep,                    "timeStep",                    -1,                                                     "Time Step Index",  "", "", "");
    RICF_InitField(&m_wellPathNames,               "wellPathNames",               std::vector<QString>(),                                 "Well Path Names",  "", "", "");
    RICF_InitField(&m_wellSelection,               "wellSelection",               RicExportCompletionDataSettingsUi::WellSelectionType(), "Well Selection",  "", "", "");
    RICF_InitField(&m_fileSplit,                   "fileSplit",                   RicExportCompletionDataSettingsUi::ExportSplitType(),   "File Split",  "", "", "");
    RICF_InitField(&m_compdatExport,               "compdatExport",               RicExportCompletionDataSettingsUi::CompdatExportType(), "Compdat Export",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportSimWellCompletions::execute()
{
    RimProject* project = RiaApplication::instance()->project();
    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData(false);
    exportSettings->timeStep = m_timeStep;
    exportSettings->fileSplit = m_fileSplit;
    exportSettings->compdatExport = m_compdatExport;

    {
        bool foundCase = false;
        for (RimEclipseCase* c : RiaApplication::instance()->project()->activeOilField()->analysisModels->cases())
        {
            if (c->caseId() == m_caseId())
            {
                exportSettings->caseToApply = c;
                foundCase = true;
                break;
            }
        }
        if (!foundCase)
        {
            RiaLogging::error(QString("exportSimWellCompletions: Could not find case with ID %1").arg(m_caseId()));
            return;
        }
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::COMPLETIONS);
    if (exportFolder.isNull())
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("completions");
    }
    exportSettings->folder = exportFolder;

    // FIXME : Select correct view?
    RimEclipseView* view;
    for (Rim3dView* v : exportSettings->caseToApply->views())
    {
        view = dynamic_cast<RimEclipseView*>(v);
        if (view) break;
    }
    if (!view)
    {
        RiaLogging::error(QString("exportSimWellCompletions: Could not find a view for case with ID %1").arg(m_caseId()));
        return;
    }

    std::vector<RimSimWellInView*> simWells;
    if (m_wellPathNames().empty())
    {
        std::copy(view->wellCollection()->wells.begin(),
                  view->wellCollection()->wells.end(),
                  std::back_inserter(simWells));
    }
    else
    {
        for (const QString& wellPathName : m_wellPathNames())
        {
            RimSimWellInView* simWell = view->wellCollection()->findWell(wellPathName);
            if (simWell)
            {
                simWells.push_back(simWell);
            }
            else
            {
                RiaLogging::warning(QString("exportSimWellCompletions: Could not find well with name %1 on case with ID %2").arg(wellPathName).arg(m_caseId()));
            }
        }
    }

    std::vector<RimWellPath*> wellPaths;

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions(wellPaths, simWells, *exportSettings);
}
