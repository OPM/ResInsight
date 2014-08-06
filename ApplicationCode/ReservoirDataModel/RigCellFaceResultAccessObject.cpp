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

#include "RigCellFaceResultAccessObject.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCellFaceResultAccessObject::RigCellFaceResultAccessObject(const QString& resultName)
    : m_resultName(resultName)
{
    m_resultAccessObjects.resize(6);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCellFaceResultAccessObject::setDataAccessObjectForFace(cvf::StructGridInterface::FaceType faceId, RigResultAccessObject* resultAccessObject)
{
    m_resultAccessObjects[faceId] = resultAccessObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCellFaceResultAccessObject::cellScalar(size_t localCellIndex) const
{

    // TODO: How to handle when we get here?
    CVF_ASSERT(false);

    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCellFaceResultAccessObject::cellFaceScalar(size_t localCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    const RigResultAccessObject* resultAccessObj = m_resultAccessObjects.at(faceId);
    if (resultAccessObj != NULL)
    {
        return resultAccessObj->cellFaceScalar(localCellIndex, faceId);
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigCellFaceResultAccessObject::resultName() const
{
    return m_resultName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCellFaceResultAccessObject::setCellScalar(size_t localCellIndex, double scalarValue)
{
    // TODO: How to handle when we get here?
    CVF_ASSERT(false);

}

