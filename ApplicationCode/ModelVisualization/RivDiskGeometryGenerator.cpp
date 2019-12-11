/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RivDiskGeometryGenerator.h"

#include "cvfBase.h"
#include "cvfGeometryBuilder.h"
#include "cvfGeometryUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivDiskGeometryGenerator::RivDiskGeometryGenerator()
    : m_relativeRadius( 0.085f )
    , m_relativeLength( 0.25f )
    , m_numSlices( 20 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::setRelativeRadius( float relativeRadius )
{
    m_relativeRadius = relativeRadius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::setRelativeLength( float relativeLength )
{
    m_relativeLength = relativeLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::setNumSlices( unsigned int numSlices )
{
    m_numSlices = numSlices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::generate( cvf::GeometryBuilder* builder )
{
    const unsigned int numPolysZDir = 1;

    cvf::GeometryUtils::createObliqueCylinder( m_relativeRadius,
                                               m_relativeRadius,
                                               1.0f - m_relativeLength,
                                               0,
                                               0,
                                               m_numSlices,
                                               true,
                                               true,
                                               false,
                                               numPolysZDir,
                                               builder );

    unsigned int startIdx = builder->vertexCount();
    cvf::GeometryUtils::createCone( m_relativeRadius, m_relativeLength, m_numSlices, true, true, true, builder );
    unsigned int endIdx = builder->vertexCount() - 1;

    cvf::Mat4f mat = cvf::Mat4f::fromTranslation( cvf::Vec3f( 0, 0, 1.0f - m_relativeLength ) );
    builder->transformVertexRange( startIdx, endIdx, mat );
}
