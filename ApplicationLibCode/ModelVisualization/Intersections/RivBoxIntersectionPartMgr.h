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

#include "RivBoxIntersectionGeometryGenerator.h"

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
class RimBoxIntersection;
class RimIntersection;
class RimEclipseView;
class RimGeoMechView;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;

//==================================================================================================
///
///
//==================================================================================================

class RivBoxIntersectionPartMgr : public cvf::Object
{
public:
    explicit RivBoxIntersectionPartMgr( RimBoxIntersection* intersectionBox );

    void applySingleColorEffect();
    void updateCellResultColor( size_t timeStepIndex );

    void appendNativeIntersectionFacesToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void appendMeshLinePartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );

    const RivIntersectionGeometryGeneratorIF* intersectionGeometryGenerator() const;

private:
    void updatePartEffect();
    void generatePartGeometry();

private:
    RimBoxIntersection* m_rimIntersectionBox;

    cvf::Color3f m_defaultColor;

    cvf::ref<cvf::Part>       m_intersectionBoxFaces;
    cvf::ref<cvf::Part>       m_intersectionBoxGridLines;
    cvf::ref<cvf::Vec2fArray> m_intersectionBoxFacesTextureCoords;

    cvf::ref<RivBoxIntersectionGeometryGenerator> m_intersectionBoxGenerator;
};
