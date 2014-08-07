/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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


RigActiveCellsResultAccessor::RigActiveCellsResultAccessor(const RigGridBase* grid, std::vector<double>* reservoirResultValues, const RigActiveCellInfo* activeCellInfo, const QString& resultName)
    : m_grid(grid),
    m_reservoirResultValues(reservoirResultValues),
    m_activeCellInfo(activeCellInfo),
    m_resultName(resultName)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigActiveCellsResultAccessor::cellScalar(size_t localCellIndex) const
{
    if (m_reservoirResultValues == NULL || m_reservoirResultValues->size() == 0 ) return HUGE_VAL;

    size_t globalGridCellIndex = m_grid->globalGridCellIndex(localCellIndex);
    size_t resultValueIndex = m_activeCellInfo->cellResultIndex(globalGridCellIndex);
    if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;

    CVF_TIGHT_ASSERT(resultValueIndex < m_reservoirResultValues->size());

    return m_reservoirResultValues->at(resultValueIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigActiveCellsResultAccessor::cellFaceScalar(size_t localCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    return cellScalar(localCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigActiveCellsResultAccessor::resultName() const
{
    return m_resultName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellsResultAccessor::setCellScalar(size_t localCellIndex, double scalarValue)
{
    size_t globalGridCellIndex = m_grid->globalGridCellIndex(localCellIndex);
    size_t resultValueIndex = m_activeCellInfo->cellResultIndex(globalGridCellIndex);

    CVF_TIGHT_ASSERT(m_reservoirResultValues != NULL && resultValueIndex < m_reservoirResultValues->size());

    (*m_reservoirResultValues)[resultValueIndex] = scalarValue;
}
