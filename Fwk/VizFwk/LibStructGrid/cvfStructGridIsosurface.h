//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfObject.h"
#include "cvfVector3.h"

namespace cvf {

class RectilinearGrid;
class DrawableGeo;
class ScalarMapper;


//==================================================================================================
//
// 
//
//==================================================================================================
class StructGridIsosurface : public Object
{
public:
    StructGridIsosurface(const RectilinearGrid* grid); 
    ~StructGridIsosurface();

    void                setScalarSetIndex(uint scalarSetIndex);
    void                setIsoValue(double value);
    void                setMapScalar(uint scalarSetIndex, const ScalarMapper* mapper);

    ref<DrawableGeo>    generateSurface() const;
    ref<DrawableGeo>    generateSurfaceCellCenterBased() const;

private:
    struct GridCell
    {
        Vec3d   p[8];       // Cell's corner coordinates
        double  val[8];     // Scalar value in cell corners used to find isosurface
        double  mapVal[8];  // Scalar value in cell corners used to map onto isosurface
        size_t  eid[12];    // Global edge IDs for each of the cell's edges
    };

    struct Triangle
    {
        Vec3d   p[3];       // Triangle vertices
        size_t  peid[3];    // Source edge ID for the vertices 
        double  scalars[3]; // Interpolated scalar values for the vertices
    };

    bool            mapScalar() const;

    static int      polygonise(double isoLevel, const GridCell& grid, Triangle triangles[5], bool useTextureMapping);
    static Vec3d    vertexInterpolate(double isoLevel, const Vec3d& p1, const Vec3d& p2, const double valp1, const double valp2);
    static Vec3d    vertexInterpolateWithMapping(double isoLevel, const Vec3d& p1, const Vec3d& p2, const double valp1, const double valp2, const double mapValp1, const double mapValp2, double* mapVal);

private:
    cref<RectilinearGrid>   m_grid;
    uint                    m_scalarSetIndex;       // Index of the value set that is used to generate the isosurface. Default 0
    double                  m_isoValue;             // Scalar value used to find isosurface for

    uint                    m_mapScalarSetIndex;    // Index of scalar set that is mapped onto the cut plane. -1 for no mapping
    cref<ScalarMapper>      m_scalarMapper;         // Scalar mapper to use when mapping. Both scalar set index and mapper must be set in order to get scalar mapping

    static const uint       sm_edgeTable[256];   
    static const int        sm_triTable[256][16];   
};

}
