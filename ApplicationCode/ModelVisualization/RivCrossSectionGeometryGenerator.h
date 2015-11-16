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
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfArray.h"

#include <vector>

class RigMainGrid;


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

    void textureCoordinates(cvf::Vec2fArray* textureCoords,  
        const cvf::ScalarMapper* mapper, size_t scalarResultIndex) const;

    // Mapping between cells and geometry
    cvf::ref<cvf::Array<size_t> >   triangleToCellIndex() const;

    // Generated geometry
    cvf::ref<cvf::DrawableGeo>    generateSurface();
    cvf::ref<cvf::DrawableGeo>    createMeshDrawable();
private:
    void calculateArrays();

    std::vector<cvf::Vec3d> m_triangleVxes;
    std::vector<size_t>     m_triangleToCellIdxMap;

};

