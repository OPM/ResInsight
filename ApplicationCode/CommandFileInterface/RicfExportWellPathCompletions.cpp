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

#include "RicfExportWellPathCompletions.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimDialogData.h"
#include "RimProject.h"
#include "RimOilField.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimWellPathCollection.h"
#include "RimWellPath.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

CAF_PDM_SOURCE_INIT(RicfExportWellPathCompletions, "exportWellPathCompletions");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportWellPathCompletions::RicfExportWellPathCompletions()
{
    RICF_InitField(&m_caseId,                      "caseId",                      -1,                                                     "Case ID",  "", "", "");
    RICF_InitField(&m_timeStep,                    "timeStep",                    -1,                                                     "Time Step Index",  "", "", "");
    RICF_InitField(&m_wellPathNames,               "wellPathNames",               std::vector<QString>(),                                 "Well Path Names",  "", "", "");
    RICF_InitField(&m_wellSelection,               "wellSelection",               RicExportCompletionDataSettingsUi::WellSelectionType(), "Well Selection",  "", "", "");
    RICF_InitField(&m_fileSplit,                   "fileSplit",                   RicExportCompletionDataSettingsUi::ExportSplitType(),   "File Split",  "", "", "");
    RICF_InitField(&m_compdatExport,               "compdatExport",               RicExportCompletionDataSettingsUi::CompdatExportType(), "Compdat Export",  "", "", "");
    RICF_InitField(&m_includePerforations,         "includePerforations",         true,                                                   "Include Perforations",  "", "", "");
    RICF_InitField(&m_includeFishbones,            "includeFishbones",            true,                                                   "Include Fishbones",  "", "", "");
    RICF_InitField(&m_excludeMainBoreForFishbones, "excludeMainBoreForFishbones", false,                                                  "Exclude Main Bore for Fishbones",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportWellPathCompletions::execute()
{
    RimProject* project = RiaApplication::instance()->project();
    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData();
    exportSettings->timeStep = m_timeStep;
    exportSettings->fileSplit = m_fileSplit;
    exportSettings->compdatExport = m_compdatExport;
    exportSettings->includePerforations = m_includePerforations;
    exportSettings->includeFishbones = m_includeFishbones;
    exportSettings->excludeMainBoreForFishbones = m_excludeMainBoreForFishbones;

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
            RiaLogging::error(QString("exportWellPathCompletions: Could not find case with ID %1").arg(m_caseId()));
            return;
        }
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::COMPLETIONS);
    if (exportFolder.isNull())
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("completions");
    }
    exportSettings->folder = exportFolder;

    std::vector<RimWellPath*> wellPaths;
    if (m_wellPathNames().empty())
    {
        std::copy(RiaApplication::instance()->project()->activeOilField()->wellPathCollection->wellPaths().begin(),
                  RiaApplication::instance()->project()->activeOilField()->wellPathCollection->wellPaths().end(),
                  std::back_inserter(wellPaths));
    }
    else
    {
        for (const QString& wellPathName : m_wellPathNames())
        {
            RimWellPath* wellPath = RiaApplication::instance()->project()->activeOilField()->wellPathCollection->wellPathByName(wellPathName);
            if (wellPath)
            {
                wellPaths.push_back(wellPath);
            }
            else
            {
                RiaLogging::warning(QString("exportWellPathCompletions: Could not find well path with name %1").arg(wellPathName));
            }
        }
    }

    std::vector<RimSimWellInView*> simWells;

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions(wellPaths, simWells, *exportSettings);
}
