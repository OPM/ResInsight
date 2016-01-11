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
    void updateCellResultColor(size_t timeStepIndex);


    void appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);
    void appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);
    void appendPolylinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);

private:
    void updatePartEffect();
    void generatePartGeometry();
    void computeData();

    cvf::Vec3d      extrusionDirection(const std::vector<cvf::Vec3d>& polyline) const;
    static void calculateEclipseTextureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                   const std::vector<size_t>& triangleToCellIdxMap, 
                                                   const RigResultAccessor* resultAccessor, 
                                                   const cvf::ScalarMapper* mapper);
    static void calculateGeoMechTextureCoords(cvf::Vec2fArray* textureCoords, 
                                              const std::vector<RivVertexWeights> &vertexWeights, 
                                              const std::vector<float> &resultValues, 
                                              bool isElementNodalResult, 
                                              const RigFemPart* femPart, 
                                              const cvf::ScalarMapper* mapper);
    cvf::ref<RivCrossSectionHexGridIntf> createHexGridInterface();
private:

    const RimCrossSection*      m_rimCrossSection;

    cvf::Color3f                m_defaultColor;

    cvf::ref<RivCrossSectionGeometryGenerator>   m_crossSectionGenerator;
    cvf::ref<cvf::Part>         m_crossSectionFaces;
    cvf::ref<cvf::Part>         m_crossSectionGridLines;
    cvf::ref<cvf::Vec2fArray>   m_crossSectionFacesTextureCoords;

    cvf::ref<cvf::Part>         m_highlightLineAlongPolyline;
    cvf::ref<cvf::Part>         m_highlightPointsForPolyline;
};
