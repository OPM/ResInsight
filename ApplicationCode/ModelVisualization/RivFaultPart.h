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
#include "RimFault.h"
#include "RivFaultGeometryGenerator.h"

namespace cvf
{
    class StructGridInterface;
    class ModelBasicList;
    class Transform;
    class Part;
}

class RimResultSlot;
class RimCellEdgeResultSlot;
class RimFaultCollection;

//==================================================================================================
///
///
//==================================================================================================

class RivFaultPart : public cvf::Object
{
public:
    RivFaultPart(const RigGridBase* grid, const RimFault* rimFault);

    void setCellVisibility(cvf::UByteArray* cellVisibilities );

    void updateCellColor(cvf::Color4f color);
    void updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot);
    void updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot);

    void appendPartsToModel(cvf::ModelBasicList* model);

private:
    void generatePartGeometry();

private:
    cvf::cref<RigGridBase>      m_grid;

    RivFaultGeometryGenerator   m_faultGenerator;
    cvf::ref<cvf::Part>         m_faultFaces;
    cvf::ref<cvf::Vec2fArray>   m_faultFacesTextureCoords;

    float                       m_opacityLevel;
    cvf::Color3f                m_defaultColor;


    cvf::ref<cvf::Part>         m_faultGridLines;

    cvf::ref<cvf::UByteArray>   m_cellVisibility;

    const RimFault*             m_rimFault;
};
