/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cvfObject.h"
#include "cvfVector3.h"

#include <QString>

#include <map>
#include <vector>

class RigSurface : public cvf::Object
{
public:
    RigSurface();
    ~RigSurface() override;

    const std::vector<unsigned>&   triangleIndices();
    const std::vector<cvf::Vec3d>& vertices();

    void setTriangleData( const std::vector<unsigned>& tringleIndices, const std::vector<cvf::Vec3d>& vertices );
    void addVerticeResult( const QString resultName, const std::vector<double>& resultValues );

private:
    std::vector<unsigned>                  m_triangleIndices;
    std::vector<cvf::Vec3d>                m_vertices;
    std::map<QString, std::vector<double>> m_verticeResults;
};
