/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfStructGridGeometryGenerator.h"
#include "RigGridBase.h"

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
/// RivGridGeometry: Class to handle visualization structures that embodies a specific grid at a specific time step.
/// frame on a certain level
/// LGR's have their own instance and the parent grid as well
///
//==================================================================================================

class RivTransmissibilityColorMapper 
{
public:
    static void transmissibilitiesTextureCoordinates(cvf::Vec2fArray* textureCoords, 
        const cvf::StructGridScalarDataAccess* dataAccessObjectTranX, 
        const cvf::StructGridScalarDataAccess* dataAccessObjectTranY, 
        const cvf::StructGridScalarDataAccess* dataAccessObjectTranZ, 
        const cvf::ScalarMapper* mapper, 
        const std::vector<cvf::StructGridInterface::FaceType>& m_quadsToFaceTypes,
        const std::vector<size_t>&            m_quadsToGridCells
        );
};


class RivGridPartMgr: public cvf::Object
{
public:
    RivGridPartMgr(const RigGridBase* grid, size_t gridIdx, const RimFaultCollection* rimFaultCollection);
    ~RivGridPartMgr();
    void setTransform(cvf::Transform* scaleTransform);
    void setCellVisibility(cvf::UByteArray* cellVisibilities );
    cvf::ref<cvf::UByteArray>  cellVisibility() { return  m_cellVisibility;}

    void updateCellColor(cvf::Color4f color);
    void updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot);
    void updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, 
        RimCellEdgeResultSlot* cellEdgeResultSlot);

    void appendPartsToModel(cvf::ModelBasicList* model);

    void updateTransmissibilityCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot);

private:
    void generatePartGeometry(cvf::StructGridGeometryGenerator& geoBuilder, bool faultGeometry);
    
    

private:
    size_t                                      m_gridIdx;
    cvf::cref<RigGridBase>                      m_grid;

    cvf::ref<cvf::Transform>                    m_scaleTransform;
    float                                       m_opacityLevel;
    cvf::Color3f                                m_defaultColor;

    // Surface visualization
    cvf::StructGridGeometryGenerator            m_surfaceGenerator;
    RigGridCellFaceVisibilityFilter             m_surfaceFaceFilter;
    cvf::ref<cvf::Part>                         m_surfaceFaces;
    cvf::ref<cvf::Vec2fArray>                   m_surfaceFacesTextureCoords;

    cvf::ref<cvf::Part>                         m_surfaceGridLines;

    // Fault visualization
    cvf::StructGridGeometryGenerator            m_faultGenerator;
    RigFaultFaceVisibilityFilter                m_faultFaceFilter;
    cvf::ref<cvf::Part>                         m_faultFaces;
    cvf::ref<cvf::Vec2fArray>                   m_faultFacesTextureCoords;

    cvf::ref<cvf::Part>                         m_faultGridLines;

    cvf::ref<cvf::UByteArray>                   m_cellVisibility;

    const RimFaultCollection*                   m_rimFaultCollection;
};
