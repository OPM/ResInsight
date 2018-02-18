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

#include "RigCombTransResultAccessor.h"

#include "RigGridBase.h"

#include <cmath>
#include "RigCell.h"
#include "RigMainGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCombTransResultAccessor::RigCombTransResultAccessor(const RigGridBase* grid)
    : m_grid(grid)
{
    
}

//--------------------------------------------------------------------------------------------------
/// Only sensible to provide the positive values, as the negative ones will never be used.
/// The negative faces gets their value from the neighbor cell in that direction
//--------------------------------------------------------------------------------------------------
void RigCombTransResultAccessor::setTransResultAccessors(RigResultAccessor* xTransAccessor,
                                                         RigResultAccessor* yTransAccessor,
                                                         RigResultAccessor* zTransAccessor)

{
    m_xTransAccessor = xTransAccessor;
    m_yTransAccessor = yTransAccessor;
    m_zTransAccessor = zTransAccessor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessor::cellScalar(size_t gridLocalCellIndex) const
{
    CVF_TIGHT_ASSERT(false);

    return HUGE_VAL;
}
//--------------------------------------------------------------------------------------------------
/// Get tran value from neighbor cell. Return 0.0 on active/inactive cell borders and end of grid
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessor::neighborCellTran(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId, const RigResultAccessor* transAccessor) const
{
    if (transAccessor != nullptr)
    {
        size_t i, j, k, neighborGridCellIdx;
        m_grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

        if (m_grid->cellIJKNeighbor(i, j, k, faceId, &neighborGridCellIdx))
        {
            double neighborCellValue = transAccessor->cellScalar(neighborGridCellIdx);
            if (neighborCellValue == HUGE_VAL && transAccessor->cellScalar(gridLocalCellIndex) != HUGE_VAL)
            {
                return 0.0;
            }
            else
            {
                return neighborCellValue;
            }
        }
        else
        {
            return 0.0;
        }
    }
    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessor::cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    switch (faceId)
    {
    case cvf::StructGridInterface::POS_I:
        {
            if (m_xTransAccessor.notNull())
            {
                return m_xTransAccessor->cellScalar(gridLocalCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_I:
        {
            return this->neighborCellTran(gridLocalCellIndex, cvf::StructGridInterface::NEG_I, m_xTransAccessor.p());
        }
        break;
    case cvf::StructGridInterface::POS_J:
        {
            if (m_yTransAccessor.notNull())
            {
                return m_yTransAccessor->cellScalar(gridLocalCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_J:
        {
            return this->neighborCellTran(gridLocalCellIndex, cvf::StructGridInterface::NEG_J, m_yTransAccessor.p());
        }
        break;
    case cvf::StructGridInterface::POS_K:
        {
            if (m_zTransAccessor.notNull())
            {
                return m_zTransAccessor->cellScalar(gridLocalCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_K:
        {
            return this->neighborCellTran(gridLocalCellIndex, cvf::StructGridInterface::NEG_K, m_zTransAccessor.p());
        }
        break;
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessor::cellScalarGlobIdx(size_t globCellIndex) const
{
    CVF_TIGHT_ASSERT(false);

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessor::cellFaceScalarGlobIdx(size_t globCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    size_t gridLocalCellIndex = m_grid->mainGrid()->cell(globCellIndex).gridLocalCellIndex();
    return this->cellFaceScalar(gridLocalCellIndex, faceId);
}

