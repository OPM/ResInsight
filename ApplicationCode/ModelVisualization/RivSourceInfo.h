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

#include "RivCellSetEnum.h"
#include "RivObjectSourceInfo.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfObject.h"

#include "cvfStructGridGeometryGenerator.h"

//==================================================================================================
///
///
//==================================================================================================
class RivSourceInfo : public RivObjectSourceInfo
{
public:
    explicit RivSourceInfo(caf::PdmObject* pdmObject, size_t gridIndex);

    size_t gridIndex() const;
    bool   hasCellFaceMapping() const;
    bool   hasNNCIndices() const;

    RivCellSetEnum cellSetType() const;
    void           setCellSetType(RivCellSetEnum cellSetType);

public:
    cvf::cref<cvf::StuctGridTriangleToCellFaceMapper> m_cellFaceFromTriangleMapper;
    cvf::ref<cvf::Array<size_t>>                      m_NNCIndices;

private:
    size_t         m_gridIndex;
    RivCellSetEnum m_cellSetType;
};
