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

#include <zgyaccess/seismicslice.h>

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
void RigTexturedSection::addSection( cvf::Vec3dArray rect, std::shared_ptr<ZGYAccess::SeismicSliceData> data )
{
    m_sectionRects.push_back( rect );
    m_slicedata.push_back( data );
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
std::shared_ptr<ZGYAccess::SeismicSliceData> RigTexturedSection::slicedata( int index ) const
{
    if ( index >= (int)m_slicedata.size() ) return nullptr;
    if ( index < 0 ) return nullptr;
    return m_slicedata[index];
}
