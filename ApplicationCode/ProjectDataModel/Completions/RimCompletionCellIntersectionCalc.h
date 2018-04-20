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

#pragma once

#include <vector>

class RigMainGrid;
class RimEclipseCase;
class RimFishbonesMultipleSubs;
class RimWellPathFracture;
class RimPerforationInterval;
class RimProject;
class RimWellPath;
class RigEclipseCaseData;

class QDateTime;

//==================================================================================================
///
///
//==================================================================================================
class RimCompletionCellIntersectionCalc
{
public:
    static void calculateCompletionTypeResult(const RimProject*     project,
                                              const RimEclipseCase* eclipseCase,
                                              std::vector<double>&  completionTypeCellResult,
                                              const QDateTime&      fromDate);

private:
    static void calculateWellPathIntersections(const RimWellPath*        wellPath,
                                               const RigEclipseCaseData* eclipseCaseData,
                                               std::vector<double>&      values,
                                               const QDateTime&          fromDate);

    static void calculateFishbonesIntersections(const RimFishbonesMultipleSubs* fishbonesSubs,
                                                const RigEclipseCaseData*       eclipseCaseData,
                                                std::vector<double>&            values);

    static void calculatePerforationIntersections(const RimWellPath*            wellPath,
                                                  const RimPerforationInterval* perforationInterval,
                                                  const RigEclipseCaseData*     eclipseCaseData,
                                                  std::vector<double>&          values);

    static void calculateFractureIntersections(const RigMainGrid*   mainGrid, 
                                               const RimWellPathFracture*   fracture, 
                                               std::vector<double>& values);
};
