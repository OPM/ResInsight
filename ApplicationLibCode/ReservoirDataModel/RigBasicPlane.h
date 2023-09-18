/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "cvfArray.h"
#include "cvfColor3.h"
#include "cvfObject.h"
#include "cvfPlane.h"
#include "cvfVector3.h"

#include <memory>
#include <vector>

namespace cvf
{
class TextureImage;
}

//==================================================================================================
///
///
//==================================================================================================
class RigBasicPlane : public cvf::Object
{
public:
    RigBasicPlane();
    ~RigBasicPlane() override;

    bool isValid() const;
    void reset();

    void updateRect();

    void setPlane( cvf::Vec3d anchorPoint, cvf::Vec3d normal );
    void setMaxExtentFromAnchor( double maxExtentHorz, double maxExtentVertAbove, double maxExtentVertBelow );
    void setColor( cvf::Color3f color );

    double                            maxDepth();
    std::pair<cvf::Vec3d, cvf::Vec3d> intersectTopBottomLine();
    cvf::Vec3d                        normal() const;

    cvf::Vec3dArray             rect() const;
    cvf::ref<cvf::TextureImage> texture() const;

private:
    cvf::Vec3d m_planeNormal;
    cvf::Vec3d m_planeAnchor;

    double m_maxHorzExtent;
    double m_maxVertExtentAbove;
    double m_maxVertExtentBelow;

    cvf::Vec3d m_topIntersect;
    cvf::Vec3d m_bottomIntersect;

    cvf::Vec3dArray             m_rect;
    bool                        m_isRectValid;
    cvf::Color3f                m_color;
    cvf::ref<cvf::TextureImage> m_texture;
};
