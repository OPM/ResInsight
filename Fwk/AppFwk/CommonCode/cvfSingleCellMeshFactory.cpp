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

#include "cvfSingleCellMeshFactory.h"

#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfStructGridTools.h"

using namespace cvf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> SingleCellMeshFactory::createMeshDrawable( const StructGridInterface* grid, size_t cellIndex )
{
    return createMeshDrawable( grid, cellIndex, grid->displayModelOffset() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> SingleCellMeshFactory::createMeshDrawable( const StructGridInterface* grid,
                                                            size_t                     cellIndex,
                                                            const cvf::Vec3d&          displayModelOffset )
{
    if ( grid->gridGeometryType() == cvf::GridGeometryType::CYLINDRICAL )
    {
        return createCylindricalMesh( grid, cellIndex, displayModelOffset );
    }
    else
    {
        return createHexahedralMesh( grid, cellIndex, displayModelOffset );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> SingleCellMeshFactory::createHexahedralMesh( const StructGridInterface* grid,
                                                              size_t                     cellIndex,
                                                              const cvf::Vec3d&          displayModelOffset )
{
    std::array<cvf::Vec3d, 8> cornerVerts = grid->cellCornerVertices( cellIndex );

    std::vector<Vec3f> vertices;

    for ( int enumInt = cvf::StructGridInterface::POS_I; enumInt < cvf::StructGridInterface::NO_FACE; enumInt++ )
    {
        cvf::StructGridInterface::FaceType face = static_cast<cvf::StructGridInterface::FaceType>( enumInt );

        ubyte faceConn[4];
        grid->cellFaceVertexIndices( face, faceConn );

        for ( int n = 0; n < 4; n++ )
        {
            vertices.push_back( cvf::Vec3f( cornerVerts[faceConn[n]] - displayModelOffset ) );
        }
    }

    cvf::ref<cvf::Vec3fArray> cvfVertices = new cvf::Vec3fArray;
    cvfVertices->assign( vertices );

    if ( !( cvfVertices.notNull() && cvfVertices->size() != 0 ) ) return nullptr;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( cvfVertices.p() );

    ref<UIntArray>               indices = StructGridTools::lineIndicesFromQuadVertexArray( cvfVertices.p() );
    ref<PrimitiveSetIndexedUInt> prim    = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> SingleCellMeshFactory::createCylindricalMesh( const StructGridInterface* grid,
                                                               size_t                     cellIndex,
                                                               const cvf::Vec3d&          displayModelOffset )
{
    auto result = grid->getCylindricalCoords( cellIndex );
    if ( !result.has_value() )
    {
        return createHexahedralMesh( grid, cellIndex, displayModelOffset );
    }

    const cvf::CylindricalCell cylCell = result.value();

    std::vector<Vec3f> vertices;

    cvf::Vec3d offset            = displayModelOffset;
    int        curveSubdivisions = 10;

    double angleRange = cylCell.endAngle - cylCell.startAngle;
    double angleStep  = angleRange / curveSubdivisions;

    // Inner radial curve - bottom edge
    for ( int i = 0; i < curveSubdivisions; ++i )
    {
        double angle1 = cylCell.startAngle + i * angleStep;
        double angle2 = cylCell.startAngle + ( i + 1 ) * angleStep;

        cvf::Vec3d innerBot1 = StructGridTools::cylindricalToCartesian( cylCell.innerRadius, angle1, cylCell.bottomZ );
        cvf::Vec3d innerBot2 = StructGridTools::cylindricalToCartesian( cylCell.innerRadius, angle2, cylCell.bottomZ );

        vertices.push_back( cvf::Vec3f( innerBot1 - offset ) );
        vertices.push_back( cvf::Vec3f( innerBot2 - offset ) );
    }

    // Inner radial curve - top edge
    for ( int i = 0; i < curveSubdivisions; ++i )
    {
        double angle1 = cylCell.startAngle + i * angleStep;
        double angle2 = cylCell.startAngle + ( i + 1 ) * angleStep;

        cvf::Vec3d innerTop1 = StructGridTools::cylindricalToCartesian( cylCell.innerRadius, angle1, cylCell.topZ );
        cvf::Vec3d innerTop2 = StructGridTools::cylindricalToCartesian( cylCell.innerRadius, angle2, cylCell.topZ );

        vertices.push_back( cvf::Vec3f( innerTop1 - offset ) );
        vertices.push_back( cvf::Vec3f( innerTop2 - offset ) );
    }

    // Outer radial curve - bottom edge
    for ( int i = 0; i < curveSubdivisions; ++i )
    {
        double angle1 = cylCell.startAngle + i * angleStep;
        double angle2 = cylCell.startAngle + ( i + 1 ) * angleStep;

        cvf::Vec3d outerBot1 = StructGridTools::cylindricalToCartesian( cylCell.outerRadius, angle1, cylCell.bottomZ );
        cvf::Vec3d outerBot2 = StructGridTools::cylindricalToCartesian( cylCell.outerRadius, angle2, cylCell.bottomZ );

        vertices.push_back( cvf::Vec3f( outerBot1 - offset ) );
        vertices.push_back( cvf::Vec3f( outerBot2 - offset ) );
    }

    // Outer radial curve - top edge
    for ( int i = 0; i < curveSubdivisions; ++i )
    {
        double angle1 = cylCell.startAngle + i * angleStep;
        double angle2 = cylCell.startAngle + ( i + 1 ) * angleStep;

        cvf::Vec3d outerTop1 = StructGridTools::cylindricalToCartesian( cylCell.outerRadius, angle1, cylCell.topZ );
        cvf::Vec3d outerTop2 = StructGridTools::cylindricalToCartesian( cylCell.outerRadius, angle2, cylCell.topZ );

        vertices.push_back( cvf::Vec3f( outerTop1 - offset ) );
        vertices.push_back( cvf::Vec3f( outerTop2 - offset ) );
    }

    // Add radial lines from inner to outer radius at start and end angles
    cvf::Vec3d startInnerBot =
        StructGridTools::cylindricalToCartesian( cylCell.innerRadius, cylCell.startAngle, cylCell.bottomZ );
    cvf::Vec3d startOuterBot =
        StructGridTools::cylindricalToCartesian( cylCell.outerRadius, cylCell.startAngle, cylCell.bottomZ );
    cvf::Vec3d startInnerTop =
        StructGridTools::cylindricalToCartesian( cylCell.innerRadius, cylCell.startAngle, cylCell.topZ );
    cvf::Vec3d startOuterTop =
        StructGridTools::cylindricalToCartesian( cylCell.outerRadius, cylCell.startAngle, cylCell.topZ );

    cvf::Vec3d endInnerBot =
        StructGridTools::cylindricalToCartesian( cylCell.innerRadius, cylCell.endAngle, cylCell.bottomZ );
    cvf::Vec3d endOuterBot =
        StructGridTools::cylindricalToCartesian( cylCell.outerRadius, cylCell.endAngle, cylCell.bottomZ );
    cvf::Vec3d endInnerTop = StructGridTools::cylindricalToCartesian( cylCell.innerRadius, cylCell.endAngle, cylCell.topZ );
    cvf::Vec3d endOuterTop = StructGridTools::cylindricalToCartesian( cylCell.outerRadius, cylCell.endAngle, cylCell.topZ );

    // Start angle radial lines
    vertices.push_back( cvf::Vec3f( startInnerBot - offset ) );
    vertices.push_back( cvf::Vec3f( startOuterBot - offset ) );
    vertices.push_back( cvf::Vec3f( startOuterTop - offset ) );
    vertices.push_back( cvf::Vec3f( startInnerTop - offset ) );

    // End angle radial lines
    vertices.push_back( cvf::Vec3f( endInnerBot - offset ) );
    vertices.push_back( cvf::Vec3f( endOuterBot - offset ) );
    vertices.push_back( cvf::Vec3f( endOuterTop - offset ) );
    vertices.push_back( cvf::Vec3f( endInnerTop - offset ) );

    // Add vertical corner lines connecting top and bottom
    vertices.push_back( cvf::Vec3f( startInnerBot - offset ) );
    vertices.push_back( cvf::Vec3f( startInnerTop - offset ) );

    vertices.push_back( cvf::Vec3f( startOuterBot - offset ) );
    vertices.push_back( cvf::Vec3f( startOuterTop - offset ) );

    vertices.push_back( cvf::Vec3f( endInnerBot - offset ) );
    vertices.push_back( cvf::Vec3f( endInnerTop - offset ) );

    vertices.push_back( cvf::Vec3f( endOuterBot - offset ) );
    vertices.push_back( cvf::Vec3f( endOuterTop - offset ) );

    if ( vertices.empty() )
    {
        return nullptr;
    }

    cvf::ref<cvf::Vec3fArray> cvfVertices = new cvf::Vec3fArray;
    cvfVertices->assign( vertices );

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( cvfVertices.p() );

    ref<UIntArray> lineIndices = new UIntArray;
    lineIndices->resize( vertices.size() );
    for ( size_t i = 0; i < vertices.size(); ++i )
    {
        lineIndices->set( i, static_cast<uint>( i ) );
    }

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( lineIndices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}
