/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

class RimCellRangeFilter;
class RigMainGrid;
class RigFemPart;

class RigCaseToCaseRangeFilterMapper
{
public:
    static void convertRangeFilterEclToFem(RimCellRangeFilter* srcFilter, const RigMainGrid* srcEclGrid,
                                           RimCellRangeFilter* dstFilter, const RigFemPart*  dstFemPart);
    static void convertRangeFilterFemToEcl(RimCellRangeFilter* srcFilter, const RigFemPart* srcFemPart, 
                                           RimCellRangeFilter* dstFilter, const RigMainGrid* dstEclGrid);

private:

    static void convertRangeFilter(RimCellRangeFilter* srcFilter, RimCellRangeFilter* dstFilter, 
                                   const RigMainGrid* eclGrid, const RigFemPart*  femPart, 
                                   bool femIsDestination);
    

    static bool findBestFemCellFromEclCell(const RigMainGrid* masterEclGrid, size_t ei, size_t ej, size_t ek,
                                           const RigFemPart* dependentFemPart, size_t* fi, size_t * fj, size_t* fk);

    static bool findBestEclCellFromFemCell(const RigFemPart* dependentFemPart, size_t fi, size_t fj, size_t fk,
                                           const RigMainGrid* masterEclGrid, size_t* ei, size_t* ej, size_t* ek);
};


