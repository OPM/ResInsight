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

#include "cvfCollection.h"

class RigGridBase;


//==================================================================================================
/// 
//==================================================================================================
class RigCombTransResultAccessor : public RigResultAccessor
{
public:
    explicit RigCombTransResultAccessor(const RigGridBase* grid);

    void setTransResultAccessors(RigResultAccessor* xTransAccessor,
                                 RigResultAccessor* yTransAccessor,
                                 RigResultAccessor* zTransAccessor);

    virtual double  cellScalar(size_t gridLocalCellIndex) const;
    virtual double  cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const;
    virtual double  cellScalarGlobIdx(size_t globCellIndex) const;
    virtual double  cellFaceScalarGlobIdx(size_t globCellIndex, cvf::StructGridInterface::FaceType faceId) const;

private:
    double neighborCellTran(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId, const RigResultAccessor* transAccessor) const;


    cvf::ref<RigResultAccessor> m_xTransAccessor;
    cvf::ref<RigResultAccessor> m_yTransAccessor;
    cvf::ref<RigResultAccessor> m_zTransAccessor;

    const RigGridBase* m_grid;
};
