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

#include <map>

class RigWellPath;
class RimFracture;

class RigWellPathStimplanIntersector
{
public:
    struct WellCellIntersection 
    { 
        WellCellIntersection():hlength(0.0), vlength(0.0), endpointCount(0) {}  
        double hlength; 
        double vlength; 
        int endpointCount;
    };

    RigWellPathStimplanIntersector(const RigWellPath* wellpathGeom, const RimFracture * rimFracture);

    const std::map<size_t, WellCellIntersection >& intersections() { return m_stimPlanCellIdxToIntersectionInfoMap; }

private:
     std::map<size_t, WellCellIntersection > m_stimPlanCellIdxToIntersectionInfoMap;
};



