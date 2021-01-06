/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafPdmPointer.h"
#include "cvfArray.h"
#include "cvfObject.h"

namespace cvf
{
class ModelBasicList;
class Transform;
class Part;
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

class RimSurfaceInView;
class RigSurface;
class RigResultAccessor;

class RivSurfaceIntersectionGeometryGenerator;

class RivSurfacePartMgr : public cvf::Object
{
public:
    explicit RivSurfacePartMgr( RimSurfaceInView* surface );

    void updateNativeSurfaceColors();
    void updateCellResultColor( size_t timeStepIndex );
    void appendIntersectionGeometryPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );

    void appendNativeGeometryPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );

private:
    void generatePartGeometry();

    void generateNativePartGeometry();

    cvf::ref<RivSurfaceIntersectionGeometryGenerator> m_intersectionGenerator;

    caf::PdmPointer<RimSurfaceInView> m_surfaceInView;
    cvf::ref<RigSurface> m_usedSurfaceData; // Store the reference to the old data, to know when new data has arrived.

    cvf::ref<cvf::Part> m_nativeTrianglesPart;
    cvf::ref<cvf::Part> m_nativeMeshLinesPart;

    cvf::ref<cvf::Part> m_intersectionFaces;
    cvf::ref<cvf::Part> m_intersectionGridLines;
    cvf::ref<cvf::Part> m_intersectionFaultGridLines;

    cvf::ref<cvf::Vec2fArray> m_intersectionFacesTextureCoords;
    cvf::ref<cvf::Vec2fArray> m_nativeTrianglesTextureCoords;
};
