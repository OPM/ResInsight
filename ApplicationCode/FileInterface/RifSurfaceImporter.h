/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include <utility>
#include <vector>
#include <limits>

#include <QString>

class RigGocadData;

class RifSurfaceImporter
{
public:
    static void readGocadFile( const QString& filename, RigGocadData* gocadData );
    static std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> readPetrelFile( const QString& filename );
    static std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> readOpenWorksXyzFile( const QString& filename );

private:
    static bool       generateTriangleIndices( const std::vector<std::vector<unsigned>>& indexToPointData,
                                               const size_t&                             i,
                                               const size_t&                             j,
                                               std::vector<unsigned>&                    triangleIndices );
    static bool       vectorFuzzyCompare( const cvf::Vec2d& vector1,
                                          const cvf::Vec2d& vector2,
                                          double            epsilon = std::numeric_limits<double>::epsilon() );
};
