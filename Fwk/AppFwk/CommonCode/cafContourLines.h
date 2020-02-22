/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//  Copyright (C) 2018-     Ceetron Solutions AS
//
//  Adapted from work by Paul D. Bourke named "conrec"
//
//  http://paulbourke.net/papers/conrec/.
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

#include "cvfBase.h"
#include "cvfVector2.h"
#include "cvfVector3.h"

#include <deque>
#include <limits>
#include <list>
#include <vector>

namespace caf
{
class ContourLines
{
public:
    typedef std::pair<cvf::Vec3d, cvf::Vec3d> LineSegment;
    typedef std::list<LineSegment> ListOfLineSegments;

    static std::vector<ListOfLineSegments> create(const std::vector<double>&            dataXY,
                                                  const std::vector<double>&            xPositions,
                                                  const std::vector<double>&            yPositions,
                                                  const std::vector<double>&            contourLevels);
    
private:
    static void create(const std::vector<double>& dataXY,
                       const std::vector<double>& xPositions,
                       const std::vector<double>& yPositions,
                       const std::vector<double>& contourLevels,
                       std::vector<std::vector<cvf::Vec2d>>* polygons);
    static double contourRange(const std::vector<double>& contourLevels);
    static double invalidValue(const std::vector<double>& contourLevels);
    static double saneValue(int index, const std::vector<double>& dataXY, const std::vector<double>& contourLevels);
    static double xsect(int p1, int p2, const std::vector<double>& h, const std::vector<double>& xh, const std::vector<double>& yh);
    static double ysect(int p1, int p2, const std::vector<double>& h, const std::vector<double>& xh, const std::vector<double>& yh);
    static int    gridIndex1d(int i, int j, int nx);
private:
    static const int s_castab[3][3][3];
};
}