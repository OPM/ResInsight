/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include <vector>

namespace cvf
{
class TextureImage;
}

//==================================================================================================
///
///
//==================================================================================================
class RigTexturedSection : public cvf::Object
{
public:
    RigTexturedSection();
    ~RigTexturedSection() override;

    void addSection( cvf::Vec3dArray rect, cvf::TextureImage* image );

    const std::vector<cvf::Vec3dArray>&    rects() const;
    const std::vector<cvf::TextureImage*>& images() const;

    cvf::Vec3dArray    rect( int index ) const;
    cvf::TextureImage* image( int index ) const;

private:
    std::vector<cvf::Vec3dArray>    m_sectionRects;
    std::vector<cvf::TextureImage*> m_images;
};
