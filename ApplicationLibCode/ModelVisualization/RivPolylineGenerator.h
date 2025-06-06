/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include <cvfVector3.h>

#include <QString>

#include <vector>

namespace cvf
{
class DrawableGeo;
class DrawableText;
class Camera;
} // namespace cvf

//==================================================================================================
///
//==================================================================================================
class RivPolylineGenerator
{
public:
    static cvf::ref<cvf::DrawableGeo> createLineAlongPolylineDrawable( const std::vector<cvf::Vec3d>& polyLine, bool closeLine = false );
    static cvf::ref<cvf::DrawableGeo> createLineAlongPolylineDrawable( const std::vector<std::vector<cvf::Vec3d>>& polyLines,
                                                                       bool                                        closeLine = false );

    static cvf::ref<cvf::DrawableGeo> createPointsFromPolylineDrawable( const std::vector<cvf::Vec3d>& polyLine );
    static cvf::ref<cvf::DrawableGeo> createPointsFromPolylineDrawable( const std::vector<std::vector<cvf::Vec3d>>& polyLines );

    static cvf::ref<cvf::DrawableGeo> createSetOfLines( const std::vector<std::vector<cvf::Vec3d>>& lines );

    static cvf::ref<cvf::DrawableText>
        createOrientedLabel( bool negativeXDirection, const cvf::Camera* camera, const cvf::Vec3d& labelPosition, const QString& labelText );
};
