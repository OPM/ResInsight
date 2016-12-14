/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimFlowDiagSolution.h"
#include "RimEclipseResultCase.h"
#include "RigCaseData.h"

CAF_PDM_SOURCE_INIT(RimFlowDiagSolution, "FlowDiagSolution");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::RimFlowDiagSolution(void)
{
    CAF_PDM_InitObject("Flow Diagnostics Solution", "", "", "");

    //CAF_PDM_InitFieldNoDefault(&m_selectedWells, "SelectedWells", "Selected Wells","","");

    CAF_PDM_InitField(&m_userDescription, "UserDescription", QString("All Wells") ,"Description", "", "","");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::~RimFlowDiagSolution(void)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QString> RimFlowDiagSolution::tracerNames()
{
    RimEclipseResultCase* eclCase; 
    this->firstAncestorOrThisOfType(eclCase);

    std::set<QString> tracerNameSet;
    
    if (eclCase)
    {
        const cvf::Collection<RigSingleWellResultsData>& wellResults = eclCase->reservoirData()->wellResults();   

        for (size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx)
        {
            tracerNameSet.insert(wellResults[wIdx]->m_wellName); 
        }
    }

    return tracerNameSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::TracerStatusType RimFlowDiagSolution::tracerStatus(QString tracerName)
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfType(eclCase);
    TracerStatusType tracerStatus = UNDEFINED;
    if ( eclCase )
    {
        const cvf::Collection<RigSingleWellResultsData>& wellResults = eclCase->reservoirData()->wellResults();

        for ( size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx )
        {
            if(wellResults[wIdx]->m_wellName == tracerName)
            {
               for (const RigWellResultFrame& wellResFrame : wellResults[wIdx]->m_wellCellsTimeSteps)
               {
                    if (wellResFrame.m_productionType == RigWellResultFrame::GAS_INJECTOR 
                    || wellResFrame.m_productionType == RigWellResultFrame::OIL_INJECTOR
                    ||  wellResFrame.m_productionType == RigWellResultFrame::WATER_INJECTOR)
                    {
                        if (tracerStatus == PRODUCER) tracerStatus = VARYING;
                        else tracerStatus = INJECTOR;
                    }
                    else if (wellResFrame.m_productionType == RigWellResultFrame::PRODUCER)
                    {
                        if ( tracerStatus == INJECTOR ) tracerStatus = VARYING;
                        else tracerStatus = PRODUCER;
                    }
                    
                    if (tracerStatus == VARYING) break;
               }

               break;
            }
        }
    }

    return tracerStatus;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFlowDiagSolution::userDescriptionField()
{
    return &m_userDescription;
}

