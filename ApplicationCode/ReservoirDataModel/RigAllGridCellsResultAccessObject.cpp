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

#include "RigAllGridCellsResultAccessObject.h"

#include "RigGridBase.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigAllGridCellsResultAccessObject::RigAllGridCellsResultAccessObject(const RigGridBase* grid, std::vector<double>* reservoirResultValues, const QString& resultName)
    : m_grid(grid),
    m_reservoirResultValues(reservoirResultValues),
    m_resultName(resultName)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigAllGridCellsResultAccessObject::cellScalar(size_t localCellIndex) const
{
    if (m_reservoirResultValues->size() == 0 ) return HUGE_VAL;

    size_t globalGridCellIndex = m_grid->globalGridCellIndex(localCellIndex);
    CVF_TIGHT_ASSERT(globalGridCellIndex < m_reservoirResultValues->size());

    return m_reservoirResultValues->at(globalGridCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigAllGridCellsResultAccessObject::cellFaceScalar(size_t localCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    return cellScalar(localCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigAllGridCellsResultAccessObject::resultName() const
{
    return m_resultName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAllGridCellsResultAccessObject::setCellScalar(size_t localCellIndex, double scalarValue)
{
    size_t globalGridCellIndex = m_grid->globalGridCellIndex(localCellIndex);
    CVF_TIGHT_ASSERT(globalGridCellIndex < m_reservoirResultValues->size());

    (*m_reservoirResultValues)[globalGridCellIndex] = scalarValue;
}
