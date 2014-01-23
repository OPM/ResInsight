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
#include "cvfVector2.h"
#include "cvfArray.h"
#include "cvfRect.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class RegGrid2D : public Object
{
public:
    RegGrid2D();

    void            allocateGrid(int gridPointCountI, int gridPointCountJ);
    int             gridPointCountI() const;
    int             gridPointCountJ() const;
    int             gridPointCount() const;

    Vec2d           spacing() const;
    void            setSpacing(const Vec2d& spacing);

    Vec2d           offset() const;
    void            setOffset(const Vec2d& offset);

    Rectd           boundingRectangle() const;

    void            setElevation(int i, int j, double elevation);
    void            setElevation(int arrayIndex, double elevation);
    double          elevation(int i, int j) const;

    void            setAllElevations(double elevation);
    void            setElevations(const DoubleArray& elevations);
    void            setElevations(const double* elevations, int numElevationValues);

    double          pointElevation(const Vec2d& coord) const;
    void            pointElevations(const Array<Vec2d>& coords, DoubleArray* elevations) const;
    void            mapPolylineOnGrid(const Array<Vec2d>& lineCoords, Vec3dArray* intersections) const;
    void            mapLineSegmentOnGrid(const Vec2d& point1, const Vec2d& point2, Vec3dArray* intersections) const;

    bool            minMaxElevation(double* minElevation, double* maxElevation) const;
    bool            minMaxElevationInRegion(int minI, int minJ, int maxI, int maxJ, double* minElevation, double* maxElevation) const;

    Vec2d           gridPointCoordinate(int i, int j) const;
    void            cellFromPoint(const Vec2d& coord, int* i, int* j) const;

private:
    int             toArrayIndex(int i, int j) const;

private:
    int         m_gridPointCountI;
    int         m_gridPointCountJ;
    Vec2d       m_spacing;
    Vec2d       m_offset;

    DoubleArray m_elevations;
};

}
