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

#pragma once

#include <cstddef>
#include <vector>
#include <string>

class RimEclipseCase;
class RigMainGrid;
class RigConnection;

//==================================================================================================
/// 
//==================================================================================================

class RigNumberOfFloodedPoreVolumesCalculator
{

public:
    explicit RigNumberOfFloodedPoreVolumesCalculator(RigMainGrid* mainGrid, 
                                                     RimEclipseCase* caseToApply,
                                                     std::vector<std::string> tracerNames
                                                     );

    std::vector<std::vector<double>>    numberOfFloodedPorevolumes();
    std::vector<double>                 numberOfFloodedPorevolumesAtTimeStep(size_t timeStep);


private:
    void calculate(RigMainGrid* mainGrid, 
                   RimEclipseCase* caseToApply,
                   std::vector<double> daysSinceSimulationStart, 
                   const std::vector<double>* porvResults, 
                   std::vector<const std::vector<double>* > flowrateIatAllTimeSteps, 
                   std::vector<const std::vector<double>* > flowrateJatAllTimeSteps, 
                   std::vector<const std::vector<double>* > flowrateKatAllTimeSteps, 
                   const std::vector<RigConnection> connections, 
                   std::vector<const std::vector<double>* > flowrateNNCatAllTimeSteps, 
                   std::vector<std::vector<double> > summedTracersAtAllTimesteps);


    void distributeNNCflow(std::vector<RigConnection> connections,
                           std::vector<double> summedTracerValues, 
                           const std::vector<double>* flowrateNNC, 
                           std::vector<double> &flowrateIntoCell);

    void distributeNeighbourCellFlow(RigMainGrid* mainGrid,
                                     RimEclipseCase* caseToApply,
                                     std::vector<double> summedTracerValues, 
                                     const std::vector<double>* flrWatResultI, 
                                     const std::vector<double>* flrWatResultJ, 
                                     const std::vector<double>* flrWatResultK, 
                                     std::vector<double> &totalFlowrateIntoCell);

private:
    std::vector<std::vector<double>> m_cumWinflowPVAllTimeSteps;

};

