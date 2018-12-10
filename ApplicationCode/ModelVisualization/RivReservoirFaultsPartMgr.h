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

#include "cvfBase.h"
#include "cvfArray.h"
#include "cvfCollection.h"
#include "cvfObject.h"

#include "cafPdmPointer.h"

namespace cvf
{
    class Transform;
    class ModelBasicList;
}

class RimEclipseCellColors;
class RimCellEdgeColors;
class RimEclipseView;
class RivFaultPartMgr;
class RigMainGrid;

//==================================================================================================
///
//==================================================================================================
class RivReservoirFaultsPartMgr : public cvf::Object
{
public:
    RivReservoirFaultsPartMgr(const RigMainGrid* grid, RimEclipseView* reservoirView);
    ~RivReservoirFaultsPartMgr() override;

    void setTransform(cvf::Transform* scaleTransform);
    void setCellVisibility(cvf::UByteArray* cellVisibilities);

    void forceWatertightGeometryOn();
    void clearWatertightGeometryFlag();

    void setOpacityLevel(float opacity);
    void applySingleColorEffect();
    void updateColors(size_t timeStepIndex, RimEclipseCellColors* cellResultColors);
    void updateCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, 
        RimCellEdgeColors* cellEdgeResultColors);

    void appendPartsToModel(cvf::ModelBasicList* model);

    void appendLabelPartsToModel(cvf::ModelBasicList* model);


private:
    cvf::ref<cvf::Transform>            m_scaleTransform;
    caf::PdmPointer<RimEclipseView>   m_reservoirView;
    cvf::Collection<RivFaultPartMgr>    m_faultParts;
    bool                                m_forceWatertightGeometry;
};
