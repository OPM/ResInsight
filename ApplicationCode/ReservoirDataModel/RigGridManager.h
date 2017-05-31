/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfCollection.h"

#include <vector>

class RigMainGrid;
class RigEclipseCaseData;

class RigGridManager : public cvf::Object
{
public:
    void addCase(RigEclipseCaseData* eclipseCase);
    
    void removeCase(RigEclipseCaseData* eclipseCase);
    
    RigMainGrid* findEqualGrid(RigMainGrid* candidateGrid);

    void clear();

    static bool isEqual(RigMainGrid* gridA, RigMainGrid* gridB);
    static bool isGridDimensionsEqual(const std::vector< std::vector<int> >& mainCaseGridDimensions, const std::vector< std::vector<int> >& caseGridDimensions);
private:
    class CaseToGridMap : public cvf::Object
    {
    public:
        CaseToGridMap(RigEclipseCaseData* eclipseCase, RigMainGrid* mainGrid);

        RigEclipseCaseData*    m_eclipseCase;
        RigMainGrid*       m_mainGrid;
    };


private:
    cvf::Collection<CaseToGridMap> m_caseToGrid;
};
