#include "RicfCreateSaturationPressurePlots.h"
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

#include "RicfCreateSaturationPressurePlots.h"

#include "GridCrossPlotCommands/RicCreateSaturationPressurePlotsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSaturationPressurePlotCollection.h"

CAF_PDM_SOURCE_INIT(RicfCreateSaturationPressurePlots, "createSaturationPressurePlots");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateSaturationPressurePlots::RicfCreateSaturationPressurePlots()
{
    RICF_InitField(&m_caseIds, "caseIds", std::vector<int>(), "Case IDs", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfCreateSaturationPressurePlots::execute()
{
    std::vector<int> caseIds = m_caseIds();
    if (caseIds.empty())
    {
        RimProject* project = RiaApplication::instance()->project();
        if (project)
        {
            auto eclipeCases = project->eclipseCases();
            for (auto c : eclipeCases)
            {
                caseIds.push_back(c->caseId());
            }
        }
    }

    RimProject* project = RiaApplication::instance()->project();
    if (project)
    {
        auto eclipeCases = project->eclipseCases();
        for (auto c : eclipeCases)
        {
            auto eclipseResultCase = dynamic_cast<RimEclipseResultCase*>(c);
            if (!eclipseResultCase) continue;

            for (auto caseId : caseIds)
            {
                if (c->caseId == caseId)
                {
                    RicCreateSaturationPressurePlotsFeature::createPlots(eclipseResultCase);
                }
            }
        }
    }

    RimSaturationPressurePlotCollection* collection = project->mainPlotCollection()->saturationPressurePlotCollection();
    collection->updateAllRequiredEditors();
    RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
}
