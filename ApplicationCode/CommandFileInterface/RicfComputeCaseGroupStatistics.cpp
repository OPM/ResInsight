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

#include "RicfComputeCaseGroupStatistics.h"

#include "RimProject.h"
#include "RimOilField.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimCaseCollection.h"
#include "RimView.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

CAF_PDM_SOURCE_INIT(RicfComputeCaseGroupStatistics, "computeCaseGroupStatistics");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfComputeCaseGroupStatistics::RicfComputeCaseGroupStatistics()
{
    RICF_InitField(&m_caseIds, "caseIds", std::vector<int>(), "Case IDs",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfComputeCaseGroupStatistics::execute()
{
    for (int caseId : m_caseIds())
    {
        bool foundCase = false;
        for (RimIdenticalGridCaseGroup* group : RiaApplication::instance()->project()->activeOilField()->analysisModels()->caseGroups)
        {
            for (RimEclipseCase* c : group->statisticsCaseCollection->reservoirs)
            {
                if (c->caseId == caseId)
                {
                    RimEclipseStatisticsCase* statsCase = dynamic_cast<RimEclipseStatisticsCase*>(c);
                    if (statsCase)
                    {
                        statsCase->computeStatisticsAndUpdateViews();
                    }
                    else
                    {
                        RiaLogging::warning(QString("computeCaseGroupStatistics: Found case with ID %1, but it is not a statistics case, cannot compute statistics.").arg(caseId));
                    }
                    foundCase = true;
                    break;
                }
            }

            if (foundCase) break;
        }

        if (!foundCase)
        {
            RiaLogging::warning(QString("computeCaseGroupStatistics: Could not find statistics case with ID %1.").arg(caseId));
        }
    }
}
