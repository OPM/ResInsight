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

#include "RigCombTransResultAccessor.h"

#include "RigGridBase.h"

#include <cmath>


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
double RigCombTransResultAccessor::cellScalar(size_t localCellIndex) const
{
    CVF_TIGHT_ASSERT(false);

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessor::cellFaceScalar(size_t localCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    switch (faceId)
    {
    case cvf::StructGridInterface::POS_I:
        {
            if (m_xTransAccessor.notNull())
            {
                return m_xTransAccessor->cellScalar(localCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_I:
        {
            if (m_xTransAccessor.notNull())
            {
                size_t i, j, k, neighborGridCellIdx;
                m_grid->ijkFromCellIndex(localCellIndex, &i, &j, &k);

                if (m_grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_I, &neighborGridCellIdx))
                {
                    return m_xTransAccessor->cellScalar(neighborGridCellIdx);
                }
            }
        }
        break;
    case cvf::StructGridInterface::POS_J:
        {
            if (m_yTransAccessor.notNull())
            {
                return m_yTransAccessor->cellScalar(localCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_J:
        {
            if (m_yTransAccessor.notNull())
            {
                size_t i, j, k, neighborGridCellIdx;
                m_grid->ijkFromCellIndex(localCellIndex, &i, &j, &k);

                if (m_grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_J, &neighborGridCellIdx))
                {
                    return m_yTransAccessor->cellScalar(neighborGridCellIdx);
                }
            }
        }
        break;
    case cvf::StructGridInterface::POS_K:
        {
            if (m_zTransAccessor.notNull())
            {
                return m_zTransAccessor->cellScalar(localCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_K:
        {
            if (m_zTransAccessor.notNull())
            {
                size_t i, j, k, neighborGridCellIdx;
                m_grid->ijkFromCellIndex(localCellIndex, &i, &j, &k);

                if (m_grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_K, &neighborGridCellIdx))
                {
                    return m_zTransAccessor->cellScalar(neighborGridCellIdx);
                }
            }
        }
        break;
    }

    return HUGE_VAL;
}

