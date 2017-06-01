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

class RimProject;
class RimWellPath;
class RimFishbonesMultipleSubs;
class RimPerforationInterval;

class RigMainGrid;
class QDateTime;

//==================================================================================================
///  
///  
//==================================================================================================
class RimCompletionCellIntersectionCalc
{
public:
    static void                     calculateIntersections(const RimProject* project, const RigMainGrid* grid, std::vector<double>& values, const QDateTime& fromDate);
    
private:
    static void                     calculateWellPathIntersections(const RimWellPath* wellPath, const RigMainGrid* grid, std::vector<double>& values, const QDateTime& fromDate);
    static void                     calculateFishbonesIntersections(const RimFishbonesMultipleSubs* fishbonesSubs, const RigMainGrid* grid, std::vector<double>& values);
    static void                     calculatePerforationIntersections(const RimWellPath* wellPath, const RimPerforationInterval* perforationInterval, const RigMainGrid* grid, std::vector<double>& values);
};
