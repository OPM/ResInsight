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

#pragma once

#include "RigResultAccessor.h"

#include <array>


//==================================================================================================
/// 
//==================================================================================================
class RigCellEdgeResultAccessor : public RigResultAccessor
{
public:
    RigCellEdgeResultAccessor();

    void setDataAccessObjectForFace(cvf::StructGridInterface::FaceType faceId, RigResultAccessor* resultAccessObject);

    double  cellScalar(size_t gridLocalCellIndex) const override;
    double  cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const override;

    double cellScalarGlobIdx(size_t globCellIndex) const override;
    double cellFaceScalarGlobIdx(size_t globCellIndex, cvf::StructGridInterface::FaceType faceId) const override;

private:
    std::array<cvf::ref<RigResultAccessor>, 6> m_resultAccessObjects;
};
