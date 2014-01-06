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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include "RigGridBase.h"
#include "RivFaultPartMgr.h"

namespace cvf
{
    class Transform;
}

class RimResultSlot;
class RimCellEdgeResultSlot;
class RimFaultCollection;

//==================================================================================================
///
//==================================================================================================
class RivReservoirFaultsPartMgr : public cvf::Object
{
public:
    RivReservoirFaultsPartMgr(const RigMainGrid* grid, const RimFaultCollection* faultCollection);
    ~RivReservoirFaultsPartMgr();

    void setTransform(cvf::Transform* scaleTransform);
    void setCellVisibility(cvf::UByteArray* cellVisibilities);
    void setFilterPart(bool filterPart);

    void applySingleColorEffect();
    void updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot);
    void updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, 
        RimCellEdgeResultSlot* cellEdgeResultSlot);

    void appendPartsToModel(cvf::ModelBasicList* model);

    void appendLabelPartsToModel(cvf::ModelBasicList* model);


private:
    cvf::ref<cvf::Transform>        m_scaleTransform;
    const RimFaultCollection*       m_faultCollection;
    cvf::Collection<RivFaultPartMgr>   m_faultParts;

    bool m_isFilterPart;
};
