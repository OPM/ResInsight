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


#include "cvfBase.h"
#include "cvfRegGrid2DGeometry.h"
#include "cvfRegGrid2D.h"
#include "cvfGeometryUtils.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfGeometryBuilderDrawableGeo.h"

namespace cvf {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RegGrid2DGeometry::RegGrid2DGeometry(const RegGrid2D* regGrid2D)
{
    CVF_ASSERT(regGrid2D);
    m_regGrid2D = const_cast<RegGrid2D*>(regGrid2D);

    m_translation = Vec3d::ZERO;
    m_elevationScaleFactor = 1.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RegGrid2DGeometry::~RegGrid2DGeometry()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2DGeometry::setTranslation(const Vec3d& translation)
{
    m_translation = translation;        
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2DGeometry::setElevationScaleFactor(double scaleFactor)
{
    m_elevationScaleFactor = scaleFactor;
}

//--------------------------------------------------------------------------------------------------
///  It is allowed to create a surface when elevations are undefined. Should this be allowed?
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RegGrid2DGeometry::generateSurface() const
{
    CVF_ASSERT(m_regGrid2D->gridPointCountI() >= 0);
    CVF_ASSERT(m_regGrid2D->gridPointCountJ() >= 0);
    uint numPointsI = static_cast<uint>(m_regGrid2D->gridPointCountI());
    uint numPointsJ = static_cast<uint>(m_regGrid2D->gridPointCountJ());
    if (numPointsI < 2 || numPointsJ < 2)
    {
        CVF_FAIL_MSG("Must have at least two grid points in each direction");
        return NULL;
    }

     ref<UIntArray> indices = new UIntArray;
     GeometryUtils::tesselatePatchAsTriangles(numPointsI, numPointsJ, 0, true, indices.p());
 
     ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
     primSet->setIndices(indices.p());

    ref<Vec3fArray> vertices = createSurfaceVertexArray();

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(primSet.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RegGrid2DGeometry::generateClosedVolume(double minimumZ) const
{
    int numPointsI = m_regGrid2D->gridPointCountI();
    int numPointsJ = m_regGrid2D->gridPointCountJ();
    if (numPointsI < 2 || numPointsJ < 2)
    {
        return NULL;
    }

    double minElevation, maxElevation;
    if (!m_regGrid2D->minMaxElevation(&minElevation, &maxElevation))
    {
        return NULL;
    }

    // Not possible to create a valid closed volume if requested minimum Z is larger than minElevation
    if (minimumZ > minElevation)
    {
        return NULL;
    }

    ref<DrawableGeo> surface = generateSurface();
    CVF_ASSERT(surface.notNull());

    ref<DrawableGeo> sideMinimumX = generateSideMinimumX(minimumZ);
    CVF_ASSERT(sideMinimumX.notNull());

    ref<DrawableGeo> sideMaximumX = generateSideMaximumX(minimumZ);
    CVF_ASSERT(sideMaximumX.notNull());

    ref<DrawableGeo> sideMinimumY = generateSideMinimumY(minimumZ);
    CVF_ASSERT(sideMinimumY.notNull());

    ref<DrawableGeo> sideMaximumY = generateSideMaximumY(minimumZ);
    CVF_ASSERT(sideMaximumY.notNull());

    ref<DrawableGeo> sideMinimumZ = generateSideMinimumZ(minimumZ);
    CVF_ASSERT(sideMinimumZ.notNull());

    Collection<DrawableGeo> geoCollection;
    geoCollection.push_back(surface.p());
    geoCollection.push_back(sideMinimumX.p());
    geoCollection.push_back(sideMaximumX.p());
    geoCollection.push_back(sideMinimumY.p());
    geoCollection.push_back(sideMaximumY.p());
    geoCollection.push_back(sideMinimumZ.p());

    ref<DrawableGeo> closedVolume = new DrawableGeo;
    closedVolume->mergeInto(geoCollection);

    return closedVolume;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RegGrid2DGeometry::generateSideMinimumX(double minimumZ) const
{
    int numPointsY = m_regGrid2D->gridPointCountJ();

    ref<GeometryBuilderDrawableGeo> geoBuilder = new GeometryBuilderDrawableGeo;

    Vec3f a, b, c, d;
    for (int iy = 0; iy < numPointsY - 1; iy++)
    {
        {
            b = transformedGridPointCoordinate(0, static_cast<uint>(iy));
            a = transformedGridPointCoordinateFixedElevation(0, static_cast<uint>(iy), minimumZ);
        }

        {
            c = transformedGridPointCoordinate(0, static_cast<uint>(iy + 1));
            d = transformedGridPointCoordinateFixedElevation(0, static_cast<uint>(iy + 1), minimumZ);
        }

        geoBuilder->addTriangleByVertices(a, b, c);
        geoBuilder->addTriangleByVertices(a, c, d);
    }

    return geoBuilder->drawableGeo();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RegGrid2DGeometry::generateSideMaximumX(double minimumZ) const
{
    int numPointsX = m_regGrid2D->gridPointCountI();
    int numPointsY = m_regGrid2D->gridPointCountJ();

    ref<GeometryBuilderDrawableGeo> geoBuilder = new GeometryBuilderDrawableGeo;

    Vec3f a, b, c, d;
    for (int iy = 0; iy < numPointsY - 1; iy++)
    {
        {
            b = transformedGridPointCoordinate(                 static_cast<uint>(numPointsX - 1), static_cast<uint>(iy));
            a = transformedGridPointCoordinateFixedElevation(   static_cast<uint>(numPointsX - 1), static_cast<uint>(iy), minimumZ);
        }

        {
            c = transformedGridPointCoordinate(                 static_cast<uint>(numPointsX - 1), static_cast<uint>(iy));
            d = transformedGridPointCoordinateFixedElevation(   static_cast<uint>(numPointsX - 1), static_cast<uint>(iy), minimumZ);
        }

        geoBuilder->addTriangleByVertices(b, a, c);
        geoBuilder->addTriangleByVertices(a, d, c);
    }

    return geoBuilder->drawableGeo();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RegGrid2DGeometry::generateSideMinimumY(double minimumZ) const
{
    int numPointsX = m_regGrid2D->gridPointCountI();

    ref<GeometryBuilderDrawableGeo> geoBuilder = new GeometryBuilderDrawableGeo;

    Vec3f a, b, c, d;
    for (int ix = 0; ix < numPointsX - 1; ix++)
    {
        {
            b = transformedGridPointCoordinate(                 static_cast<uint>(ix), 0);
            a = transformedGridPointCoordinateFixedElevation(   static_cast<uint>(ix), 0, minimumZ);
        }

        {
            c = transformedGridPointCoordinate(                 static_cast<uint>(ix + 1), 0);
            d = transformedGridPointCoordinateFixedElevation(   static_cast<uint>(ix + 1), 0, minimumZ);
        }

        geoBuilder->addTriangleByVertices(b, a, c);
        geoBuilder->addTriangleByVertices(a, d, c);
    }

    return geoBuilder->drawableGeo();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RegGrid2DGeometry::generateSideMaximumY(double minimumZ) const
{
    int numPointsX = m_regGrid2D->gridPointCountI();
    int numPointsY = m_regGrid2D->gridPointCountJ();

    ref<GeometryBuilderDrawableGeo> geoBuilder = new GeometryBuilderDrawableGeo;

    Vec3f a, b, c, d;
    for (int ix = 0; ix < numPointsX - 1; ix++)
    {
        {
            b = transformedGridPointCoordinate(                 static_cast<uint>(ix), static_cast<uint>(numPointsY - 1));
            a = transformedGridPointCoordinateFixedElevation(   static_cast<uint>(ix), static_cast<uint>(numPointsY - 1), minimumZ);
        }

        {
            c = transformedGridPointCoordinate(                 static_cast<uint>(ix + 1), static_cast<uint>(numPointsY - 1));
            d = transformedGridPointCoordinateFixedElevation(   static_cast<uint>(ix + 1), static_cast<uint>(numPointsY - 1), minimumZ);
        }

        geoBuilder->addTriangleByVertices(a, b, c);
        geoBuilder->addTriangleByVertices(d, a, c);
    }

    return geoBuilder->drawableGeo();
}

//--------------------------------------------------------------------------------------------------
/// NOTE: To avoid bad edges in exported STL geometry, the bottom geometry is fully tessellated.
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RegGrid2DGeometry::generateSideMinimumZ(double minimumZ) const
{
    CVF_ASSERT(m_regGrid2D->gridPointCountI() >= 0);
    CVF_ASSERT(m_regGrid2D->gridPointCountJ() >= 0);
    uint numPointsI = static_cast<uint>(m_regGrid2D->gridPointCountI());
    uint numPointsJ = static_cast<uint>(m_regGrid2D->gridPointCountJ());
    if (numPointsI < 2 || numPointsJ < 2)
    {
        CVF_FAIL_MSG("Must have at least two grid points in each direction");
        return NULL;
    }

    ref<UIntArray> indices = new UIntArray;

    // Use opposite winding type compared to generateSurface() to make the surface normal point out of the closed volume
    GeometryUtils::tesselatePatchAsTriangles(numPointsI, numPointsJ, 0, false, indices.p());

    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    primSet->setIndices(indices.p());

    uint numPoints = numPointsI * numPointsJ;
    ref<Vec3fArray> vertices = new Vec3fArray;
    vertices->reserve(numPoints);

    uint j;
    for (j = 0; j < numPointsJ; j++)
    {
        uint i;
        for (i = 0; i < numPointsI; i++)
        {
            Vec3f coord = transformedGridPointCoordinateFixedElevation(i, j, minimumZ);
            vertices->add(coord);
        }
    }

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(primSet.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> RegGrid2DGeometry::createSurfaceVertexArray() const
{
    CVF_ASSERT(m_regGrid2D->gridPointCountI() >= 0);
    CVF_ASSERT(m_regGrid2D->gridPointCountJ() >= 0);
    uint gridPointCountI = static_cast<uint>(m_regGrid2D->gridPointCountI());
    uint gridPointCountJ = static_cast<uint>(m_regGrid2D->gridPointCountJ());

    uint numPoints = gridPointCountI * gridPointCountJ;

    ref<Vec3fArray> vertices = new Vec3fArray;
    vertices->reserve(numPoints);

    uint j;
    for (j = 0; j < gridPointCountJ; j++)
    {
        uint i;
        for (i = 0; i < gridPointCountI; i++)
        {
            Vec3f coord = transformedGridPointCoordinate(i, j);
            vertices->add(coord);
        }
    }

    return vertices;

}

//--------------------------------------------------------------------------------------------------
/// Scale elevation using elevation scale factor and add translation
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RegGrid2DGeometry::transformedGridPointCoordinate(uint i, uint j) const
{
    double elevation = m_regGrid2D->elevation(static_cast<int>(i), static_cast<int>(j));
    elevation *= m_elevationScaleFactor;

    Vec2d coord2D = m_regGrid2D->gridPointCoordinate(static_cast<int>(i), static_cast<int>(j));
    Vec3d gridPointCoord(coord2D, elevation);

    gridPointCoord += m_translation;

    return Vec3f(gridPointCoord);
}

//--------------------------------------------------------------------------------------------------
/// Create 3D point from XY coords based on grid point IJ and Z based on given elevation. Add 3D translation to this vector.
/// Note: Elevation scaling is not applied
//--------------------------------------------------------------------------------------------------
Vec3f RegGrid2DGeometry::transformedGridPointCoordinateFixedElevation(uint i, uint j, double elevation) const
{
    Vec2d coord2D = m_regGrid2D->gridPointCoordinate(static_cast<int>(i), static_cast<int>(j));
    Vec3d gridPointCoord(coord2D, elevation);

    gridPointCoord += m_translation;

    return Vec3f(gridPointCoord);
}


} // namespace cvf

