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

class RivSurfacePartMgr : public cvf::Object
{
public:
    explicit RivSurfacePartMgr( RimSurfaceInView* surface );

    void appendNativeGeometryPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void updateCellResultColor( size_t timeStepIndex );
    void applySingleColor();

private:
    void generateNativePartGeometry();
    void generateVertexToCellIndexMap();

    static void calculateVertexTextureCoordinates( cvf::Vec2fArray*           textureCoords,
                                                   const std::vector<size_t>& vertexToCellIdxMap,
                                                   const RigResultAccessor*   resultAccessor,
                                                   const cvf::ScalarMapper*   mapper );

    caf::PdmPointer<RimSurfaceInView> m_surfaceInView;
    cvf::ref<RigSurface> m_usedSurfaceData; // Store the reference to the old data, to know when new data has arrived.

    cvf::ref<cvf::Part> m_nativeTrianglesPart;
    cvf::ref<cvf::Part> m_nativeMeshLinesPart;

    std::vector<size_t> m_vertexToCellIndexMap;
};
