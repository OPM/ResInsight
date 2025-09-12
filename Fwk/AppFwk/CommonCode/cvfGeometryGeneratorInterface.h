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
#include "cvfObject.h"
#include "cvfStructGrid.h"

namespace cvf
{
class DrawableGeo;
class StructGridScalarDataAccess;
class ScalarMapper;
class StructGridQuadToCellFaceMapper;
class StuctGridTriangleToCellFaceMapper;
class CellFaceVisibilityFilter;

//==================================================================================================
//
// Base interface for grid geometry generators
//
//==================================================================================================
class GeometryGeneratorInterface
{
public:
    explicit GeometryGeneratorInterface( const StructGridInterface* grid, bool useOpenMP );

    // Setup methods
    virtual void setCellVisibility( const UByteArray* cellVisibility )                           = 0;
    virtual void addFaceVisibilityFilter( const CellFaceVisibilityFilter* cellVisibilityFilter ) = 0;

    // Core geometry generation
    virtual ref<DrawableGeo> generateSurface()    = 0;
    virtual ref<DrawableGeo> createMeshDrawable() = 0;

    // Geometry type identification
    virtual GridGeometryType geometryType() const = 0;

    // Texture coordinate support
    virtual void textureCoordinates( Vec2fArray*                       textureCoords,
                                     const StructGridScalarDataAccess* resultAccessor,
                                     const ScalarMapper*               mapper ) const = 0;

    // Mapping support (may not be applicable to all geometry types)
    virtual const StructGridQuadToCellFaceMapper*    quadToCellFaceMapper() const     = 0;
    virtual const StuctGridTriangleToCellFaceMapper* triangleToCellFaceMapper() const = 0;

protected:
    cref<StructGridInterface> m_grid;
    bool                      m_useOpenMP;
};

} // namespace cvf
