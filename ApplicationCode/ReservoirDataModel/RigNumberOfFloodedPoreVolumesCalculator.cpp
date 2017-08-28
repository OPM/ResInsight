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


#include "RigMainGrid.h"
#include <string>
#include "RimReservoirCellResultsStorage.h"
#include "RigEclipseCaseData.h"
#include "RimEclipseCase.h"
#include "RiaPorosityModel.h"
#include "RigEclipseCaseData.h"
#include "RigCaseCellResultsData.h"


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
    
    size_t scalarResultIndexFlrWatI = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATI+");
    size_t scalarResultIndexFlrWatJ = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATJ+");
    size_t scalarResultIndexFlrWatK = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATK+");

    std::vector<size_t> scalarResultIndexTracers;
    for (std::string tracerName : tracerNames)
    {
        scalarResultIndexTracers.push_back(gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "FLRWATI+"));
    }

    std::vector<double> daysSinceSimulationStart = caseToApply->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->daysSinceSimulationStart();

    //TODO
//     std::vector<const std::vector<double>* > flrWatResultIatAllTimeSteps;
//     std::vector<const std::vector<double>* > flrWatResultJatAllTimeSteps;
//     std::vector<const std::vector<double>* > flrWatResultKatAllTimeSteps;
// 
//     for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
//     {
//         const std::vector<double>* flrWatResultI = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlrWatI, timeStep));
//         flrWatResultIatAllTimeSteps.push_back(flrWatResultI);
//         const std::vector<double>* flrWatResultJ = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlrWatJ, timeStep));
//         flrWatResultJatAllTimeSteps.push_back(flrWatResultJ);
//         const std::vector<double>* flrWatResultK = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlrWatK, timeStep));
//         flrWatResultKatAllTimeSteps.push_back(flrWatResultK);
//     }
       

    size_t totalNumberOfCells = porvResults->size();

    std::vector<std::vector<double>> cellQwInAtAllTimeSteps;
    std::vector<double> cellQwInTimeStep0(totalNumberOfCells);
    cellQwInAtAllTimeSteps.push_back(cellQwInTimeStep0);

    
    for (size_t timeStep = 1; timeStep < daysSinceSimulationStart.size(); timeStep++)
    {
        double daysSinceSimStartNow = daysSinceSimulationStart[timeStep];
        double daysSinceSimStartLastTimeStep = daysSinceSimulationStart[timeStep -1];
        double deltaT = daysSinceSimStartNow - daysSinceSimStartLastTimeStep;

        const std::vector<double>* flrWatResultI = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlrWatI, timeStep));
        const std::vector<double>* flrWatResultJ = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlrWatJ, timeStep));
        const std::vector<double>* flrWatResultK = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexFlrWatK, timeStep));

        std::vector<double> FwI(totalNumberOfCells);
        std::vector<double> FwJ(totalNumberOfCells);
        std::vector<double> FwK(totalNumberOfCells);

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
                                    flrWatResultI, 
                                    FwI, 
                                    flrWatResultJ, 
                                    FwJ, 
                                    flrWatResultK, 
                                    FwK);

        //TODO: Add NNC contributions

        std::vector<double> CellQwIn(totalNumberOfCells);

        for (size_t globalCellIndex = 0; globalCellIndex < totalNumberOfCells; globalCellIndex++)
        {
            CellQwIn[globalCellIndex] = cellQwInAtAllTimeSteps[timeStep-1][globalCellIndex] 
                + (FwI[globalCellIndex] + FwJ[globalCellIndex] + FwK[globalCellIndex]) * deltaT;
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
void RigNumberOfFloodedPoreVolumesCalculator::distributeNeighbourCellFlow(RigMainGrid* mainGrid,
                                                                          size_t totalNumberOfCells,
                                                                          std::vector<double> summedTracerValues,
                                                                          const std::vector<double>* flrWatResultI,
                                                                          std::vector<double> &FwI,
                                                                          const std::vector<double>* flrWatResultJ,
                                                                          std::vector<double> &FwJ,
                                                                          const std::vector<double>* flrWatResultK,
                                                                          std::vector<double> &FwK)
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
                FwI[globalCellIndexPosINeightbour] += flrWatResultI->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultI->at(globalCellIndex) < 0 && i < mainGrid->cellCountI())
            {
                //Flow into cell globelCellIndex, from cell i+1
                FwI[globalCellIndex] += flrWatResultI->at(globalCellIndex) * summedTracerValues[globalCellIndexPosINeightbour];
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
                FwJ[globalCellIndexPosJNeightbour] += flrWatResultJ->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultJ->at(globalCellIndex) < 0 && i < mainGrid->cellCountI())
            {
                //Flow into cell globelCellIndex, from cell i+1
                FwJ[globalCellIndex] += flrWatResultJ->at(globalCellIndex) * summedTracerValues[globalCellIndexPosJNeightbour];
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
                FwK[globalCellIndexPosKNeightbour] += flrWatResultK->at(globalCellIndex) * summedTracerValues[globalCellIndex];
            }
            else if (flrWatResultK->at(globalCellIndex) < 0 && i < mainGrid->cellCountI())
            {
                //Flow into cell globelCellIndex, from cell i+1
                FwK[globalCellIndex] += flrWatResultK->at(globalCellIndex) * summedTracerValues[globalCellIndexPosKNeightbour];
            }
        }
    }
}
