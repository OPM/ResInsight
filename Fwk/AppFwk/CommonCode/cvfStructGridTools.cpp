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

#include "cvfStructGridTools.h"

#include "cvfAssert.h"

#include <cmath>
#include <numbers>

using namespace cvf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<UIntArray> StructGridTools::lineIndicesFromQuadVertexArray( const Vec3fArray* vertexArray )
{
    CVF_ASSERT( vertexArray );

    size_t numVertices = vertexArray->size();
    int    numQuads    = static_cast<int>( numVertices / 4 );
    CVF_ASSERT( numVertices % 4 == 0 );

    ref<UIntArray> indices = new UIntArray;
    indices->resize( (size_t)numQuads * 8 );

#pragma omp parallel for
    for ( int i = 0; i < numQuads; i++ )
    {
        size_t idx = (size_t)i * 8;
        indices->set( idx + 0, i * 4 + 0 );
        indices->set( idx + 1, i * 4 + 1 );
        indices->set( idx + 2, i * 4 + 1 );
        indices->set( idx + 3, i * 4 + 2 );
        indices->set( idx + 4, i * 4 + 2 );
        indices->set( idx + 5, i * 4 + 3 );
        indices->set( idx + 6, i * 4 + 3 );
        indices->set( idx + 7, i * 4 + 0 );
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Vec3d StructGridTools::cylindricalToCartesian( double radius, double angleDegrees, double z )
{
    // Convert angle from degrees to radians for trigonometric functions
    double angleRadians = angleDegrees * std::numbers::pi / 180.0;
    double x            = radius * std::cos( angleRadians );
    double y            = radius * std::sin( angleRadians );
    return Vec3d( x, y, z );
}