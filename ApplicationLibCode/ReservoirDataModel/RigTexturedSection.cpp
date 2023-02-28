/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RigTexturedSection.h"

#include "cvfTextureImage.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTexturedSection::RigTexturedSection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTexturedSection::~RigTexturedSection()
{
    for ( auto image : m_images )
    {
        delete image;
    }
    m_images.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3dArray>& RigTexturedSection::rects() const
{
    return m_sectionRects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTexturedSection::addSection( cvf::Vec3dArray rect, cvf::TextureImage* image )
{
    m_sectionRects.push_back( rect );
    m_images.push_back( image );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3dArray RigTexturedSection::rect( int index ) const
{
    if ( index >= (int)m_sectionRects.size() ) return {};
    if ( index < 0 ) return {};
    return m_sectionRects[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::TextureImage* RigTexturedSection::image( int index ) const
{
    if ( index >= (int)m_images.size() ) return nullptr;
    if ( index < 0 ) return nullptr;
    return m_images[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::TextureImage*>& RigTexturedSection::images() const
{
    return m_images;
}
