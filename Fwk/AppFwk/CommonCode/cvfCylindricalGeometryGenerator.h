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

#include "cvfArray.h"
#include "cvfGeometryGeneratorInterface.h"

namespace cvf
{
class CellFaceVisibilityFilter;

//==================================================================================================
//
// Geometry generator for cylindrical/radial grids
//
//==================================================================================================
class CylindricalGeometryGenerator : public GeometryGeneratorInterface
{
public:
    explicit CylindricalGeometryGenerator( const StructGridInterface* grid, bool useOpenMP );

    // Setup methods
    void setCellVisibility( const UByteArray* cellVisibility );
    void addFaceVisibilityFilter( const CellFaceVisibilityFilter* cellVisibilityFilter );
    void setCurveSubdivisions( int subdivisions );

    // GeometryGeneratorInterface implementation
    ref<DrawableGeo> generateSurface() override;
    ref<DrawableGeo> createMeshDrawable() override;

    GridGeometryType geometryType() const override;

    void textureCoordinates( Vec2fArray*                       textureCoords,
                             const StructGridScalarDataAccess* resultAccessor,
                             const ScalarMapper*               mapper ) const override;

    const StructGridQuadToCellFaceMapper*    quadToCellFaceMapper() const override;
    const StuctGridTriangleToCellFaceMapper* triangleToCellFaceMapper() const override;

private:
    void computeArrays();
    void generateCylindricalQuads( const cvf::CylindricalCell& cell, size_t cellIndex, std::vector<Vec3f>& vertices );
    void addRadialFaces( const cvf::CylindricalCell& cell, size_t cellIndex, std::vector<Vec3f>& vertices );
    void addCircumferentialFaces( const cvf::CylindricalCell& cell, size_t cellIndex, std::vector<Vec3f>& vertices );
    void addTopBottomFaces( const cvf::CylindricalCell& cell, size_t cellIndex, std::vector<Vec3f>& vertices );

    bool isCellFaceVisible( size_t i, size_t j, size_t k, StructGridInterface::FaceType face ) const;

    ref<UIntArray> createCylindricalMeshLineIndices();

private:
    // Input
    std::vector<const CellFaceVisibilityFilter*> m_cellVisibilityFilters;
    cref<UByteArray>                             m_cellVisibility;
    int                                          m_curveSubdivisions;

    // Created arrays
    cvf::ref<cvf::Vec3fArray> m_vertices;
    cvf::ref<cvf::UIntArray>  m_meshLineIndices;

    // Mappings
    ref<StructGridQuadToCellFaceMapper>    m_quadMapper;
    ref<StuctGridTriangleToCellFaceMapper> m_triangleMapper;
};

} // namespace cvf
