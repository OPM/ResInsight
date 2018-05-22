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

#include "RigCombMultResultAccessor.h"

#include "RigGridBase.h"

#include <cmath>
#include "RigCell.h"
#include "RigMainGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCombMultResultAccessor::RigCombMultResultAccessor(const RigGridBase* grid)
    : m_grid(grid)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCombMultResultAccessor::setMultResultAccessors(
    RigResultAccessor* multXPosAccessor, RigResultAccessor* multXNegAccessor, 
    RigResultAccessor* multYPosAccessor, RigResultAccessor* multYNegAccessor, 
    RigResultAccessor* multZPosAccessor, RigResultAccessor* multZNegAccessor)
{
    m_multXPosAccessor = multXPosAccessor;
    m_multXNegAccessor = multXNegAccessor;
    m_multYPosAccessor = multYPosAccessor;
    m_multYNegAccessor = multYNegAccessor;
    m_multZPosAccessor = multZPosAccessor;
    m_multZNegAccessor = multZNegAccessor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombMultResultAccessor::cellScalar(size_t gridLocalCellIndex) const
{
    CVF_TIGHT_ASSERT(false);

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombMultResultAccessor::cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    size_t i, j, k, neighborGridCellIdx;
    m_grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

    double faceScalarThisCell = nativeMultScalar(gridLocalCellIndex, faceId);

    double faceScalarNeighborCell = 1.0;
    if (m_grid->cellIJKNeighbor(i, j, k, faceId, &neighborGridCellIdx))
    {
        faceScalarNeighborCell = nativeMultScalar(neighborGridCellIdx, cvf::StructGridInterface::oppositeFace(faceId));
    }

    return faceScalarThisCell * faceScalarNeighborCell;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombMultResultAccessor::nativeMultScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    double faceScalar = 1.0;

    switch (faceId)
    {
        case cvf::StructGridInterface::POS_I:
        {
            if (m_multXPosAccessor.notNull())
            {
                faceScalar = m_multXPosAccessor->cellScalar(gridLocalCellIndex);
            }
            break;
        }
        case cvf::StructGridInterface::NEG_I:
        {
            if (m_multXNegAccessor.notNull())
            {
                faceScalar = m_multXNegAccessor->cellScalar(gridLocalCellIndex);
            }
            break;
        }

        case cvf::StructGridInterface::POS_J:
        {
            if (m_multYPosAccessor.notNull())
            {
                faceScalar = m_multYPosAccessor->cellScalar(gridLocalCellIndex);
            }
            break;
        }
        case cvf::StructGridInterface::NEG_J:
        {
            if (m_multYNegAccessor.notNull())
            {
                faceScalar = m_multYNegAccessor->cellScalar(gridLocalCellIndex);
            }
            break;
        }

        case cvf::StructGridInterface::POS_K:
        {
            if (m_multZPosAccessor.notNull())
            {
                faceScalar = m_multZPosAccessor->cellScalar(gridLocalCellIndex);
            }
            break;
        }
        case cvf::StructGridInterface::NEG_K:
        {
            if (m_multZNegAccessor.notNull())
            {
                faceScalar = m_multZNegAccessor->cellScalar(gridLocalCellIndex);
            }
            break;
        }
    default:
        break;
    }

    // FaceScalar with value HUGE_VAL means value outside valid IJK-range. Clamp to 1.0 as this means no change in MULT factor.
    if (faceScalar == HUGE_VAL)
    {
        faceScalar = 1.0;
    }

    return faceScalar;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombMultResultAccessor::cellScalarGlobIdx(size_t globCellIndex) const
{
    CVF_TIGHT_ASSERT(false);

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombMultResultAccessor::cellFaceScalarGlobIdx(size_t globCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    size_t gridLocalCellIndex = m_grid->mainGrid()->cell(globCellIndex).gridLocalCellIndex();
    return this->cellFaceScalar(gridLocalCellIndex, faceId);
}

