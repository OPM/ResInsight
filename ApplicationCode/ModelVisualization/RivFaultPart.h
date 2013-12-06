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
#include "cvfColor4.h"

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

    void setCellVisibility(cvf::UByteArray* cellVisibilities);

    void updateCellColor(cvf::Color4f color);
    void updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot);
    void updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot);

    void setShowNativeFaces(bool showNativeFaces);
    void setShowOppositeFaces(bool showOppositeFaces);

    void appendPartsToModel(cvf::ModelBasicList* model);

private:
    void generatePartGeometry();
    void updatePartEffect();

private:
    cvf::cref<RigGridBase>      m_grid;
    const RimFault*             m_rimFault;

    float                       m_opacityLevel;
    cvf::Color4f                m_defaultColor;

    bool                        m_showNativeFaces;
    bool                        m_showOppositeFaces;

    cvf::ref<cvf::UByteArray>   m_cellVisibility;

    RivFaultGeometryGenerator   m_nativeFaultGenerator;
    cvf::ref<cvf::Part>         m_nativeFaultFaces;
    cvf::ref<cvf::Part>         m_nativeFaultGridLines;
    cvf::ref<cvf::Vec2fArray>   m_nativeFaultFacesTextureCoords;

    RivFaultGeometryGenerator   m_oppositeFaultGenerator;
    cvf::ref<cvf::Part>         m_oppositeFaultFaces;
    cvf::ref<cvf::Part>         m_oppositeFaultGridLines;
    cvf::ref<cvf::Vec2fArray>   m_oppositeFaultFacesTextureCoords;
};
