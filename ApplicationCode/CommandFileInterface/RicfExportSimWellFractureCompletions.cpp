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

#include "RicfExportSimWellFractureCompletions.h"

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

CAF_PDM_SOURCE_INIT(RicfExportSimWellFractureCompletions, "exportSimWellFractureCompletions");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportSimWellFractureCompletions::RicfExportSimWellFractureCompletions()
{
    RICF_InitField(&m_caseId,          "caseId",                -1,                                                     "Case ID",  "", "", "");
    RICF_InitField(&m_viewName,        "viewName",              QString(""),                                            "View Name", "", "", "");
    RICF_InitField(&m_timeStep,        "timeStep",              -1,                                                     "Time Step Index",  "", "", "");
    RICF_InitField(&m_simWellNames,    "simulationWellNames",   std::vector<QString>(),                                 "Simulation Well Names",  "", "", "");
    RICF_InitField(&m_fileSplit,       "fileSplit",             RicExportCompletionDataSettingsUi::ExportSplitType(),   "File Split",  "", "", "");
    RICF_InitField(&m_compdatExport,   "compdatExport",         RicExportCompletionDataSettingsUi::CompdatExportType(), "Compdat Export",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportSimWellFractureCompletions::execute()
{
    RimProject* project = RiaApplication::instance()->project();
    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData();
    
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

    std::vector<RimEclipseView*> views;
    for (Rim3dView* v : exportSettings->caseToApply->views())
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(v);
        if (eclipseView && eclipseView->name() == m_viewName())
        {
            views.push_back(eclipseView);
        }
    }
    if (views.empty())
    {
        RiaLogging::error(QString("exportSimWellCompletions: Could not find any views named \"%1\" in the case with ID %2").arg(m_viewName).arg(m_caseId()));
        return;
    }

    std::vector<RimSimWellInView*> simWells;
    if (m_simWellNames().empty())
    {
        for (RimEclipseView* view : views)
        {
            std::copy(view->wellCollection()->wells.begin(),
                      view->wellCollection()->wells.end(),
                      std::back_inserter(simWells));
        }
    }
    else
    {
        for (const QString& wellPathName : m_simWellNames())
        {
            for (RimEclipseView* view : views)
            {
                RimSimWellInView* simWell = view->wellCollection()->findWell(wellPathName);
                if (simWell)
                {
                    simWells.push_back(simWell);
                }
                else
                {
                    RiaLogging::warning(QString("exportSimWellCompletions: Could not find well with name %1 in view \"%2\" on case with ID %2").arg(wellPathName).arg(m_viewName).arg(m_caseId()));
                }
            }
        }
    }

    std::vector<RimWellPath*> wellPaths;

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions(wellPaths, simWells, *exportSettings);
}
