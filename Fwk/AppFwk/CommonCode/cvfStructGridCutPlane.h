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

#include "cvfPlane.h"
#include "cvfBoundingBox.h"

#include <vector>

namespace cvf {

class DrawableGeo;
class StructGridInterface;
class ScalarMapper;
class StructGridScalarDataAccess;


//==================================================================================================
//
// 
//
//==================================================================================================
class StructGridCutPlane : public Object
{
public:
    explicit StructGridCutPlane(const StructGridInterface* grid);
    ~StructGridCutPlane();

    void                setPlane(const Plane& plane);
    void                setClippingBoundingBox(const BoundingBox& boundingBox);
    void                setMapScalar(uint scalarSetIndex, const ScalarMapper* mapper, bool nodeAveragedScalars);

    ref<DrawableGeo>    generateSurface(const cvf::StructGridScalarDataAccess* dataAccessObject);
    ref<DrawableGeo>    generateMesh(const cvf::StructGridScalarDataAccess* dataAccessObject);

private:
    struct GridCell
    {
        Vec3d   p[8];       // Cell's corner coordinates
        double  s[8];       // Scalar values in cell corners
    };

    struct Triangles
    {
        Vec3d   vertices[12];       // The vertices, one on each edge in the cell
        double  scalars[12];        // Interpolated scalar values for the vertices
        bool    usedVertices[12];   // Flag to indicate which of the vertices (and scalars) are being referenced by the triangle indices
        int     triangleIndices[15];// Triangle indices (into vertices), max 5 triangles.
    };

private:
    void                computeCutPlane(const cvf::StructGridScalarDataAccess* dataAccessObject);
    void                addMeshLineIndices(const uint* triangleIndices, uint triangleCount);
    static uint         polygonise(const Plane& plane, const GridCell& cell, Triangles* triangles);
    static Vec3d        planeLineIntersection(const Plane& plane, const Vec3d& p1, const Vec3d& p2, const double s1, const double s2, double* s);
    static bool         isCellIntersectedByPlane(const Plane& plane, const Vec3d& cellMinCoord, const Vec3d& cellMaxCoord);

private:
    cref<StructGridInterface>   m_grid;
    
    Plane                   m_plane;
    BoundingBox             m_clippingBoundingBox;

    uint                    m_mapScalarSetIndex;        // Index of scalar set that should be mapped onto the cut plane. -1 for no mapping
    cref<ScalarMapper>      m_scalarMapper;             // Scalar mapper to use when mapping. Both scalar set index and mapper must be set in order to get scalar mapping
    bool                    m_mapNodeAveragedScalars;   // If true we'll compute node averaged scalars before mapping them on the cut plane. If false per cell scalars will be mapped. 

    bool                    m_mustRecompute;            // Flag to indicate that cut plane must be recomputed
    std::vector<Vec3f>      m_vertices;                 // Vertices of computed surface
    std::vector<double>     m_vertexScalars;            // Scalar values for vertices
    std::vector<uint>       m_triangleIndices;          // Triangle connectivities
    std::vector<uint>       m_meshLineIndices;          // Mesh line connectivities

    static const uint       sm_edgeTable[256];   
    static const int        sm_triTable[256][16];   
};

}
