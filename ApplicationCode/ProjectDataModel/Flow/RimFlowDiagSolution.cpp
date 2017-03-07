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

#include "RiaApplication.h"
#include "RiaColorTables.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"
#include "RigMainGrid.h"
#include "RigSingleWellResultsData.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"

CAF_PDM_SOURCE_INIT(RimFlowDiagSolution, "FlowDiagSolution");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool hasCrossFlowEnding(const QString& tracerName)
{
    return tracerName.endsWith("-Xf");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString removeCrossFlowEnding(const QString& tracerName)
{
    if (tracerName.endsWith("-Xf"))
    {
        return tracerName.left(tracerName.size() - 3);
    }
    else
    {
        return tracerName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString addCrossFlowEnding(const QString& wellName)
{
    return wellName + "-Xf";
}

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
RigFlowDiagResults* RimFlowDiagSolution::flowDiagResults()
{
    if ( m_flowDiagResults.isNull() )
    {
        size_t timeStepCount;
        {
            RimEclipseResultCase* eclCase;
            this->firstAncestorOrThisOfType(eclCase);
            
            CVF_ASSERT(eclCase && eclCase->reservoirData() );

            timeStepCount = eclCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->maxTimeStepCount();

        }

        m_flowDiagResults = new RigFlowDiagResults(this, timeStepCount);
    }

    return m_flowDiagResults.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFlowDiagSolution::tracerNames()
{
    RimEclipseResultCase* eclCase; 
    this->firstAncestorOrThisOfType(eclCase);

    std::vector<QString> tracerNameSet;
    
    if (eclCase)
    {
        const cvf::Collection<RigSingleWellResultsData>& wellResults = eclCase->reservoirData()->wellResults();   

        for (size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx)
        {
            tracerNameSet.push_back(wellResults[wIdx]->m_wellName); 
            tracerNameSet.push_back(addCrossFlowEnding(wellResults[wIdx]->m_wellName)); 
        }
    }

    return tracerNameSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<int> > RimFlowDiagSolution::allInjectorTracerActiveCellIndices(size_t timeStepIndex)
{
    return allTracerActiveCellIndices(timeStepIndex, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<int> > RimFlowDiagSolution::allProducerTracerActiveCellIndices(size_t timeStepIndex)
{
    return allTracerActiveCellIndices(timeStepIndex, false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<int> > RimFlowDiagSolution::allTracerActiveCellIndices(size_t timeStepIndex, bool useInjectors)
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfType(eclCase);

    std::map<std::string, std::vector<int> > tracersWithCells;

    if ( eclCase )
    {
        const cvf::Collection<RigSingleWellResultsData>& wellResults = eclCase->reservoirData()->wellResults();
        RigMainGrid* mainGrid = eclCase->reservoirData()->mainGrid();
        RigActiveCellInfo* activeCellInfo = eclCase->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS); //Todo: Must come from the results definition

        for ( size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx )
        {
            if (!wellResults[wIdx]->hasWellResult(timeStepIndex) ) continue;
            const RigWellResultFrame& wellResFrame = wellResults[wIdx]->wellResultFrame(timeStepIndex);
           
            bool isInjectorWell =  (   wellResFrame.m_productionType != RigWellResultFrame::PRODUCER
                                    && wellResFrame.m_productionType != RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE);

            std::string wellname   = wellResults[wIdx]->m_wellName.toStdString();
            std::string wellNameXf = addCrossFlowEnding(wellResults[wIdx]->m_wellName).toStdString();

            std::vector<int>& tracerCells = tracersWithCells[wellname];
            std::vector<int>& tracerCellsCrossFlow = tracersWithCells[wellNameXf];

            for (const RigWellResultBranch& wBr: wellResFrame.m_wellResultBranches)
            {
                for (const RigWellResultPoint& wrp: wBr.m_branchResultPoints)
                {
                    if (wrp.isValid() && wrp.m_isOpen 
                        && ( (useInjectors  && wrp.flowRate() < 0.0) || (!useInjectors && wrp.flowRate() > 0.0) ) )
                    {
                        RigGridBase * grid = mainGrid->gridByIndex(wrp.m_gridIndex);
                        size_t reservoirCellIndex = grid->reservoirCellIndex(wrp.m_gridCellIndex);

                        int cellActiveIndex = static_cast<int>(activeCellInfo->cellResultIndex(reservoirCellIndex));

                        if ( useInjectors == isInjectorWell )
                        {
                            tracerCells.push_back(cellActiveIndex);
                        }
                        else 
                        {
                            tracerCellsCrossFlow.push_back(cellActiveIndex);
                        }
                    }
                }
            }
        }
    }

    return tracersWithCells;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::TracerStatusType RimFlowDiagSolution::tracerStatusOverall(const QString& tracerName)
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfTypeAsserted(eclCase);

    TracerStatusType tracerStatus = UNDEFINED;

    const cvf::Collection<RigSingleWellResultsData>& wellResults = eclCase->reservoirData()->wellResults();

    for ( size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx )
    {
        QString wellName = removeCrossFlowEnding(tracerName);

        if ( wellResults[wIdx]->m_wellName != wellName ) continue;

        tracerStatus = CLOSED;
        for ( const RigWellResultFrame& wellResFrame : wellResults[wIdx]->m_wellCellsTimeSteps )
        {
            if ( wellResFrame.m_productionType == RigWellResultFrame::GAS_INJECTOR
                || wellResFrame.m_productionType == RigWellResultFrame::OIL_INJECTOR
                ||  wellResFrame.m_productionType == RigWellResultFrame::WATER_INJECTOR )
            {
                if ( tracerStatus == PRODUCER ) tracerStatus = VARYING;
                else tracerStatus = INJECTOR;
            }
            else if ( wellResFrame.m_productionType == RigWellResultFrame::PRODUCER )
            {
                if ( tracerStatus == INJECTOR ) tracerStatus = VARYING;
                else tracerStatus = PRODUCER;
            }
            if ( tracerStatus == VARYING ) break;
        }

        break;
    }

    if (hasCrossFlowEnding(tracerName))
    {
        if      (tracerStatus == PRODUCER) tracerStatus = INJECTOR;
        else if (tracerStatus == INJECTOR) tracerStatus = PRODUCER;
    }

    return tracerStatus;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::TracerStatusType RimFlowDiagSolution::tracerStatusInTimeStep(const QString& tracerName, size_t timeStepIndex)
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfTypeAsserted(eclCase);

    const cvf::Collection<RigSingleWellResultsData>& wellResults = eclCase->reservoirData()->wellResults();

    for ( size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx )
    {
        QString wellName = removeCrossFlowEnding(tracerName);

        if ( wellResults[wIdx]->m_wellName != wellName ) continue;
        if (!wellResults[wIdx]->hasWellResult(timeStepIndex))  return CLOSED;

        const RigWellResultFrame& wellResFrame = wellResults[wIdx]->wellResultFrame(timeStepIndex);

        if ( wellResFrame.m_productionType == RigWellResultFrame::GAS_INJECTOR
            || wellResFrame.m_productionType == RigWellResultFrame::OIL_INJECTOR
            ||  wellResFrame.m_productionType == RigWellResultFrame::WATER_INJECTOR )
        {
            if ( hasCrossFlowEnding(tracerName) )  return PRODUCER;

            return INJECTOR;
        }
        else if ( wellResFrame.m_productionType == RigWellResultFrame::PRODUCER
                 || wellResFrame.m_productionType == RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE )
        {
            if ( hasCrossFlowEnding(tracerName) )  return INJECTOR;

            return PRODUCER;
        }
        else
        {
            CVF_ASSERT(false);
        }
    }

    CVF_ASSERT(false);

    return UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFlowDiagSolution::tracerColor(const QString& tracerName)
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfType(eclCase);

    QString wellName = removeCrossFlowEnding(tracerName);

    if ( eclCase )
    {
        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());

        if (activeView)
        {
            RimEclipseWell* well = activeView->wellCollection->findWell(wellName);
            if (well)
            {
                return well->wellPipeColor();
            }
        }
        else
        {
            // If we do not find a well color, use index in well result data to be able to get variation of tracer colors
            // This can be the case if we do not have any views at all

            const cvf::Collection<RigSingleWellResultsData>& wellResults = eclCase->reservoirData()->wellResults();

            for ( size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx )
            {
                if ( wellResults[wIdx]->m_wellName == wellName )
                {
                    return RiaColorTables::wellsPaletteColors().cycledColor3f(wIdx);
                }
            }
        }
    }

    if (wellName == RIG_FLOW_TOTAL_NAME)        return cvf::Color3f::LIGHT_GRAY;
    if (wellName == RIG_RESERVOIR_TRACER_NAME)  return cvf::Color3f::LIGHT_GRAY;
    if (wellName == RIG_TINY_TRACER_GROUP_NAME) return cvf::Color3f::DARK_GRAY;

    return cvf::Color3f::LIGHT_GRAY;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFlowDiagSolution::userDescriptionField()
{
    return &m_userDescription;
}

