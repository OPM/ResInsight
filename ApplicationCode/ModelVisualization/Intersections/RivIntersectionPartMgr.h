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
#include "cvfObject.h"
#include "cvfColor4.h"
#include "cvfVector3.h"
#include "cvfArray.h"
#include "cafPdmPointer.h"


namespace cvf
{
    class ModelBasicList;
    class Transform;
    class Part;
    class ScalarMapper;
}

class RigFemPart;
class RigFemResultAddress;
class RigGeoMechCaseData;
class RigMainGrid;
class RigResultAccessor;
class RimCellEdgeColors;
class RimEclipseCellColors;
class RimIntersection;
class RivTernaryScalarMapper;
class RivIntersectionGeometryGenerator;
class RivIntersectionHexGridInterface;
class RivIntersectionVertexWeights;


//==================================================================================================
///
///
//==================================================================================================

class RivIntersectionPartMgr : public cvf::Object
{
public:
    explicit RivIntersectionPartMgr(RimIntersection* rimCrossSection, bool isFlattened = false);

    void applySingleColorEffect();
    void updateCellResultColor(size_t timeStepIndex, 
                               const cvf::ScalarMapper* scalarColorMapper, 
                               const RivTernaryScalarMapper* ternaryColorMapper);


    void appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);
    void appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);
    void appendPolylinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);
    
    const RimIntersection* intersection() const;

private:
    void updatePartEffect();
    void generatePartGeometry();

    void createPolyLineParts(bool useBufferObjects);
    void createExtrusionDirParts(bool useBufferObjects);

    void computeData();

public:
    static void calculateEclipseTextureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                   const std::vector<size_t>& triangleToCellIdxMap, 
                                                   const RigResultAccessor* resultAccessor, 
                                                   const cvf::ScalarMapper* mapper);

    static void calculateNodeOrElementNodeBasedGeoMechTextureCoords(cvf::Vec2fArray* textureCoords, 
                                                                    const std::vector<RivIntersectionVertexWeights> &vertexWeights,
                                                                    const std::vector<float> &resultValues,
                                                                    bool isElementNodalResult,
                                                                    const RigFemPart* femPart,
                                                                    const cvf::ScalarMapper* mapper);

    static void calculateElementBasedGeoMechTextureCoords(cvf::Vec2fArray* textureCoords,
                                                          const std::vector<float> &resultValues,
                                                          const std::vector<size_t>& triangleToCellIdx,
                                                          const cvf::ScalarMapper* mapper);

    static void calculateGeoMechTensorXfTextureCoords(cvf::Vec2fArray* textureCoords, 
                                                      const cvf::Vec3fArray* triangelVertices,
                                                      const std::vector<RivIntersectionVertexWeights> &vertexWeights,
                                                      RigGeoMechCaseData* caseData,
                                                      const RigFemResultAddress& resVarAddress,
                                                      int   timeStepIdx,
                                                      const cvf::ScalarMapper* mapper);

    static void calculatePlaneAngleTextureCoords(cvf::Vec2fArray* textureCoords, 
                                                 const cvf::Vec3fArray* triangelVertices, 
                                                 const RigFemResultAddress& resVarAddress, 
                                                 const cvf::ScalarMapper* mapper);

private:
    cvf::ref<RivIntersectionHexGridInterface> createHexGridInterface();

    caf::PdmPointer<RimIntersection> m_rimCrossSection;

    cvf::Color3f                m_defaultColor;

    cvf::ref<RivIntersectionGeometryGenerator>   m_crossSectionGenerator;
    cvf::ref<cvf::Part>         m_crossSectionFaces;
    cvf::ref<cvf::Part>         m_crossSectionGridLines;
    cvf::ref<cvf::Vec2fArray>   m_crossSectionFacesTextureCoords;

    cvf::ref<cvf::Part>         m_highlightLineAlongPolyline;
    cvf::ref<cvf::Part>         m_highlightPointsForPolyline;

    cvf::ref<cvf::Part>         m_highlightLineAlongExtrusionDir;
    cvf::ref<cvf::Part>         m_highlightPointsForExtrusionDir;

    bool                        m_isFlattened;
};

