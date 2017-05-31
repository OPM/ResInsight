/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigActiveCellsResultAccessor.h"

#include "RigGridBase.h"
#include "RigActiveCellInfo.h"

#include <cmath>


RigActiveCellsResultAccessor::RigActiveCellsResultAccessor(const RigGridBase* grid, const std::vector<double>* reservoirResultValues, const RigActiveCellInfo* activeCellInfo)
    : m_grid(grid),
    m_reservoirResultValues(reservoirResultValues),
    m_activeCellInfo(activeCellInfo)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigActiveCellsResultAccessor::cellScalar(size_t gridLocalCellIndex) const
{
    if (m_reservoirResultValues == NULL || m_reservoirResultValues->size() == 0 ) return HUGE_VAL;

    size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
    size_t resultValueIndex = m_activeCellInfo->cellResultIndex(reservoirCellIndex);
    if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;

    if (resultValueIndex < m_reservoirResultValues->size())
        return m_reservoirResultValues->at(resultValueIndex);

    CVF_TIGHT_ASSERT(resultValueIndex < m_activeCellInfo->reservoirActiveCellCount()); // Because some static results might lack LGR data
    
    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigActiveCellsResultAccessor::cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    return cellScalar(gridLocalCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigActiveCellsResultAccessor::cellScalarGlobIdx(size_t reservoirCellIndex) const
{
    if (m_reservoirResultValues == NULL || m_reservoirResultValues->size() == 0) return HUGE_VAL;

    size_t resultValueIndex = m_activeCellInfo->cellResultIndex(reservoirCellIndex);
    if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;

    if(resultValueIndex < m_reservoirResultValues->size())
        return m_reservoirResultValues->at(resultValueIndex);

    CVF_TIGHT_ASSERT(resultValueIndex < m_activeCellInfo->reservoirActiveCellCount()); // Because some static results might lack LGR data

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigActiveCellsResultAccessor::cellFaceScalarGlobIdx(size_t globCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    return cellScalarGlobIdx(globCellIndex);
}

