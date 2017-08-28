/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigNumberOfFloodedPoreVolumesCalculator.h"

#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilderMock.h"

#include "RimEclipseCase.h"
#include "RimReservoirCellResultsStorage.h"

#include <string>
#include <vector>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigNumberOfFloodedPoreVolumesCalculator::RigNumberOfFloodedPoreVolumesCalculator(RigMainGrid* mainGrid, 
                                                                                 RimEclipseCase* caseToApply, 
                                                                                 std::vector<std::string> tracerNames )
{
    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();
    RiaDefines::PorosityModelType porosityModel = RiaDefines::MATRIX_MODEL;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(porosityModel);

    size_t scalarResultIndexPorv = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORV");
    const std::vector<double>* porvResults = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexPorv, 0));  

    std::vector<size_t> scalarResultIndexTracers;
    for (std::string tracerName : tracerNames)
    {
        scalarResultIndexTracers.push_back(gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATI+"));
    }

    //TODO: Option for Oil and Gas instead of water
    size_t scalarResultIndexFlowrateI = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATI+");
    size_t scalarResultIndexFlowrateJ = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATJ+");
    size_t scalarResultIndexFlowrateK = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATK+");

    std::vector<const std::vector<double>* > flowrateIatAllTimeSteps;
    std::vector<const std::vector<double>* > flowrateJatAllTimeSteps;
    std::vector<const std::vector<double>* > flowrateKatAllTimeSteps;

    RigNNCData* nncData = eclipseCaseData->mainGrid()->nncData();
    const std::vector<RigConnection> connections = nncData->connections();

    //TODO: oil or gas flowrate
    std::vector<const std::vector<double>* > flowrateNNCatAllTimeSteps;
    QString nncConnectionProperty = mainGrid->nncData()->propertyNameFluxWat();

    
    std::vector<double> daysSinceSimulationStart = caseToApply->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->daysSinceSimulationStart();

    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        const std::vector<double>* flowrateI = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlowrateI, 
                                                                                                                       timeStep));
        flowrateIatAllTimeSteps.push_back(flowrateI);
        const std::vector<double>* flowrateJ = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlowrateJ, 
                                                                                                                       timeStep));
        flowrateJatAllTimeSteps.push_back(flowrateJ);
        const std::vector<double>* flowrateK = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlowrateK, 
                                                                                                                       timeStep));
        flowrateKatAllTimeSteps.push_back(flowrateK);

        const std::vector<double>* connectionFlowrate = nncData->dynamicConnectionScalarResultByName(nncConnectionProperty,
                                                                                                     timeStep);
        flowrateNNCatAllTimeSteps.push_back(connectionFlowrate);
    }

    size_t totalNumberOfCells = porvResults->size();

    std::vector<std::vector<double>> cellQwInAtAllTimeSteps;
    std::vector<double> cellQwInTimeStep0(totalNumberOfCells);
    cellQwInAtAllTimeSteps.push_back(cellQwInTimeStep0);

    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        double daysSinceSimStartNow = daysSinceSimulationStart[timeStep];
        double daysSinceSimStartLastTimeStep = daysSinceSimulationStart[timeStep -1];
        double deltaT = daysSinceSimStartNow - daysSinceSimStartLastTimeStep;

        const std::vector<double>* flowrateI = flowrateIatAllTimeSteps[timeStep];
        const std::vector<double>* flowrateJ = flowrateJatAllTimeSteps[timeStep];
        const std::vector<double>* flowrateK = flowrateKatAllTimeSteps[timeStep];

        const std::vector<double>* flowrateNNC = flowrateNNCatAllTimeSteps[timeStep];

        std::vector<double> flowrateIntoCell(totalNumberOfCells);

        std::vector<double> summedTracerValues(totalNumberOfCells);
        //sum all tracers at current timestep
        for (size_t tracerIndex : scalarResultIndexTracers)
        {
            const std::vector<double>* tracerResult = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(tracerIndex, timeStep));

            for (size_t i = 0; i < summedTracerValues.size(); i++)
            {
                summedTracerValues[i] += tracerResult->at(i);
            }
        }

        distributeNeighbourCellFlow(mainGrid,
                                    totalNumberOfCells, 
                                    summedTracerValues, 
                                    flowrateI, 
                                    flowrateJ, 
                                    flowrateK, 
                                    flowrateIntoCell);

        distributeNNCflow(connections, 
                          summedTracerValues, 
                          flowrateNNC, 
                          flowrateIntoCell);

        std::vector<double> CellQwIn(totalNumberOfCells);

        for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
        {
            CellQwIn[globalCellIndex] = cellQwInAtAllTimeSteps[timeStep-1][globalCellIndex] 
                + (flowrateIntoCell[globalCellIndex]) * deltaT;
        }
        cellQwInAtAllTimeSteps.push_back(CellQwIn);

    }

    //Calculate number-of-cell-PV flooded
    std::vector<double> cumWinflowPVTimeStep0(totalNumberOfCells);
    m_cumWinflowPVAllTimeSteps.clear();
    m_cumWinflowPVAllTimeSteps.push_back(cumWinflowPVTimeStep0);

    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
        {
            m_cumWinflowPVAllTimeSteps[timeStep][globalCellIndex]  = cellQwInAtAllTimeSteps[timeStep][globalCellIndex] 
                                                                   / porvResults->at(globalCellIndex);

        }
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNumberOfFloodedPoreVolumesCalculator::distributeNNCflow(std::vector<RigConnection> connections,
                                                                std::vector<double> summedTracerValues,
                                                                const std::vector<double>* flowrateNNC, 
                                                                std::vector<double> &flowrateIntoCell)
{
    for (size_t connectionIndex = 0; connectionIndex < connections.size(); connectionIndex++)
    {
        RigConnection connection = connections[connectionIndex];
        double connectionValue = flowrateNNC->at(connectionIndex);

        size_t cell1Index = connection.m_c1GlobIdx;
        size_t cell2Index = connection.m_c2GlobIdx;

        if (connectionValue > 0)
        {
            //Flow out of cell with cell1index, into cell cell2index
            flowrateIntoCell[cell2Index] += connectionValue * summedTracerValues[cell1Index];
        }
        else if (connectionValue < 0)
        {
            //flow out of cell with cell2index, into cell cell1index
            flowrateIntoCell[cell1Index] += connectionValue * summedTracerValues[cell2Index];
        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNumberOfFloodedPoreVolumesCalculator::distributeNeighbourCellFlow(RigMainGrid* mainGrid,
                                                                          size_t totalNumberOfCells,
                                                                          std::vector<double> summedTracerValues,
                                                                          const std::vector<double>* flrWatResultI,
                                                                          const std::vector<double>* flrWatResultJ,
                                                                          const std::vector<double>* flrWatResultK,
                                                                          std::vector<double> &totalFlowrateIntoCell)
{
    for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
    {
        size_t i, j, k;
        mainGrid->ijkFromCellIndex(globalCellIndex, &i, &j, &k); //TODO: Generalize grid!!!!

        if (i < mainGrid->cellCountI())
        {
            size_t globalCellIndexPosINeightbour = mainGrid->cellIndexFromIJK(i + 1, j, k);

            if (flrWatResultI->at(globalCellIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[globalCellIndexPosINeightbour] += flrWatResultI->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultI->at(globalCellIndex) < 0 && i < mainGrid->cellCountI())
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[globalCellIndex] += (-1) * flrWatResultI->at(globalCellIndex) * summedTracerValues[globalCellIndexPosINeightbour];
            }
        }
    }

    for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
    {
        size_t i, j, k;
        mainGrid->ijkFromCellIndex(globalCellIndex, &i, &j, &k); //TODO: Generalize grid!!!!

        if (i < mainGrid->cellCountI())
        {
            size_t globalCellIndexPosJNeightbour = mainGrid->cellIndexFromIJK(i, j + 1, k);

            if (flrWatResultJ->at(globalCellIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[globalCellIndexPosJNeightbour] += flrWatResultJ->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultJ->at(globalCellIndex) < 0 && i < mainGrid->cellCountI())
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[globalCellIndex] += flrWatResultJ->at(globalCellIndex) * summedTracerValues[globalCellIndexPosJNeightbour];
            }
        }
    }

    for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
    {
        size_t i, j, k;
        mainGrid->ijkFromCellIndex(globalCellIndex, &i, &j, &k); //TODO: Generalize grid!!!!

        if (i < mainGrid->cellCountI())
        {
            size_t globalCellIndexPosKNeightbour = mainGrid->cellIndexFromIJK(i, j, k + 1);

            if (flrWatResultK->at(globalCellIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[globalCellIndexPosKNeightbour] += flrWatResultK->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultK->at(globalCellIndex) < 0 && i < mainGrid->cellCountI())
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[globalCellIndex] += flrWatResultK->at(globalCellIndex) * summedTracerValues[globalCellIndexPosKNeightbour];
            }
        }
    }
}
