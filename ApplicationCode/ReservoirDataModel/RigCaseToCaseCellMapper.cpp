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

#include "RigCaseToCaseCellMapper.h"
#include "RigFemPart.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigFemPart* dependentFemPart)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(NULL),
      m_masterFemPart(dependentFemPart),
      m_dependentFemPart(NULL)
{
    m_masterCellOrIntervalIndex.resize(dependentFemPart->elementCount(), cvf::UNDEFINED_INT);
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigMainGrid* dependentEclGrid)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(NULL),
      m_dependentFemPart(NULL)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigMainGrid* dependentEclGrid)
    : m_masterGrid(NULL),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(NULL)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigFemPart* dependentFemPart)
    : m_masterGrid(NULL),
      m_dependentGrid(NULL),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(dependentFemPart)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const int * RigCaseToCaseCellMapper::masterCaseCellIndices(int dependentCaseReservoirCellIndex, int* masterCaseCellIndexCount) const
{
    int seriesIndex = m_masterCellOrIntervalIndex[dependentCaseReservoirCellIndex];

    if (seriesIndex == cvf::UNDEFINED_INT)
    {
        (*masterCaseCellIndexCount) = 0;
        return NULL;
    }

    if (seriesIndex < 0)
    {
        (*masterCaseCellIndexCount) = static_cast<int>(m_masterCellIndexSeries[-seriesIndex].size());
        return &(m_masterCellIndexSeries[-seriesIndex][0]);    
    }
    else
    {
        (*masterCaseCellIndexCount) = 1;
        return &(m_masterCellOrIntervalIndex[dependentCaseReservoirCellIndex]);
    }
}
