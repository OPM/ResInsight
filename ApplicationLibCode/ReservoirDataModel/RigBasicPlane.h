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

    void setRect( cvf::Vec3dArray rect );
    void setColor( cvf::Color3f color );

    cvf::Vec3dArray             rect() const;
    cvf::ref<cvf::TextureImage> texture() const;

private:
    cvf::Vec3dArray             m_rect;
    bool                        m_isRectValid;
    cvf::Color3f                m_color;
    cvf::ref<cvf::TextureImage> m_texture;
};
