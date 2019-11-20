/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RivIntersectionBoxGeometryGenerator.h"

#include "cvfObject.h"

namespace cvf
{
class ModelBasicList;
class Transform;
class Part;
class ScalarMapper;
} // namespace cvf

class RigMainGrid;
class RigResultAccessor;

class RivTernaryScalarMapper;

class RimCellEdgeColors;
class RimEclipseCellColors;
class RimIntersectionBox;
class RimIntersectionHandle;
class RimEclipseView;
class RimGeoMechView;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;


//==================================================================================================
///
///
//==================================================================================================

class RivIntersectionBoxPartMgr : public cvf::Object
{
public:
    explicit RivIntersectionBoxPartMgr( RimIntersectionBox* intersectionBox );

    void        applySingleColorEffect();
    void        updateCellResultColor( size_t timeStepIndex );

    void appendNativeCrossSectionFacesToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void appendMeshLinePartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );

private:
    void updatePartEffect();
    void generatePartGeometry();

    static void updateCellResultColorStatic( size_t                                    timeStepIndex,
                                             RimIntersectionHandle*                    m_rimIntersectionBox,
                                             const RivIntersectionGeometryGeneratorIF* m_intersectionBoxGenerator,
                                             cvf::Part*                                m_intersectionBoxFaces,
                                             cvf::Vec2fArray* m_intersectionBoxFacesTextureCoords );
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

    static void updateGeoMechCellResultColors( const RimGeoMechResultDefinition*         geomResultDef,
                                               size_t                                    timeStepIndex,
                                               const cvf::ScalarMapper*                  scalarColorMapper,
                                               bool                                      isLightingDisabled,
                                               const RivIntersectionGeometryGeneratorIF* geomGenerator,
                                               cvf::Part*                                m_intersectionBoxFaces,
                                               cvf::Vec2fArray* m_intersectionBoxFacesTextureCoords );

private:
    RimIntersectionBox* m_rimIntersectionBox;

    cvf::Color3f m_defaultColor;

    cvf::ref<cvf::Part>       m_intersectionBoxFaces;
    cvf::ref<cvf::Part>       m_intersectionBoxGridLines;
    cvf::ref<cvf::Vec2fArray> m_intersectionBoxFacesTextureCoords;

    cvf::ref<RivIntersectionBoxGeometryGenerator> m_intersectionBoxGenerator;
};
