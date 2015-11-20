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
#include "cvfVector3.h"
#include "cvfArray.h"

#include <vector>

class RigMainGrid;
class RigResultAccessor;

namespace cvf
{
    class ScalarMapper;
    class DrawableGeo;
}


class RivCrossSectionGeometryGenerator : public cvf::Object
{
public:
    RivCrossSectionGeometryGenerator(const std::vector<cvf::Vec3d> &polyline, 
                                     const cvf::Vec3d& extrusionDirection, 
                                     const RigMainGrid* grid );

    ~RivCrossSectionGeometryGenerator();


    void                        textureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                   const RigResultAccessor* resultAccessor,
                                                   const cvf::ScalarMapper* mapper) const;

    // Mapping between cells and geometry
    const std::vector<size_t>&  triangleToCellIndex() const;

    // Generated geometry
    cvf::ref<cvf::DrawableGeo>  generateSurface();
    cvf::ref<cvf::DrawableGeo>  createMeshDrawable();

private:
    void                        calculateArrays();



    void                        adjustPolyline();

    cvf::cref<RigMainGrid>      m_mainGrid;
    std::vector<cvf::Vec3d>     m_polyLine;
    cvf::Vec3d                  m_extrusionDirection;
    std::vector<cvf::Vec3d>     m_adjustedPolyline;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>   m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>   m_cellBorderLineVxes;
    std::vector<size_t>         m_triangleToCellIdxMap;

    struct VxInterPolData
    {
        explicit VxInterPolData(int edge1Vx1, int edge1Vx2, double normDistFromE1V1, 
                                int edge2Vx1, int edge2Vx2, double normDistFromE2V1,
                                double normDistFromE1Cut)
                       : vx1Id(edge1Vx1), 
                       weight1((float)(1.0 - normDistFromE1V1 - normDistFromE1Cut + normDistFromE1V1*normDistFromE1Cut)),
                       vx2Id(edge1Vx2),
                       weight2((float)(normDistFromE1V1 - normDistFromE1V1*normDistFromE1Cut)),
                       vx3Id(edge2Vx1),
                       weight3((float)(normDistFromE1Cut - normDistFromE2V1*normDistFromE1Cut)),
                       vx4Id(edge2Vx2),
                       weight4((float)(normDistFromE2V1*normDistFromE1Cut))
        {}

        int vx1Id;
        float weight1;
        int vx2Id;
        float weight2;
    
        int vx3Id;
        float weight3;
        int vx4Id;
        float weight4;
    };

    std::vector<VxInterPolData> m_triangleVxInterPolationData;
};

