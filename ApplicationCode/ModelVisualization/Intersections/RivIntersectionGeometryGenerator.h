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

#include "cafPdmPointer.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RigMainGrid;
class RigActiveCellInfo;
class RigResultAccessor;
class RimIntersection;
class RivIntersectionHexGridInterface;
class RivIntersectionVertexWeights;

namespace cvf
{
    class ScalarMapper;
    class DrawableGeo;
}


class RivIntersectionGeometryGenerator : public cvf::Object
{
public:
    RivIntersectionGeometryGenerator(RimIntersection* crossSection,
                                     std::vector<std::vector<cvf::Vec3d> > &polylines, 
                                     const cvf::Vec3d& extrusionDirection, 
                                     const RivIntersectionHexGridInterface* grid,
                                     bool isFlattened,
                                     const cvf::Vec3d& flattenedPolylineStartPoint);

    ~RivIntersectionGeometryGenerator();

    bool                        isAnyGeometryPresent() const;
 
    // Generate geometry
    cvf::ref<cvf::DrawableGeo>  generateSurface();
    cvf::ref<cvf::DrawableGeo>  createMeshDrawable();
    cvf::ref<cvf::DrawableGeo>  createFaultMeshDrawable();

    cvf::ref<cvf::DrawableGeo>  createLineAlongPolylineDrawable();
    cvf::ref<cvf::DrawableGeo>  createLineAlongExtrusionLineDrawable(const std::vector<cvf::Vec3d>& extrusionLine);
    cvf::ref<cvf::DrawableGeo>  createPointsFromPolylineDrawable();
    cvf::ref<cvf::DrawableGeo>  createPointsFromExtrusionLineDrawable(const std::vector<cvf::Vec3d>& extrusionLine);

    const std::vector<std::vector<cvf::Vec3d> >&     flattenedOrOffsettedPolyLines() { return m_flattenedOrOffsettedPolyLines; }

    // Mapping between cells and geometry
    const std::vector<size_t>&                       triangleToCellIndex() const;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const;
    const cvf::Vec3fArray*                           triangleVxes() const;

    RimIntersection*            crossSection() const;

    cvf::Mat4d                  unflattenTransformMatrix(const cvf::Vec3d& intersectionPointFlat);

private:
    cvf::ref<cvf::DrawableGeo>  createLineAlongPolylineDrawable(const std::vector<std::vector<cvf::Vec3d> >& polyLines);
    cvf::ref<cvf::DrawableGeo>  createPointsFromPolylineDrawable(const std::vector<std::vector<cvf::Vec3d> >& polyLines);

    void                        calculateArrays();
    void                        calculateSegementTransformPrLinePoint();
    void                        calculateFlattenedOrOffsetedPolyline();

    //static size_t               indexToNextValidPoint(const std::vector<cvf::Vec3d>& polyLine,
    //                                                  const cvf::Vec3d extrDir,
    //                                                  size_t idxToStartOfLineSegment);

    RimIntersection*                            m_crossSection;
    cvf::cref<RivIntersectionHexGridInterface>  m_hexGrid;
    const std::vector<std::vector<cvf::Vec3d> > m_polyLines;
    cvf::Vec3d                                  m_extrusionDirection;
    bool                                        m_isFlattened;
    cvf::Vec3d                                  m_flattenedPolylineStartPoint;
                                                
    // Output arrays                            
    cvf::ref<cvf::Vec3fArray>                   m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>                   m_cellBorderLineVxes;
    cvf::ref<cvf::Vec3fArray>                   m_faultCellBorderLineVxes;
    std::vector<size_t>                         m_triangleToCellIdxMap;
    std::vector<RivIntersectionVertexWeights>   m_triVxToCellCornerWeights;
    std::vector<std::vector<cvf::Vec3d> >       m_flattenedOrOffsettedPolyLines;
    std::vector<std::vector<cvf::Mat4d> >       m_segementTransformPrLinePoint;

};

