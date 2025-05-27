/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "cvfVector3.h"

#include <QString>

#include <vector>

namespace RigPolygonTools
{
// Integer images are assumed to be 2D arrays of integers, where 0 is background and 1 is foreground.
using IntegerImage = std::vector<std::vector<int>>;
using Point        = std::pair<int, int>;

IntegerImage erode( IntegerImage image, int kernelSize );
IntegerImage dilate( IntegerImage image, int kernelSize );
IntegerImage fillInterior( IntegerImage sourceImage );

std::vector<std::pair<int, int>> boundary( const IntegerImage& image );
IntegerImage                     assignValueInsidePolygon( IntegerImage image, const std::vector<Point>& polygon, int value );
double                           area( const std::vector<Point>& polygon );

// Recursive function modifying the incoming vertices
void simplifyPolygon( std::vector<cvf::Vec3d>& vertices, double epsilon );

QString geometryDataAsText( const std::vector<cvf::Vec3d>& vertices, bool includeLastSegmentInfo );
} // namespace RigPolygonTools
