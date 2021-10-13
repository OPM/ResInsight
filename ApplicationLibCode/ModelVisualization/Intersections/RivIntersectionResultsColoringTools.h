/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cvfArray.h"

class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;
class RimIntersection;

class RivIntersectionGeometryGeneratorInterface;
class RivIntersectionVertexWeights;
class RivTernaryScalarMapper;

class RigResultAccessor;
class RigFemPartCollection;
class RigGeoMechCaseData;
class RigFemResultAddress;

namespace cvf
{
class Part;
class ScalarMapper;
} // namespace cvf

class RivIntersectionResultsColoringTools
{
public:
    static void calculateIntersectionResultColors( size_t           timeStepIndex,
                                                   bool             useSeparateIntersectionResDefTimeStep,
                                                   RimIntersection* rimIntersectionHandle,
                                                   const RivIntersectionGeometryGeneratorInterface* intersectionGeomGenIF,
                                                   const cvf::ScalarMapper*      explicitScalarColorMapper,
                                                   const RivTernaryScalarMapper* explicitTernaryColorMapper,
                                                   cvf::Part*                    intersectionFacesPart,
                                                   cvf::Vec2fArray*              intersectionFacesTextureCoords );

private:
    static void updateEclipseCellResultColors( const RimEclipseResultDefinition* eclipseResDef,
                                               const cvf::ScalarMapper*          scalarColorMapper,
                                               size_t                            timeStepIndex,
                                               bool                              isLightingDisabled,
                                               const std::vector<size_t>&        triangleToCellIndexMapping,
                                               cvf::Part*                        m_intersectionBoxFaces,
                                               cvf::Vec2fArray*                  m_intersectionBoxFacesTextureCoords );

    static void updateEclipseTernaryCellResultColors( const RimEclipseResultDefinition* eclipseResDef,
                                                      const RivTernaryScalarMapper*     ternaryColorMapper,
                                                      size_t                            timeStepIndex,
                                                      bool                              isLightingDisabled,
                                                      const std::vector<size_t>&        triangleToCellIndexMapping,
                                                      cvf::Part*                        m_intersectionBoxFaces,
                                                      cvf::Vec2fArray* m_intersectionBoxFacesTextureCoords );

    static void updateGeoMechCellResultColors( const RimGeoMechResultDefinition*                geomResultDef,
                                               size_t                                           timeStepIndex,
                                               const cvf::ScalarMapper*                         scalarColorMapper,
                                               bool                                             isLightingDisabled,
                                               const RivIntersectionGeometryGeneratorInterface* geomGenerator,
                                               cvf::Part*                                       m_intersectionBoxFaces,
                                               cvf::Vec2fArray* m_intersectionBoxFacesTextureCoords );

    static void calculateEclipseTextureCoordinates( cvf::Vec2fArray*           textureCoords,
                                                    const std::vector<size_t>& triangleToCellIdxMap,
                                                    const RigResultAccessor*   resultAccessor,
                                                    const cvf::ScalarMapper*   mapper );

    static void
        calculateNodeOrElementNodeBasedGeoMechTextureCoords( cvf::Vec2fArray* textureCoords,
                                                             const std::vector<RivIntersectionVertexWeights>& vertexWeights,
                                                             const std::vector<float>&   resultValues,
                                                             bool                        isElementNodalResult,
                                                             const RigFemPartCollection* femParts,
                                                             const cvf::ScalarMapper*    mapper );

    static void calculateElementBasedGeoMechTextureCoords( cvf::Vec2fArray*           textureCoords,
                                                           const std::vector<float>&  resultValues,
                                                           const std::vector<size_t>& triangleToCellIdx,
                                                           const cvf::ScalarMapper*   mapper );

    static void calculateGeoMechTensorXfTextureCoords( cvf::Vec2fArray*       textureCoords,
                                                       const cvf::Vec3fArray* triangelVertices,
                                                       const std::vector<RivIntersectionVertexWeights>& vertexWeights,
                                                       RigGeoMechCaseData*                              caseData,
                                                       const RigFemResultAddress&                       resVarAddress,
                                                       int                                              timeStepIdx,
                                                       const cvf::ScalarMapper*                         mapper );

    static void calculatePlaneAngleTextureCoords( cvf::Vec2fArray*           textureCoords,
                                                  const cvf::Vec3fArray*     triangelVertices,
                                                  const RigFemResultAddress& resVarAddress,
                                                  const cvf::ScalarMapper*   mapper );
};
