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
#include "cvfStructGrid.h"

namespace cvf
{
class DrawableGeo;
class StructGridInterface;

//==================================================================================================
//
// Factory for creating mesh drawables from single cells
//
//==================================================================================================
class SingleCellMeshFactory
{
public:
    static ref<DrawableGeo> createMeshDrawable( const StructGridInterface* grid, size_t cellIndex );
    static ref<DrawableGeo>
        createMeshDrawable( const StructGridInterface* grid, size_t cellIndex, const cvf::Vec3d& displayModelOffset );

private:
    static ref<DrawableGeo>
        createHexahedralMesh( const StructGridInterface* grid, size_t cellIndex, const cvf::Vec3d& displayModelOffset );

    static ref<DrawableGeo>
        createCylindricalMesh( const StructGridInterface* grid, size_t cellIndex, const cvf::Vec3d& displayModelOffset );
};

} // namespace cvf