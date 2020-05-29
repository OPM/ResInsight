/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Equinor ASA
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

#include <QString>
#include <vector>

#include "cvfVector3.h"

class RigGocadData
{
public:
    RigGocadData();
    ~RigGocadData();

    std::vector<QString>                                      propertyNames();
    std::pair<std::vector<cvf::Vec3d>, std::vector<unsigned>> gocadGeometry();
    std::vector<float>                                        propertyValues( const QString& property );

    void setGeometryData( const std::vector<cvf::Vec3d>& nodeCoord, const std::vector<unsigned>& connectivities );
    void addPropertyData( const std::vector<QString>& propertyNames, std::vector<std::vector<float>>& propertyValues );

private:
    std::vector<cvf::Vec3d>         m_vertices;
    std::vector<unsigned>           m_tringleIndices;
    std::vector<QString>            m_propertyNames;
    std::vector<std::vector<float>> m_propertyValues;
};
