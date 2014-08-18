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

#include "RigCombRiTransResultAccessor.h"

#include "RigGridBase.h"

#include <cmath>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCombRiTransResultAccessor::RigCombRiTransResultAccessor(const RigGridBase* grid)
    : m_grid(grid)
{
    m_cdarchy = 0.008527; // (ECLIPSE 100) (METRIC)
}

//--------------------------------------------------------------------------------------------------
/// Only sensible to provide the positive values, as the negative ones will never be used.
/// The negative faces gets their value from the neighbor cell in that direction
//--------------------------------------------------------------------------------------------------
void RigCombRiTransResultAccessor::setPermResultAccessors( RigResultAccessor* xPermAccessor,
                                                           RigResultAccessor* yPermAccessor,
                                                           RigResultAccessor* zPermAccessor)

{
    m_xPermAccessor = xPermAccessor;
    m_yPermAccessor = yPermAccessor;
    m_zPermAccessor = zPermAccessor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCombRiTransResultAccessor::setNTGResultAccessor(RigResultAccessor* ntgAccessor)
{
    m_ntgAccessor = ntgAccessor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::cellScalar(size_t gridLocalCellIndex) const
{
    CVF_TIGHT_ASSERT(false);

    return HUGE_VAL;
}

double RigCombRiTransResultAccessor::getPermValue(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId)
{
    switch (faceId)
    {
    case cvf::StructGridInterface::POS_I:
    case cvf::StructGridInterface::NEG_I:
        {
            return m_xPermAccessor->cellScalar(gridLocalCellIndex);
        }
        break;
    case cvf::StructGridInterface::POS_J:
    case cvf::StructGridInterface::NEG_J:
        {
            return m_yPermAccessor->cellScalar(gridLocalCellIndex);
        }
        break;
    case cvf::StructGridInterface::POS_K:
    case cvf::StructGridInterface::NEG_K:
        {
            return zPermAccessor->cellScalar(gridLocalCellIndex);
        }
        break;
    }
    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::calculateHalfCellTrans( size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId)
{
    double perm = getPermValue(gridLocalCellIndex, faceId);
    double ntg = 1.0;
    
    if (faceId != cvf::StructGridInterface::POS_K && faceId != cvf::StructGridInterface::NEG_K) 
    {
        m_ntgAccessor->cellScalar(gridLocalCellIndex);
    }

    RigCell& cell = m_grid->cell(gridLocalCellIndex);
    
    cvf::Vec3d centerToFace = cell.faceCenter(faceId) - cell.center();
    cvf::Vec3d faceAreaVec = cell.faceNormalWithAreaLenght(faceId);

    double halfCellTrans = perm*ntg*(faceAreaVec*centerToFace) / (centerToFace*centerToFace);

    return halfCellTrans;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
    RigFault* fault = m_grid->mainGrid()->findFaultFromCellIndexAndCellFace(reservoirCellIndex, faceId);

    if (fault)
    {

    }
    else
    {
        size_t i, j, k, neighborGridCellIdx;
        m_grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

        if (m_grid->cellIJKNeighbor(i, j, k, faceId, &neighborGridCellIdx))
        {
            double halfCellTrans = calculateHalfCellTrans(gridLocalCellIndex, faceId);
            double neighborHalfCellTrans = calculateHalfCellTrans(neighborGridCellIdx, cvf::StructGridInterface::oppositeFace(faceId));

            return m_cdarchy/ ( ( 1 / halfCellTrans) + (1 / neighborHalfCellTrans) );
        }
    }

    return HUGE_VAL;
}