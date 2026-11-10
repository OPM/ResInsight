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

#include "RivPipeSurfaceLabelSourceInfo.h"

#include "cvfCamera.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivPipeSurfaceLabelSourceInfo::RivPipeSurfaceLabelSourceInfo( const std::vector<Label>& labels,
                                                              double                    pipeRadius,
                                                              const cvf::Color3f&       anchorColor,
                                                              const cvf::Color3f&       backgroundColor,
                                                              cvf::Font*                font )
    : m_labels( labels )
    , m_pipeRadius( pipeRadius )
    , m_anchorColor( anchorColor )
    , m_backgroundColor( backgroundColor )
    , m_font( font )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RivPipeSurfaceLabelSourceInfo::Label>& RivPipeSurfaceLabelSourceInfo::labels() const
{
    return m_labels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RivPipeSurfaceLabelSourceInfo::anchorColor() const
{
    return m_anchorColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RivPipeSurfaceLabelSourceInfo::backgroundColor() const
{
    return m_backgroundColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RivPipeSurfaceLabelSourceInfo::font() const
{
    return m_font.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivPipeSurfaceLabelSourceInfo::anchorDisplayCoord( const Label& label, const cvf::Camera* camera ) const
{
    if ( !camera ) return label.centerDisplayCoord;

    cvf::Vec3d directionToCamera = -camera->direction();
    if ( camera->projection() == cvf::Camera::PERSPECTIVE )
    {
        directionToCamera = camera->position() - label.centerDisplayCoord;
    }

    return closestSurfacePointToCamera( label.centerDisplayCoord, label.tangent, m_pipeRadius, directionToCamera );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivPipeSurfaceLabelSourceInfo::closestSurfacePointToCamera( const cvf::Vec3d& centerDisplayCoord,
                                                                       const cvf::Vec3d& tangent,
                                                                       double            pipeRadius,
                                                                       const cvf::Vec3d& directionToCamera )
{
    cvf::Vec3d unitTangent = tangent;
    if ( !unitTangent.normalize() ) return centerDisplayCoord;

    // Project the direction to the camera onto the plane perpendicular to the pipe
    cvf::Vec3d radialDirection = directionToCamera - unitTangent * ( directionToCamera * unitTangent );

    // The camera is looking along the pipe, place the label at the pipe center
    if ( !radialDirection.normalize() ) return centerDisplayCoord;

    return centerDisplayCoord + radialDirection * pipeRadius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RivPipeSurfaceLabelSourceInfo::screenOffsetDirection( const cvf::Vec2d& screenTangent )
{
    cvf::Vec2d direction( -screenTangent.y(), screenTangent.x() );

    // The pipe is seen end-on, place the label above the anchor point
    if ( !direction.normalize() ) return { 0.0, 1.0 };

    // Screen coordinates have origin in the lower left corner
    if ( direction.y() < 0.0 ) direction = -direction;

    return direction;
}