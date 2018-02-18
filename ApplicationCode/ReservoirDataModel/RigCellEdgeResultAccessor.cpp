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

#include "RigCellEdgeResultAccessor.h"

#include <cmath>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCellEdgeResultAccessor::RigCellEdgeResultAccessor()
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCellEdgeResultAccessor::setDataAccessObjectForFace(cvf::StructGridInterface::FaceType faceId, RigResultAccessor* resultAccessObject)
{
    m_resultAccessObjects[faceId] = resultAccessObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCellEdgeResultAccessor::cellScalar(size_t gridLocalCellIndex) const
{

    // TODO: How to handle when we get here?
    CVF_ASSERT(false);

    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCellEdgeResultAccessor::cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    const RigResultAccessor* resultAccessObj = m_resultAccessObjects[faceId].p();
    if (resultAccessObj != nullptr)
    {
        return resultAccessObj->cellFaceScalar(gridLocalCellIndex, faceId);
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCellEdgeResultAccessor::cellScalarGlobIdx(size_t globCellIndex) const
{
    // TODO: How to handle when we get here?
    CVF_ASSERT(false);

    return cvf::UNDEFINED_DOUBLE;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCellEdgeResultAccessor::cellFaceScalarGlobIdx(size_t globCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    const RigResultAccessor* resultAccessObj = m_resultAccessObjects[faceId].p();
    if (resultAccessObj != nullptr)
    {
        return resultAccessObj->cellFaceScalarGlobIdx(globCellIndex, faceId);
    }

    return HUGE_VAL;
}

