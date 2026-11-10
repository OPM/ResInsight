/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "cvfColor3.h"
#include "cvfFont.h"
#include "cvfObject.h"
#include "cvfVector2.h"
#include "cvfVector3.h"

#include <string>
#include <vector>

namespace cvf
{
class Camera;
} // namespace cvf

//==================================================================================================
/// Labels attached to the surface of a pipe, e.g. measured depth labels along a well path. The label positions are
/// computed when the camera changes. The anchor point of each label is placed at the radial position of the pipe closest
/// to the camera, and the label is offset from the anchor point in the screen plane, perpendicular to the pipe.
//==================================================================================================
class RivPipeSurfaceLabelSourceInfo : public cvf::Object
{
public:
    struct Label
    {
        std::string text;
        cvf::Vec3d  centerDisplayCoord;
        cvf::Vec3d  tangent;
    };

    RivPipeSurfaceLabelSourceInfo( const std::vector<Label>& labels,
                                   double                    pipeRadius,
                                   const cvf::Color3f&       anchorColor,
                                   const cvf::Color3f&       backgroundColor,
                                   cvf::Font*                font );

    const std::vector<Label>& labels() const;
    cvf::Color3f              anchorColor() const;
    cvf::Color3f              backgroundColor() const;
    cvf::Font*                font() const;

    cvf::Vec3d anchorDisplayCoord( const Label& label, const cvf::Camera* camera ) const;

    static cvf::Vec3d closestSurfacePointToCamera( const cvf::Vec3d& centerDisplayCoord,
                                                   const cvf::Vec3d& tangent,
                                                   double            pipeRadius,
                                                   const cvf::Vec3d& directionToCamera );

    // Unit direction in screen coordinates perpendicular to the pipe direction in screen coordinates, pointing upwards
    static cvf::Vec2d screenOffsetDirection( const cvf::Vec2d& screenTangent );

private:
    std::vector<Label>          m_labels;
    double                      m_pipeRadius;
    cvf::Color3f                m_anchorColor;
    cvf::Color3f                m_backgroundColor;
    mutable cvf::ref<cvf::Font> m_font;
};