/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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
#include <map>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RimWellIAModelBox
{
public:
    RimWellIAModelBox();
    ~RimWellIAModelBox();

    std::vector<cvf::Vec3d> vertices() const;
    cvf::Vec3d              center() const;

    bool updateBox( cvf::Vec3d startPos, cvf::Vec3d endPos, double xyBuffer, double depthBuffer );

private:
    std::vector<cvf::Vec3d> generateRectangle( cvf::Vec3d center, cvf::Vec3d unitX, cvf::Vec3d unitY, double buffer );

    std::vector<cvf::Vec3d> m_vertices;
    cvf::Vec3d              m_center;
};
