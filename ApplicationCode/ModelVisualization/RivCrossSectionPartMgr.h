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

#include "RivCrossSectionGeometryGenerator.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfColor4.h"
#include "cvfVector3.h"


namespace cvf
{
    class ModelBasicList;
    class Transform;
    class Part;
}

class RigMainGrid;
class RimEclipseCellColors;
class RimCellEdgeColors;
class RimCrossSection;

//==================================================================================================
///
///
//==================================================================================================

class RivCrossSectionPartMgr : public cvf::Object
{
public:
    RivCrossSectionPartMgr(const RimCrossSection* rimCrossSection);

    void applySingleColorEffect();
    void updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors);

    void appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);
    void appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);

private:
    void updatePartEffect();
    void generatePartGeometry();
    void computeData();

    RigMainGrid*    mainGrid();
    cvf::Vec3d      extrusionDirection(const std::vector<cvf::Vec3d>& polyline) const;

private:

    cvf::cref<RigMainGrid>      m_grid;
    const RimCrossSection*      m_rimCrossSection;

    cvf::Color3f                m_defaultColor;

    cvf::ref<RivCrossSectionGeometryGenerator>   m_nativeCrossSectionGenerator;
    cvf::ref<cvf::Part>         m_nativeCrossSectionFaces;
    cvf::ref<cvf::Part>         m_nativeCrossSectionGridLines;
    cvf::ref<cvf::Vec2fArray>   m_nativeCrossSectionFacesTextureCoords;

};
