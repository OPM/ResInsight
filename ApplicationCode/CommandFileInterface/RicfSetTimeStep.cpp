#include "RicfSetTimeStep.h"
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

#include "RicfSetTimeStep.h"

#include "RimProject.h"
#include "RimOilField.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimView.h"

#include "RiaApplication.h"

CAF_PDM_SOURCE_INIT(RicfSetTimeStep, "setTimeStep");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfSetTimeStep::RicfSetTimeStep()
{
    RICF_InitField(&m_caseId,        "case",      -1, "Case ID",  "", "", "");
    RICF_InitField(&m_timeStepIndex, "timeStep",  -1, "Time Step Index",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfSetTimeStep::execute()
{
    RimEclipseCase* eclipseCase;

    for (RimEclipseCase* c : RiaApplication::instance()->project()->activeOilField()->analysisModels()->cases)
    {
        if (c->caseId == m_caseId)
        {
            eclipseCase = c;
            break;
        }
    }

    for (RimView* view : eclipseCase->views())
    {
        view->setCurrentTimeStepAndUpdate(m_timeStepIndex);
    }
}
