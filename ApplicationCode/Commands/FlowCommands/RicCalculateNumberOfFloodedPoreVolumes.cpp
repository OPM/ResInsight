/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicCalculateNumberOfFloodedPoreVolumes.h"


#include "RiaApplication.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigNumberOfFloodedPoreVolumesCalculator.h"

#include "RimEclipseCase.h"
#include "RimView.h"

#include <QAction>
#include <QString>
#include "RimReservoirCellResultsStorage.h"
#include "RigActiveCellInfo.h"

CAF_CMD_SOURCE_INIT(RicCalculateNumberOfFloodedPoreVolumes, "RicCalculateNumberOfFloodedPoreVolumes");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCalculateNumberOfFloodedPoreVolumes::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCalculateNumberOfFloodedPoreVolumes::onActionTriggered(bool isChecked)
{
    RimView* view = RiaApplication::instance()->activeReservoirView();
    RimEclipseCase* caseToApply;
    view->firstAncestorOrThisOfType(caseToApply);
    RigMainGrid* mainGrid = caseToApply->eclipseCaseData()->mainGrid();
    
    std::vector<QString> tracerNames;
    tracerNames.push_back("SOIL");

    RigNumberOfFloodedPoreVolumesCalculator calc(caseToApply, tracerNames);

    std::vector<std::vector<double>>  numberOfFloodedPorevolumes = calc.numberOfFloodedPorevolumes();
    std::vector<std::vector<double>>  cumInflow = calc.cumInflow();

    //Test
//     size_t cellIndex = mainGrid->reservoirCellIndex(mainGrid->cellIndexFromIJK(19, 34, 9));
//     RigActiveCellInfo* actCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
//     size_t cellResultIndex = actCellInfo->cellResultIndex(cellIndex);
// 
//     std::vector<double> numberOfFloodedPorevolumesForSingleCell;
//     std::vector<double> cumInflowForSingleCell;
// 
//     for (size_t timeStep = 0; timeStep < numberOfFloodedPorevolumes.size(); timeStep++)
//     {
//         numberOfFloodedPorevolumesForSingleCell.push_back(numberOfFloodedPorevolumes[timeStep][cellResultIndex]);
//         cumInflowForSingleCell.push_back(cumInflow[timeStep][cellResultIndex]);
//     }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCalculateNumberOfFloodedPoreVolumes::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("TEST Calculate Number of Flooded Pore Volumes");
}
