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

#include "RigGridManager.h"
#include "RigCaseData.h"
#include "RigMainGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridManager::addCase(RigCaseData* eclipseCase)
{
    cvf::ref<CaseToGridMap> caseAndGrid = new CaseToGridMap(eclipseCase, eclipseCase->mainGrid());
    m_caseToGrid.push_back(caseAndGrid.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridManager::removeCase(RigCaseData* eclipseCase)
{
    size_t indexToErase = cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < m_caseToGrid.size(); i++)
    {
        if (m_caseToGrid[i]->m_eclipseCase == eclipseCase)
        {
            m_caseToGrid.eraseAt(i);
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RigGridManager::findEqualGrid(RigMainGrid* candidateGrid)
{
    for (size_t i = 0; i < m_caseToGrid.size(); i++)
    {
        RigMainGrid* mainGrid = m_caseToGrid.at(i)->m_mainGrid;
        if (RigGridManager::isEqual(mainGrid, candidateGrid))
        {
            return mainGrid;
        }
    }
    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridManager::isEqual(RigMainGrid* gridA, RigMainGrid* gridB)
{
    if (gridA == NULL || gridB == NULL) return false;

    if (gridA == gridB) return true;

    if (gridA->gridCount() != gridB->gridCount()) return false;

    if (gridA->nodes().size() != gridB->nodes().size()) return false;

    for (size_t i = 0; i < gridA->nodes().size(); i++)
    {
        if (!gridA->nodes()[i].equals(gridB->nodes()[i]))
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridManager::clear()
{
    m_caseToGrid.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGridManager::CaseToGridMap::CaseToGridMap(RigCaseData* eclipseCase, RigMainGrid* mainGrid) :
m_eclipseCase(eclipseCase),
    m_mainGrid(mainGrid)
{
}
