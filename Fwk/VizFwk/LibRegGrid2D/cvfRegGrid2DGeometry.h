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

namespace cvf {

class RegGrid2D;
class DrawableGeo;


//==================================================================================================
//
// 
//
//==================================================================================================
class RegGrid2DGeometry : public Object
{
public:
    RegGrid2DGeometry(const RegGrid2D* regGrid2D);
    ~RegGrid2DGeometry();

    void                setTranslation(const Vec3d& translation);
    void                setElevationScaleFactor(double scaleFactor);

    ref<DrawableGeo>    generateSurface() const;

    ref<DrawableGeo>    generateClosedVolume(double minimumZ) const;

private:
    ref<DrawableGeo>    generateSideMinimumX(double minimumZ) const; // Does not compute normals
    ref<DrawableGeo>    generateSideMaximumX(double minimumZ) const; // Does not compute normals
    ref<DrawableGeo>    generateSideMinimumY(double minimumZ) const; // Does not compute normals
    ref<DrawableGeo>    generateSideMaximumY(double minimumZ) const; // Does not compute normals
    ref<DrawableGeo>    generateSideMinimumZ(double minimumZ) const; // Does not compute normals

    ref<Vec3fArray>     createSurfaceVertexArray() const;
    Vec3f               transformedGridPointCoordinate(uint i, uint j) const;
    Vec3f               transformedGridPointCoordinateFixedElevation(uint i, uint j, double elevation) const;

private:
    ref<RegGrid2D>  m_regGrid2D;
    Vec3d           m_translation;
    double          m_elevationScaleFactor;
};

}
