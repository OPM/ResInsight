/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "cvfDrawableVectors.h"

class RivDrawableSpheres : public cvf::DrawableVectors
{
public:
    RivDrawableSpheres();
    RivDrawableSpheres( cvf::String vectorMatrixUniformName, cvf::String colorUniformName );

    bool rayIntersectCreateDetail( const cvf::Ray&           ray,
                                   cvf::Vec3d*               intersectionPoint,
                                   cvf::ref<cvf::HitDetail>* hitDetail ) const override;

    void setRadius( float radius );
    void setCenterCoords( cvf::Vec3fArray* vertexArray );

private:
    cvf::ref<cvf::Vec3fArray> m_centerCoordArray; // Coordinates for sphere center
    float                     m_radius; // Sphere radius
};
