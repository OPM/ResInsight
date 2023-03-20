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
void RigTexturedSection::setWhatToUpdate( WhatToUpdateEnum updateInfo, int index /*=-1*/ )
{
    int start = index;
    int stop  = index + 1;

    if ( index < 0 )
    {
        start = 0;
        stop  = m_sectionParts.size();
    }

    for ( int i = start; i < stop; i++ )
    {
        switch ( updateInfo )
        {
            case RigTexturedSection::WhatToUpdateEnum::UPDATE_ALL:
            case RigTexturedSection::WhatToUpdateEnum::UPDATE_GEOMETRY:
                m_sectionParts[i].isRectValid = false;
                m_sectionParts[i].rect.clear();
            case RigTexturedSection::WhatToUpdateEnum::UPDATE_DATA:
                m_sectionParts[i].sliceData = nullptr;
            case RigTexturedSection::WhatToUpdateEnum::UPDATE_TEXTURE:
                m_sectionParts[i].texture = nullptr;
            case RigTexturedSection::WhatToUpdateEnum::UPDATE_NONE:
            default:
                break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigTexturedSection::isValid() const
{
    if ( m_sectionParts.size() == 0 ) return false;

    bool valid = true;

    for ( int i = 0; i < m_sectionParts.size(); i++ )
    {
        valid = valid && m_sectionParts[i].allDataValid();
    }

    return valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigTexturedSection::partsCount() const
{
    return m_sectionParts.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTexturedSection::resize( int size )
{
    m_sectionParts.resize( size );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTexturedSectionPart& RigTexturedSection::part( int index )
{
    CVF_ASSERT( index < m_sectionParts.size() );
    return m_sectionParts[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTexturedSection::setSectionPartRect( int index, cvf::Vec3dArray rect )
{
    while ( index >= m_sectionParts.size() )
    {
        m_sectionParts.emplace_back();
    }

    m_sectionParts[index].rect        = rect;
    m_sectionParts[index].isRectValid = true;
    m_sectionParts[index].sliceData   = nullptr;
    m_sectionParts[index].texture     = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTexturedSection::setSectionPartData( int index, std::shared_ptr<ZGYAccess::SeismicSliceData> data )
{
    if ( index >= m_sectionParts.size() ) return;
    m_sectionParts[index].sliceData = data;
    m_sectionParts[index].texture   = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigTexturedSection::setSectionPartTexture( int index, cvf::ref<cvf::TextureImage> texture )
{
    if ( index >= m_sectionParts.size() ) return;
    m_sectionParts[index].texture = texture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigTexturedSection::hasSectionPartRect( int index )
{
    if ( index >= m_sectionParts.size() ) return false;
    return m_sectionParts[index].isRectValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigTexturedSection::hasSectionPartData( int index )
{
    if ( index >= m_sectionParts.size() ) return false;
    return ( m_sectionParts[index].sliceData != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigTexturedSection::hasSectionPartTexture( int index )
{
    if ( index >= m_sectionParts.size() ) return false;
    return m_sectionParts[index].texture.notNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3dArray RigTexturedSection::rect( int index ) const
{
    if ( index >= (int)m_sectionParts.size() ) return {};
    if ( index < 0 ) return {};
    return m_sectionParts[index].rect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData> RigTexturedSection::slicedata( int index ) const
{
    if ( index >= (int)m_sectionParts.size() ) return nullptr;
    if ( index < 0 ) return nullptr;
    return m_sectionParts[index].sliceData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> RigTexturedSection::texture( int index ) const
{
    if ( index >= (int)m_sectionParts.size() ) return nullptr;
    if ( index < 0 ) return nullptr;
    return m_sectionParts[index].texture;
}
