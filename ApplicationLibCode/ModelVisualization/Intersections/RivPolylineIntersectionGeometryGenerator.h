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

#include "cvfArray.h"

#include "cvfBoundingBox.h"
#include "cvfVector3.h"

#include <vector>

class RigMainGrid;
class RigActiveCellInfo;
class RigResultAccessor;
class RivIntersectionHexGridInterface;
class RimSurface;

namespace cvf
{
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

struct PolylineSegmentMeshData
{
    // U-axis defined by the unit length vector from start (x,y) to end (x,y), Z is global Z
    std::vector<float>     vertexArrayUZ; // U coordinate is length along U-axis. Array [u0,z0,u1,z1,...,ui,zi]
    std::vector<cvf::uint> polygonIndices; // Not needed when vertices/nodes are not shared between polygons?
    std::vector<cvf::uint> verticesPerPolygon;
    std::vector<cvf::uint> polygonToCellIndexMap;
    cvf::Vec2d             startUtmXY;
    cvf::Vec2d             endUtmXY;
};

class RivPolylineIntersectionGeometryGenerator
{
public:
    RivPolylineIntersectionGeometryGenerator( const std::vector<cvf::Vec2d>& polylineUtmXy, RivIntersectionHexGridInterface* grid );
    ~RivPolylineIntersectionGeometryGenerator();

    void generateIntersectionGeometry( cvf::UByteArray* visibleCells );
    bool isAnyGeometryPresent() const;

    const std::vector<PolylineSegmentMeshData>& polylineSegmentsMeshData() const;

private:
    void                       calculateArrays( cvf::UByteArray* visibleCells );
    static std::vector<size_t> createPolylineSegmentCellCandidates( const RivIntersectionHexGridInterface& hexGrid,
                                                                    const cvf::Vec3d&                      startPoint,
                                                                    const cvf::Vec3d&                      endPoint,
                                                                    const cvf::Vec3d&                      heightVector,
                                                                    const double                           topDepth,
                                                                    const double                           bottomDepth );

    static std::vector<cvf::Vec3d> initializePolylineUtmFromPolylineUtmXy( const std::vector<cvf::Vec2d>& polylineUtmXy );

private:
    RivIntersectionHexGridInterface* m_hexGrid = nullptr;
    const std::vector<cvf::Vec3d>    m_polylineUtm;

    // Output
    std::vector<PolylineSegmentMeshData> m_polylineSegmentsMeshData;
};
