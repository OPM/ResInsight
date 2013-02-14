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
class RigEclipseCase;

class RigGridCollection
{
public:
    void addGrid(RigEclipseCase* eclipseCase, RigMainGrid* mainGrid);
    void removeCase(RigEclipseCase* eclipseCase);
    
    RigMainGrid* findEqualGrid(RigMainGrid* candidateGrid);

private:

    static bool isEqual(RigMainGrid* gridA, RigMainGrid* gridB);

    class CaseToGridMap
    {
    public:
        CaseToGridMap(RigEclipseCase* eclipseCase, RigMainGrid* mainGrid) :
            m_eclipseCase(eclipseCase),
            m_mainGrid(mainGrid)
        {
        }

        cvf::ref<RigEclipseCase>    m_eclipseCase;
        cvf::ref<RigMainGrid>       m_mainGrid;
    };


private:
    std::vector<CaseToGridMap> m_caseToGrid;
};