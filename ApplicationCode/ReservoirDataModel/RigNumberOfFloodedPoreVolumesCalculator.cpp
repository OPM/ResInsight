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

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilderMock.h"

#include "RimEclipseCase.h"
#include "RimReservoirCellResultsStorage.h"

#include <vector>
#include <QString>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigNumberOfFloodedPoreVolumesCalculator::RigNumberOfFloodedPoreVolumesCalculator(RigMainGrid* mainGrid, 
                                                                                 RimEclipseCase* caseToApply, 
                                                                                 const std::vector<QString> tracerNames)
{
    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(RiaDefines::MATRIX_MODEL);

    RigActiveCellInfo* actCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    size_t numberOfActiveCells = actCellInfo->reservoirCellResultCount();

    size_t scalarResultIndexPorv = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORV");
    const std::vector<double>* porvResults = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexPorv, 0));  

    std::vector<size_t> scalarResultIndexTracers;
    for (QString tracerName : tracerNames)
    {
        scalarResultIndexTracers.push_back(gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, tracerName));
    }
    std::vector<std::vector<double> > summedTracersAtAllTimesteps;

    //TODO: Option for Oil and Gas instead of water
    size_t scalarResultIndexFlowrateI = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "FLRWATI+");
    size_t scalarResultIndexFlowrateJ = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "FLRWATJ+");
    size_t scalarResultIndexFlowrateK = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "FLRWATK+");

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
            flowrateJ = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlowrateJ,
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
        std::vector<double> summedTracerValues(numberOfActiveCells);
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
              porvResults, 
              flowrateIatAllTimeSteps, 
              flowrateJatAllTimeSteps, 
              flowrateKatAllTimeSteps, 
              connections, 
              flowrateNNCatAllTimeSteps, 
              summedTracersAtAllTimesteps);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>& RigNumberOfFloodedPoreVolumesCalculator::numberOfFloodedPorevolumes() const
{
    return m_cumWinflowPVAllTimeSteps;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>&  RigNumberOfFloodedPoreVolumesCalculator::numberOfFloodedPorevolumesAtTimeStep(size_t timeStep) const
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
    //size_t totalNumberOfCells = mainGrid->globalCellArray().size();
    RigActiveCellInfo* actCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    size_t numberOfActiveCells = actCellInfo->reservoirCellResultCount();


    std::vector<std::vector<double>> cellQwInAtAllTimeSteps;
    std::vector<double> cellQwInTimeStep0(numberOfActiveCells);
    cellQwInAtAllTimeSteps.push_back(cellQwInTimeStep0);

    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        std::vector<double> totoalFlowrateIntoCell(numberOfActiveCells); //brukt result celle index / active  antall i stedet

        if (flowrateIatAllTimeSteps[timeStep-1] != nullptr
            && flowrateJatAllTimeSteps[timeStep-1] != nullptr
            && flowrateKatAllTimeSteps[timeStep-1] != nullptr)

        {
            const std::vector<double>* flowrateI = flowrateIatAllTimeSteps[timeStep-1];
            const std::vector<double>* flowrateJ = flowrateJatAllTimeSteps[timeStep-1];
            const std::vector<double>* flowrateK = flowrateKatAllTimeSteps[timeStep-1];

            if (flowrateI->size() > 0 && flowrateJ->size() > 0 && flowrateK->size() > 0)
            {
                distributeNeighbourCellFlow(mainGrid,
                                            caseToApply,
                                            summedTracersAtAllTimesteps[timeStep-1],
                                            flowrateI,
                                            flowrateJ,
                                            flowrateK,
                                            totoalFlowrateIntoCell);
            }

        }

        const std::vector<double>* flowrateNNC = flowrateNNCatAllTimeSteps[timeStep-1];

        if (flowrateNNC->size() > 0)
        {
            distributeNNCflow(connections,
                              caseToApply,
                              summedTracersAtAllTimesteps[timeStep-1],
                              flowrateNNC,
                              totoalFlowrateIntoCell);
        }

        std::vector<double> CellQwIn(numberOfActiveCells);

        double daysSinceSimStartNow = daysSinceSimulationStart[timeStep];
        double daysSinceSimStartLastTimeStep = daysSinceSimulationStart[timeStep - 1];
        double deltaT = daysSinceSimStartNow - daysSinceSimStartLastTimeStep;

        for (size_t cellResultIndex = 0; cellResultIndex < numberOfActiveCells; cellResultIndex++)
        {
            CellQwIn[cellResultIndex] = cellQwInAtAllTimeSteps[timeStep - 1][cellResultIndex]
                + (totoalFlowrateIntoCell[cellResultIndex]) * deltaT;
        }
        cellQwInAtAllTimeSteps.push_back(CellQwIn);

    }

    //Using porv only for active cells
    std::vector<double> porvResultsActiveCellsOnly;
    for (size_t globalCellIndex = 0; globalCellIndex < mainGrid->globalCellArray().size(); globalCellIndex++)
    {
        if (actCellInfo->isActive(globalCellIndex))
        {
            porvResultsActiveCellsOnly.push_back(porvResults->at(globalCellIndex));
        }
    }
    CVF_ASSERT(porvResultsActiveCellsOnly.size() == numberOfActiveCells);


    //Calculate number-of-cell-PV flooded
    std::vector<double> cumWinflowPVTimeStep0(numberOfActiveCells);
    m_cumWinflowPVAllTimeSteps.clear();
    m_cumWinflowPVAllTimeSteps.push_back(cumWinflowPVTimeStep0);

    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        std::vector<double> cumWinflowPV(numberOfActiveCells);
        for (size_t cellResultIndex = 0; cellResultIndex < numberOfActiveCells; cellResultIndex++)
        {
            cumWinflowPV[cellResultIndex] = cellQwInAtAllTimeSteps[timeStep][cellResultIndex]
                / porvResultsActiveCellsOnly[cellResultIndex];
        }
        m_cumWinflowPVAllTimeSteps.push_back(cumWinflowPV);
    }

    //TODO: Only for testing
    m_cellQwInAtAllTimeSteps = cellQwInAtAllTimeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigNumberOfFloodedPoreVolumesCalculator::distributeNNCflow(std::vector<RigConnection> connections,
                                                                RimEclipseCase* caseToApply,
                                                                std::vector<double> summedTracerValues,
                                                                const std::vector<double>* flowrateNNC, 
                                                                std::vector<double> &flowrateIntoCell)
{
    RigActiveCellInfo* actCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);


    for (size_t connectionIndex = 0; connectionIndex < connections.size(); connectionIndex++)
    {
        RigConnection connection = connections[connectionIndex];
        double connectionValue = flowrateNNC->at(connectionIndex);

        size_t cell1Index = connection.m_c1GlobIdx;
        size_t cell1ResultIndex = actCellInfo->cellResultIndex(cell1Index);

        size_t cell2Index = connection.m_c2GlobIdx;
        size_t cell2ResultIndex = actCellInfo->cellResultIndex(cell2Index);

        if (connectionValue > 0)
        {
            //Flow out of cell with cell1index, into cell cell2index
            flowrateIntoCell[cell2ResultIndex] += connectionValue * summedTracerValues[cell1ResultIndex];
        }
        else if (connectionValue < 0)
        {
            //flow out of cell with cell2index, into cell cell1index
            flowrateIntoCell[cell1ResultIndex] += connectionValue * summedTracerValues[cell2ResultIndex];
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
    RigActiveCellInfo* actCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

    for (size_t globalCellIndex = 0; globalCellIndex < mainGrid->globalCellArray().size(); globalCellIndex++)
    {
        if (!actCellInfo->isActive(globalCellIndex)) continue;

        const RigCell& cell = mainGrid->globalCellArray()[globalCellIndex];
        RigGridBase* hostGrid = cell.hostGrid();
        size_t gridLocalCellIndex = cell.gridLocalCellIndex();
        
        size_t cellResultIndex = actCellInfo->cellResultIndex(globalCellIndex);

        size_t i, j, k;
        hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

        if (i < (hostGrid->cellCountI()-1))
        {
            size_t gridLocalCellIndexPosINeighbour = hostGrid->cellIndexFromIJK(i + 1, j, k);
            size_t reservoirCellIndexPosINeighbour = hostGrid->reservoirCellIndex(gridLocalCellIndexPosINeighbour);
            size_t cellResultIndexPosINeighbour = actCellInfo->cellResultIndex(reservoirCellIndexPosINeighbour);
            
            if (!actCellInfo->isActive(reservoirCellIndexPosINeighbour)) continue;
            
            if (hostGrid->cell(gridLocalCellIndexPosINeighbour).subGrid() != NULL)
            {
                //subgrid exists in cell, will be handled though NNCs
                continue;
            }
            
            if (flrWatResultI->at(cellResultIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[cellResultIndexPosINeighbour] += flrWatResultI->at(cellResultIndex) * summedTracerValues[cellResultIndex];
            }
            else if (flrWatResultI->at(cellResultIndex) < 0)
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[cellResultIndex] += (-1) * flrWatResultI->at(cellResultIndex) * summedTracerValues[cellResultIndexPosINeighbour];
            }
        }

        if (j < (hostGrid->cellCountJ()-1))
        {
            size_t gridLocalCellIndexPosJNeighbour = hostGrid->cellIndexFromIJK(i, j + 1, k);
            size_t reservoirCellIndexPosJNeighbour = hostGrid->reservoirCellIndex(gridLocalCellIndexPosJNeighbour);
            size_t cellResultIndexPosJNeighbour = actCellInfo->cellResultIndex(reservoirCellIndexPosJNeighbour);

            if (!actCellInfo->isActive(reservoirCellIndexPosJNeighbour)) continue;
            
            if (hostGrid->cell(gridLocalCellIndexPosJNeighbour).subGrid() != NULL)
            {
                //subgrid exists in cell, will be handled though NNCs
                continue;
            }


            if (flrWatResultJ->at(cellResultIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[cellResultIndexPosJNeighbour] += flrWatResultJ->at(cellResultIndex) * summedTracerValues[cellResultIndex];
            }
            else if (flrWatResultJ->at(cellResultIndex) < 0)
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[cellResultIndex] += flrWatResultJ->at(cellResultIndex) * summedTracerValues[cellResultIndexPosJNeighbour];
            }
        }

        if (k < (hostGrid->cellCountK()-1))
        {
            size_t gridLocalCellIndexPosKNeighbour = hostGrid->cellIndexFromIJK(i, j, k + 1);
            size_t reservoirCellIndexPosKNeighbour = hostGrid->reservoirCellIndex(gridLocalCellIndexPosKNeighbour);
            size_t cellResultIndexPosKNeighbour = actCellInfo->cellResultIndex(reservoirCellIndexPosKNeighbour);

            if (!actCellInfo->isActive(reservoirCellIndexPosKNeighbour)) continue;

            if (hostGrid->cell(gridLocalCellIndexPosKNeighbour).subGrid() != NULL)
            {
                //subgrid exists in cell, will be handled though NNCs
                continue;
            }

            if (flrWatResultK->at(cellResultIndex) > 0)
            {
                //Flow out of cell globalCellIndex, into cell i+1
                totalFlowrateIntoCell[cellResultIndexPosKNeighbour] += flrWatResultK->at(cellResultIndex) * summedTracerValues[cellResultIndex];
            }
            else if (flrWatResultK->at(cellResultIndex) < 0)
            {
                //Flow into cell globelCellIndex, from cell i+1
                totalFlowrateIntoCell[cellResultIndex] += flrWatResultK->at(cellResultIndex) * summedTracerValues[cellResultIndexPosKNeighbour];
            }
        }
    }
}
