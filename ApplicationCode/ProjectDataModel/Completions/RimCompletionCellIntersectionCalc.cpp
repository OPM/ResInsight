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

#include "RimCompletionCellIntersectionCalc.h"

#include "RiaDefines.h"

#include "RigCompletionData.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaDefines::CompletionType> fromCompletionData(const std::vector<RigCompletionData>& data)
{
    std::vector<RiaDefines::CompletionType> appCompletionTypes;

    for (const auto& d : data)
    {
        switch (d.completionType())
        {
            case RigCompletionData::FRACTURE:
                appCompletionTypes.push_back(RiaDefines::FRACTURE);
                break;
            case RigCompletionData::PERFORATION:
                appCompletionTypes.push_back(RiaDefines::PERFORATION_INTERVAL);
                break;
            case RigCompletionData::FISHBONES:
                appCompletionTypes.push_back(RiaDefines::FISHBONES);
                break;
            default:
                break;
        }
    }

    return appCompletionTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateCompletionTypeResult(RimEclipseCase*      eclipseCase,
                                                                      std::vector<double>& completionTypeCellResult,
                                                                      size_t               timeStep)
{
    CVF_ASSERT(eclipseCase && eclipseCase->eclipseCaseData());

    RimProject* project = nullptr;
    eclipseCase->firstAncestorOrThisOfTypeAsserted(project);

    if (project->activeOilField()->wellPathCollection->isActive)
    {
        const RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

        for (const RimWellPath* wellPath : project->activeOilField()->wellPathCollection->wellPaths)
        {
            if (wellPath->showWellPath() && wellPath->wellPathGeometry())
            {
                auto intersectedCells = RigWellPathIntersectionTools::findIntersectedGlobalCellIndices(
                    eclipseCaseData, wellPath->wellPathGeometry()->m_wellPathPoints);

                for (auto& intersection : intersectedCells)
                {
                    completionTypeCellResult[intersection] = RiaDefines::WELL_PATH;
                }

                auto conn = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
                if (conn)
                {
                    for (const auto& connForWell : conn->multipleCompletionsPerEclipseCell(wellPath, timeStep))
                    {
                        RiaDefines::CompletionType appCompletionType = RiaDefines::WELL_PATH;

                        auto appCompletionTypes = fromCompletionData(connForWell.second);

                        if (std::find(appCompletionTypes.begin(), appCompletionTypes.end(), RiaDefines::FISHBONES) !=
                            appCompletionTypes.end())
                        {
                            appCompletionType = RiaDefines::FISHBONES;
                        }
                        else if (std::find(appCompletionTypes.begin(), appCompletionTypes.end(), RiaDefines::FRACTURE) !=
                                 appCompletionTypes.end())
                        {
                            appCompletionType = RiaDefines::FRACTURE;
                        }
                        else if (std::find(appCompletionTypes.begin(),
                                           appCompletionTypes.end(),
                                           RiaDefines::PERFORATION_INTERVAL) != appCompletionTypes.end())
                        {
                            appCompletionType = RiaDefines::PERFORATION_INTERVAL;
                        }

                        completionTypeCellResult[connForWell.first.globalCellIndex()] = appCompletionType;
                    }
                }
            }
        }
    }
}
