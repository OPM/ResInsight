/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RivIjkIntersectionGeometryGenerator.h"

#include "cvfCollection.h"
#include "cvfObject.h"

namespace cvf
{
class ModelBasicList;
class Transform;
class Part;
} // namespace cvf

class RimIjkIntersection;
class RivIntersectionGeometryGeneratorInterface;

//==================================================================================================
///
///
//==================================================================================================
class RivIjkIntersectionPartMgr : public cvf::Object
{
public:
    explicit RivIjkIntersectionPartMgr( RimIjkIntersection* intersection );

    void applySingleColorEffect();
    void updateCellResultColor( int timeStepIndex );

    void appendNativeIntersectionFacesToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void appendMeshLinePartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void appendAnnotationPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );

    const RivIntersectionGeometryGeneratorInterface* intersectionGeometryGenerator() const;

    void generatePartGeometry( cvf::UByteArray* visibleCells, cvf::Transform* scaleTransform );

private:
    void updatePartEffect();

private:
    RimIjkIntersection* m_rimIntersection;

    cvf::Color3f m_defaultColor;

    cvf::ref<cvf::Part>        m_intersectionFaces;
    cvf::ref<cvf::Part>        m_intersectionGridLines;
    cvf::Collection<cvf::Part> m_annotationParts;
    cvf::ref<cvf::Vec2fArray>  m_intersectionFacesTextureCoords;

    cvf::ref<RivIjkIntersectionGeometryGenerator> m_intersectionGenerator;
};
