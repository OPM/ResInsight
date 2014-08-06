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

#include "RigCombTransResultAccessObject.h"

#include "RigGridBase.h"

#include <cmath>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCombTransResultAccessObject::RigCombTransResultAccessObject(const RigGridBase* grid, const QString& resultName)
    : m_grid(grid),
    m_resultName(resultName)
{
    m_resultAccessObjects.resize(6);
}

//--------------------------------------------------------------------------------------------------
/// Only sensible to provide the positive values, as the negative ones will never be used.
/// The negative faces gets their value from the neighbor cell in that direction
//--------------------------------------------------------------------------------------------------
void RigCombTransResultAccessObject::setDataAccessObjectForFace(cvf::StructGridInterface::FaceType faceId, RigResultAccessObject* resultAccessObject)
{
    CVF_ASSERT(faceId == cvf::StructGridInterface::POS_I || faceId == cvf::StructGridInterface::POS_J || faceId == cvf::StructGridInterface::POS_K );

    m_resultAccessObjects[faceId] = resultAccessObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessObject::cellScalar(size_t localCellIndex) const
{

    // TODO: How to handle when we get here?
    CVF_ASSERT(false);

    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombTransResultAccessObject::cellFaceScalar(size_t localCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    switch (faceId)
    {
    case cvf::StructGridInterface::POS_I:
        {
            const RigResultAccessObject* resultAccessObj = m_resultAccessObjects.at(cvf::StructGridInterface::POS_I);
            if (resultAccessObj)
            {
                return resultAccessObj->cellScalar(localCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_I:
        {
            const RigResultAccessObject* resultAccessObj = m_resultAccessObjects.at(cvf::StructGridInterface::POS_I);
            if (resultAccessObj)
            {
                size_t i, j, k, neighborGridCellIdx;
                m_grid->ijkFromCellIndex(localCellIndex, &i, &j, &k);

                if (m_grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_I, &neighborGridCellIdx))
                {
                    return resultAccessObj->cellScalar(neighborGridCellIdx);
                }
            }
        }
        break;
    case cvf::StructGridInterface::POS_J:
        {
            const RigResultAccessObject* resultAccessObj = m_resultAccessObjects.at(cvf::StructGridInterface::POS_J);
            if (resultAccessObj)
            {
                return resultAccessObj->cellScalar(localCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_J:
        {
            const RigResultAccessObject* resultAccessObj = m_resultAccessObjects.at(cvf::StructGridInterface::POS_J);
            if (resultAccessObj)
            {
                size_t i, j, k, neighborGridCellIdx;
                m_grid->ijkFromCellIndex(localCellIndex, &i, &j, &k);

                if (m_grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_J, &neighborGridCellIdx))
                {
                    return resultAccessObj->cellScalar(neighborGridCellIdx);
                }
            }
        }
        break;
    case cvf::StructGridInterface::POS_K:
        {
            const RigResultAccessObject* resultAccessObj = m_resultAccessObjects.at(cvf::StructGridInterface::POS_K);
            if (resultAccessObj)
            {
                return resultAccessObj->cellScalar(localCellIndex);
            }
        }
        break;
    case cvf::StructGridInterface::NEG_K:
        {
            const RigResultAccessObject* resultAccessObj = m_resultAccessObjects.at(cvf::StructGridInterface::POS_K);
            if (resultAccessObj)
            {
                size_t i, j, k, neighborGridCellIdx;
                m_grid->ijkFromCellIndex(localCellIndex, &i, &j, &k);

                if (m_grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_K, &neighborGridCellIdx))
                {
                    return resultAccessObj->cellScalar(neighborGridCellIdx);
                }
            }
        }
        break;
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigCombTransResultAccessObject::resultName() const
{
    return m_resultName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCombTransResultAccessObject::setCellScalar(size_t localCellIndex, double scalarValue)
{
    // TODO: How to handle when we get here?
    CVF_ASSERT(false);

}

