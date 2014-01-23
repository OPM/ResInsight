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
#include "cvfRegGrid2D.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RegGrid2D
/// \ingroup RegGrid2D
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RegGrid2D::RegGrid2D()
{
    m_gridPointCountI = 0;
    m_gridPointCountJ = 0;
    m_spacing.set(0, 0);
    m_offset.set(0, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::allocateGrid(int gridPointCountI, int gridPointCountJ)
{
    CVF_ASSERT(gridPointCountI >= 0);
    CVF_ASSERT(gridPointCountJ >= 0);

    m_gridPointCountI = gridPointCountI;
    m_gridPointCountJ = gridPointCountJ;

    m_elevations.resize(static_cast<size_t>(gridPointCount()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RegGrid2D::gridPointCountI() const
{
    return m_gridPointCountI;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RegGrid2D::gridPointCountJ() const
{
    return m_gridPointCountJ;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RegGrid2D::gridPointCount() const
{
    return m_gridPointCountI * m_gridPointCountJ;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2d RegGrid2D::spacing() const 
{
    return m_spacing;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::setSpacing(const Vec2d& spacing)
{
    CVF_ASSERT(spacing.x() >= 0);
    CVF_ASSERT(spacing.y() >= 0);

    m_spacing = spacing;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2d RegGrid2D::offset() const
{
    return m_offset;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::setOffset(const Vec2d& offset)
{
    m_offset = offset;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rectd RegGrid2D::boundingRectangle() const
{
    Vec2d min = gridPointCoordinate(0, 0);
    Vec2d max = gridPointCoordinate(m_gridPointCountI - 1, m_gridPointCountJ - 1);

    return Rectd::fromMinMax(min, max);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::setElevation(int i, int j, double elevation)
{
    int index = toArrayIndex(i, j);

    setElevation(index, elevation);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::setElevation(int arrayIndex, double elevation)
{
    CVF_ASSERT(arrayIndex >= 0 && arrayIndex < gridPointCount());

    m_elevations.set(static_cast<size_t>(arrayIndex), elevation);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RegGrid2D::elevation(int i, int j) const
{
    int index = toArrayIndex(i, j);
    CVF_ASSERT(index >=0 && index < gridPointCount());

    return m_elevations[static_cast<size_t>(index)];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::setAllElevations(double elevation)
{
    m_elevations.setAll(elevation);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::setElevations(const DoubleArray& elevations)
{
    setElevations(elevations.ptr(), static_cast<int>(elevations.size()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::setElevations(const double* elevations, int numElevationValues)
{
    CVF_ASSERT(elevations);
    CVF_ASSERT(numElevationValues > 0);
    CVF_ASSERT(numElevationValues == gridPointCount());

    m_elevations.assign(elevations, static_cast<size_t>(numElevationValues));
}


//--------------------------------------------------------------------------------------------------
/// Use bilinear interpolation to find value at specified coordinate
/// 
/// Undefined is returned along the edge of the grid (between grid points)
//--------------------------------------------------------------------------------------------------
double RegGrid2D::pointElevation(const Vec2d& coord) const
{
    Rectd region = boundingRectangle();
    if (!region.contains(coord)) return UNDEFINED_DOUBLE;

    int x1, y1;
    cellFromPoint(coord, &x1, &y1);

    Vec2d test = gridPointCoordinate(x1, y1);
    if (test == coord)
    {
        // Early exit if we hit exactly at a grid point
        return elevation(x1, y1);
    }

    // If we hit the outer edge of the reg grid, we are not able to compute the value using bilinear interpolation
    if (x1 >= m_gridPointCountI - 1) return UNDEFINED_DOUBLE;
    if (y1 >= m_gridPointCountJ - 1) return UNDEFINED_DOUBLE;

    CVF_ASSERT(x1 >= 0);
    CVF_ASSERT(y1 >= 0);
    CVF_ASSERT(x1 < m_gridPointCountI - 1);
    CVF_ASSERT(y1 < m_gridPointCountJ - 1);

    // Variable names taken from http://en.wikipedia.org/wiki/Bilinear_interpolation
    const Vec2i q11(x1,     y1);
    const Vec2i q21(x1 + 1, y1);
    const Vec2i q22(x1 + 1, y1 + 1);
    const Vec2i q12(x1,     y1 + 1);

    Vec2d q11Coord = gridPointCoordinate(q11.x(), q11.y());
    Vec2d q22Coord = gridPointCoordinate(q22.x(), q22.y());

    Vec2d localCoord = coord;// - m_offset;

    // Interpolate in x-direction
    double elevationR1 = ((q22Coord.x() - localCoord.x()) / m_spacing.x()) * elevation(q11.x(), q11.y()) + 
        ((localCoord.x() - q11Coord.x()) / m_spacing.x()) * elevation(q21.x(), q21.y());

    double elevationR2 = ((q22Coord.x() - localCoord.x()) / m_spacing.x()) * elevation(q12.x(), q12.y()) +
        ((localCoord.x() - q11Coord.x()) / m_spacing.x()) * elevation(q22.x(), q22.y());

    // Interpolate intermediate results in y-direction
    double elevationValue = ((q22Coord.y() - localCoord.y()) / m_spacing.y()) * elevationR1 +
        ((localCoord.y() - q11Coord.y()) / m_spacing.y()) * elevationR2;

    return static_cast<double>(elevationValue);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::pointElevations(const Array<Vec2d>& coords, DoubleArray* elevations) const
{
    CVF_ASSERT(elevations);

    size_t i;
    for (i = 0; i < coords.size(); i++)
    {
        elevations->add(pointElevation(coords[i]));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::mapPolylineOnGrid(const Array<Vec2d>& lineCoords, Vec3dArray* elevations) const
{
    CVF_ASSERT(lineCoords.size() >= 2);
    CVF_ASSERT(elevations);

    size_t i;
    for (i = 1; i < lineCoords.size(); i++)
    {
        mapLineSegmentOnGrid(lineCoords[i - 1], lineCoords[i], elevations);
    }
}


//--------------------------------------------------------------------------------------------------
/// Map a line segment onto the grid
///
/// Space is allocated for the incoming vector array, there are situations where number of allocated vectors
/// is larger than number of vectors used.
/// \internal See http://mathworld.wolfram.com/Line.html
//--------------------------------------------------------------------------------------------------
void RegGrid2D::mapLineSegmentOnGrid(const Vec2d& point1, const Vec2d& point2, Vec3dArray* intersections) const
{
    CVF_ASSERT(intersections);

    double delta = 0.00001;

    Rectd gridRegion = boundingRectangle();
    Vec2d intersect1, intersect2;
    if (!gridRegion.segmentIntersect(point1, point2, &intersect1, &intersect2)) return;

    Vec2d ab = intersect2 - intersect1;

    // Find out if ij is increasing or decreasing
    int changeI = ab.x() > 0.0 ? 1 : -1;
    int changeJ = ab.y() > 0.0 ? 1 : -1;

    int gridI, gridJ;
    cellFromPoint(intersect1, &gridI, &gridJ);
    if (changeI == 1) gridI++;
    if (changeJ == 1) gridJ++;


    int maxGridI, maxGridJ;
    cellFromPoint(intersect2, &maxGridI, &maxGridJ);
    if (changeI == 1) maxGridI++;
    if (changeJ == 1) maxGridJ++;

    // Reserve space in array once
    // Will reserve number of grid points in each direction, plus endpoints
    size_t numItemsToAllocate = intersections->size() + Math::abs(maxGridI - gridI) + Math::abs(maxGridJ - gridJ) + 2;
    intersections->reserve(numItemsToAllocate);

    // Add line segment start if it not a grid point
    if ((intersect1 - gridPointCoordinate(gridI, gridJ)).lengthSquared() > delta)
    {
        intersections->add(Vec3d(intersect1, pointElevation(intersect1)));
    }

    Vec2d currentIntersection;

    bool horizontalLine = Math::abs(ab.y()) < delta;
    if (horizontalLine)
    {
        while (gridI != maxGridI)
        {
            Vec2d nextGridPointCoord = gridPointCoordinate(gridI, gridJ);
            currentIntersection.set(nextGridPointCoord.x(), intersect1.y());

            intersections->add(Vec3d(currentIntersection, pointElevation(currentIntersection)));
            gridI += changeI;
        }
    }

    bool verticalLine = Math::abs(ab.x()) < delta;
    if (verticalLine)
    {
        while (gridJ != maxGridJ)
        {
            Vec2d nextGridPointCoord = gridPointCoordinate(gridI, gridJ);
            currentIntersection.set(intersect1.x(), nextGridPointCoord.y());

            intersections->add(Vec3d(currentIntersection, pointElevation(currentIntersection)));
            gridJ += changeJ;
        }
    }

    if (!(horizontalLine || verticalLine))
    {
        // y = (y2 - y1) / (x2 - x1) * (x - x1) + y1
        // y = m * (x - x1) + y1
        // y = mx + b
        double m = (intersect2.y() - intersect1.y()) / (intersect2.x() - intersect1.x());

        // b = y1 - (m * x1)
        double b = intersect1.y() - (m * intersect1.x());

        currentIntersection = intersect1;
        while ((gridI != maxGridI || gridJ != maxGridJ))
        {
            Vec2d nextGridPointCoord = gridPointCoordinate(gridI, gridJ);
            double nextYValue = m * nextGridPointCoord.x() + b;
            double nextXValue = (nextGridPointCoord.y() - b) / m;

            Vec2d horizontalIntersection(nextXValue, nextGridPointCoord.y());
            Vec2d verticalIntersection(nextGridPointCoord.x(), nextYValue);

            if ((horizontalIntersection - verticalIntersection).lengthSquared() < delta)
            {
                // Intersection at grid point, increase both i and j
                currentIntersection = horizontalIntersection;
                gridI += changeI;
                gridJ += changeJ;
            }
            else
            {
                // Find closest candidate
                if ((currentIntersection - horizontalIntersection).lengthSquared() < (currentIntersection - verticalIntersection).lengthSquared())
                {
                    currentIntersection = horizontalIntersection;
                    gridJ += changeJ;
                }
                else
                {
                    currentIntersection = verticalIntersection;
                    gridI += changeI;
                }
            }

            intersections->add(Vec3d(currentIntersection, pointElevation(currentIntersection)));
        }
    }

    // Add line segment end if this is not a grid point
    if ((currentIntersection - intersect2).lengthSquared() > delta)
    {
        intersections->add(Vec3d(intersect2, pointElevation(intersect2)));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RegGrid2D::minMaxElevation(double* minElevation, double* maxElevation) const
{
    if (gridPointCount() == 0) return false;

    double minValue = std::numeric_limits<double>::max();
    double maxValue = -std::numeric_limits<double>::max();

    size_t i;
    for (i = 0; i < m_elevations.size(); i++)
    {
        double elevationValue = m_elevations[i];
        if (elevationValue < minValue) minValue = elevationValue;
        if (elevationValue > maxValue) maxValue = elevationValue;
    }

    if (minElevation)
    {
        *minElevation = minValue;
    }

    if (maxElevation)
    {
        *maxElevation = maxValue;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RegGrid2D::minMaxElevationInRegion(int minI, int minJ, int maxI, int maxJ, double* minElevation, double* maxElevation) const
{
    CVF_ASSERT(minI >= 0 && minI <= maxI);
    CVF_ASSERT(minJ >= 0 && minJ <= maxJ);

    if (gridPointCount() == 0) return false;

    double minValue = std::numeric_limits<double>::max();
    double maxValue = -std::numeric_limits<double>::max();

    int ix, iy;
    for (ix = minI; ix <= maxI; ix++)
    {
        for (iy = minJ; iy <= maxJ; iy++)
        {
            double elevationValue = elevation(ix, iy);
            if (elevationValue < minValue) minValue = elevationValue;
            if (elevationValue > maxValue) maxValue = elevationValue;
        }
    }

    if (minElevation)
    {
        *minElevation = minValue;
    }

    if (maxElevation)
    {
        *maxElevation = maxValue;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2d RegGrid2D::gridPointCoordinate(int i, int j) const
{
    CVF_ASSERT(i >= 0 && i < m_gridPointCountI);
    CVF_ASSERT(j >= 0 && j < m_gridPointCountJ);

    Vec2d coord(m_offset);
    coord.x() += m_spacing.x() * i;
    coord.y() += m_spacing.y() * j;

    return coord;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RegGrid2D::toArrayIndex(int i, int j) const
{
    CVF_ASSERT(i >= 0 && i < m_gridPointCountI);
    CVF_ASSERT(j >= 0 && j < m_gridPointCountJ);

    int elevationIndex = m_gridPointCountI * j + i;
    CVF_ASSERT(elevationIndex < gridPointCount());

    return elevationIndex;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2D::cellFromPoint(const Vec2d& coord, int* i, int* j) const
{
    CVF_ASSERT(i && j);

    int gridI = static_cast<int>(Math::floor((coord.x() - offset().x()) / spacing().x()));
    int gridJ = static_cast<int>(Math::floor((coord.y() - offset().y()) / spacing().y()));

    CVF_ASSERT(gridI >= 0 && gridI < m_gridPointCountI);
    CVF_ASSERT(gridJ >= 0 && gridJ < m_gridPointCountJ);

    *i = gridI;
    *j = gridJ;
}


} // namespace cvf

