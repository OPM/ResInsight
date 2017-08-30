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
#include "RigMainGrid.h"
#include "RigReservoirBuilderMock.h"

#include "RimEclipseCase.h"
#include "RimReservoirCellResultsStorage.h"

#include <string>
#include <vector>
#include "RigActiveCellInfo.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigNumberOfFloodedPoreVolumesCalculator::RigNumberOfFloodedPoreVolumesCalculator(RigMainGrid* mainGrid, 
                                                                                 RimEclipseCase* caseToApply, 
                                                                                 std::vector<std::string> tracerNames)
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
    std::vector<std::vector<double> > summedTracersAtAllTimesteps;


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

    for (size_t timeStep = 0; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        const std::vector<double>* flowrateI = nullptr;
        if (scalarResultIndexFlowrateI != cvf::UNDEFINED_SIZE_T)
        {
            flowrateI = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlowrateI, 
                                                                                                timeStep));
        }
        flowrateIatAllTimeSteps.push_back(flowrateI);


        const std::vector<double>* flowrateJ = nullptr;
        if (scalarResultIndexFlowrateJ != cvf::UNDEFINED_SIZE_T)
        {
            flowrateI = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlowrateJ,
                                                                                                timeStep));
        }
        flowrateJatAllTimeSteps.push_back(flowrateJ);


        const std::vector<double>* flowrateK = nullptr;
        if (scalarResultIndexFlowrateK != cvf::UNDEFINED_SIZE_T)
        {
            flowrateK = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlowrateK,
                                                                                                timeStep));
        }
        flowrateKatAllTimeSteps.push_back(flowrateK);

        const std::vector<double>* connectionFlowrate = nncData->dynamicConnectionScalarResultByName(nncConnectionProperty,
                                                                                                     timeStep);
        flowrateNNCatAllTimeSteps.push_back(connectionFlowrate);


        //sum all tracers at current timestep
        std::vector<double> summedTracerValues(porvResults->size());
        for (size_t tracerIndex : scalarResultIndexTracers)
        {
            if (tracerIndex != cvf::UNDEFINED_SIZE_T)
            {
                const std::vector<double>* tracerResult = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(tracerIndex, timeStep));

                for (size_t i = 0; i < summedTracerValues.size(); i++)
                {
                    summedTracerValues[i] += tracerResult->at(i);
                }
            }
        }
        summedTracersAtAllTimesteps.push_back(summedTracerValues);
        
    }
    
    calculate(mainGrid,
              caseToApply,
              daysSinceSimulationStart,
              porvResults, flowrateIatAllTimeSteps, 
              flowrateJatAllTimeSteps, 
              flowrateKatAllTimeSteps, 
              connections, 
              flowrateNNCatAllTimeSteps, 
              summedTracersAtAllTimesteps);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RigNumberOfFloodedPoreVolumesCalculator::numberOfFloodedPorevolumes()
{
    return m_cumWinflowPVAllTimeSteps;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigNumberOfFloodedPoreVolumesCalculator::numberOfFloodedPorevolumesAtTimeStep(size_t timeStep)
{
    return m_cumWinflowPVAllTimeSteps[timeStep];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNumberOfFloodedPoreVolumesCalculator::calculate(RigMainGrid* mainGrid,
                                                        RimEclipseCase* caseToApply,
                                                        std::vector<double> daysSinceSimulationStart,
                                                        const std::vector<double>* porvResults,
                                                        std::vector<const std::vector<double>* > flowrateIatAllTimeSteps,
                                                        std::vector<const std::vector<double>* > flowrateJatAllTimeSteps,
                                                        std::vector<const std::vector<double>* > flowrateKatAllTimeSteps,
                                                        const std::vector<RigConnection> connections,
                                                        std::vector<const std::vector<double>* > flowrateNNCatAllTimeSteps,
                                                        std::vector<std::vector<double> > summedTracersAtAllTimesteps)
{
    size_t totalNumberOfCells = mainGrid->globalCellArray().size();

    std::vector<std::vector<double>> cellQwInAtAllTimeSteps;
    std::vector<double> cellQwInTimeStep0(totalNumberOfCells);
    cellQwInAtAllTimeSteps.push_back(cellQwInTimeStep0);

    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        std::vector<double> totoalFlowrateIntoCell(totalNumberOfCells);

        if (flowrateIatAllTimeSteps[timeStep] != nullptr
            && flowrateJatAllTimeSteps[timeStep] != nullptr
            && flowrateKatAllTimeSteps[timeStep] != nullptr)

        {
            const std::vector<double>* flowrateI = flowrateIatAllTimeSteps[timeStep];
            const std::vector<double>* flowrateJ = flowrateJatAllTimeSteps[timeStep];
            const std::vector<double>* flowrateK = flowrateKatAllTimeSteps[timeStep];

            distributeNeighbourCellFlow(mainGrid,
                                        caseToApply,
                                        summedTracersAtAllTimesteps[timeStep],
                                        flowrateI,
                                        flowrateJ,
                                        flowrateK,
                                        totoalFlowrateIntoCell);
        }

        const std::vector<double>* flowrateNNC = flowrateNNCatAllTimeSteps[timeStep];

        distributeNNCflow(connections,
                          summedTracersAtAllTimesteps[timeStep],
                          flowrateNNC,
                          totoalFlowrateIntoCell);




        std::vector<double> CellQwIn(totalNumberOfCells);

        double daysSinceSimStartNow = daysSinceSimulationStart[timeStep];
        double daysSinceSimStartLastTimeStep = daysSinceSimulationStart[timeStep - 1];
        double deltaT = daysSinceSimStartNow - daysSinceSimStartLastTimeStep;

        for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
        {
            CellQwIn[globalCellIndex] = cellQwInAtAllTimeSteps[timeStep - 1][globalCellIndex]
                + (totoalFlowrateIntoCell[globalCellIndex]) * deltaT;
        }
        cellQwInAtAllTimeSteps.push_back(CellQwIn);

    }

    //Calculate number-of-cell-PV flooded
    std::vector<double> cumWinflowPVTimeStep0(totalNumberOfCells);
    m_cumWinflowPVAllTimeSteps.clear();
    m_cumWinflowPVAllTimeSteps.push_back(cumWinflowPVTimeStep0);

    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        std::vector<double> cumWinflowPV(totalNumberOfCells);
        for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
        {
            cumWinflowPV[globalCellIndex] = cellQwInAtAllTimeSteps[timeStep][globalCellIndex]
                / porvResults->at(globalCellIndex);
        }
        m_cumWinflowPVAllTimeSteps.push_back(cumWinflowPV);
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
                                                                          RimEclipseCase* caseToApply,
                                                                          std::vector<double> summedTracerValues,
                                                                          const std::vector<double>* flrWatResultI,
                                                                          const std::vector<double>* flrWatResultJ,
                                                                          const std::vector<double>* flrWatResultK,
                                                                          std::vector<double> &totalFlowrateIntoCell)
{
    for (size_t globalCellIndex = 0; globalCellIndex < mainGrid->globalCellArray().size(); globalCellIndex++)
    {

        const RigCell& cell = mainGrid->globalCellArray()[globalCellIndex];
        RigGridBase* hostGrid = cell.hostGrid();
        size_t gridLocalCellIndex = cell.gridLocalCellIndex();

        RigActiveCellInfo* actCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
        size_t cellResultIndex = actCellInfo->cellResultIndex(globalCellIndex);

        size_t i, j, k;
        hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

        if (i < (hostGrid->cellCountI()-1))
        {
            size_t gridLocalCellIndexPosINeighbour = hostGrid->cellIndexFromIJK(i + 1, j, k);

            if (hostGrid->cell(gridLocalCellIndexPosINeighbour).subGrid() != NULL)
            {
                //subgrid exists in cell, will be handled though NNCs
                continue;
            }
            
            if (flrWatResultI->at(cellResultIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[gridLocalCellIndexPosINeighbour] += flrWatResultI->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultI->at(cellResultIndex) < 0)
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[globalCellIndex] += (-1) * flrWatResultI->at(globalCellIndex) * summedTracerValues[gridLocalCellIndexPosINeighbour];
            }
        }

        if (j < (hostGrid->cellCountJ()-1))
        {
            size_t gridLocalCellIndexPosJNeighbour = hostGrid->cellIndexFromIJK(i, j + 1, k);
            if (hostGrid->cell(gridLocalCellIndexPosJNeighbour).subGrid() != NULL)
            {
                //subgrid exists in cell, will be handled though NNCs
                continue;
            }


            if (flrWatResultJ->at(cellResultIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[gridLocalCellIndexPosJNeighbour] += flrWatResultJ->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultJ->at(cellResultIndex) < 0)
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[globalCellIndex] += flrWatResultJ->at(globalCellIndex) * summedTracerValues[gridLocalCellIndexPosJNeighbour];
            }
        }

        if (k < (hostGrid->cellCountK()-1))
        {
            size_t gridLocalCellIndexPosKNeighbour = hostGrid->cellIndexFromIJK(i, j, k + 1);
            if (hostGrid->cell(gridLocalCellIndexPosKNeighbour).subGrid() != NULL)
            {
                //subgrid exists in cell, will be handled though NNCs
                continue;
            }

            if (flrWatResultK->at(cellResultIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[gridLocalCellIndexPosKNeighbour] += flrWatResultK->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultK->at(cellResultIndex) < 0)
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[globalCellIndex] += flrWatResultK->at(globalCellIndex) * summedTracerValues[gridLocalCellIndexPosKNeighbour];
            }
        }
    }
}
