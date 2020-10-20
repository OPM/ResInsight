/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RigFormationNames.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFormationNames::RigFormationNames()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFormationNames::~RigFormationNames()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigFormationNames::formationNameFromKLayerIdx( size_t Kidx )
{
    int idx = formationIndexFromKLayerIdx( Kidx );
    if ( idx >= static_cast<int>( m_formationNames.size() ) ) return "";
    if ( idx == -1 ) return "";

    return m_formationNames[idx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFormationNames::formationColorFromKLayerIdx( size_t Kidx, cvf::Color3f* formationColor )
{
    int idx = formationIndexFromKLayerIdx( Kidx );

    if ( idx == -1 || idx >= static_cast<int>( m_formationColors.size() ) )
    {
        return false;
    }

    if ( m_formationColors[idx] == undefinedColor() ) return false;

    *formationColor = m_formationColors[idx];
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFormationNames::appendFormationRange( const QString& name, int kStartIdx, int kEndIdx )
{
    appendFormationRangeWithColor( name, undefinedColor(), kStartIdx, kEndIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFormationNames::appendFormationRange( const QString& name, cvf::Color3f color, int kStartIdx, int kEndIdx )
{
    appendFormationRangeWithColor( name, color, kStartIdx, kEndIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFormationNames::appendFormationRangeHeight( const QString& name, int kLayerCount )
{
    if ( kLayerCount < 1 ) return;

    int kStartIdx = static_cast<int>( m_nameIndexPrKLayer.size() );
    int kEndIdx   = kStartIdx + kLayerCount;

    appendFormationRangeWithColor( name, undefinedColor(), kStartIdx, kEndIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFormationNames::appendFormationRangeHeight( const QString& name, cvf::Color3f color, int kLayerCount )
{
    if ( kLayerCount < 1 ) return;

    int kStartIdx = static_cast<int>( m_nameIndexPrKLayer.size() );
    int kEndIdx   = kStartIdx + kLayerCount;

    appendFormationRangeWithColor( name, color, kStartIdx, kEndIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RigFormationNames::undefinedColor()
{
    static cvf::Color3f noColor( -1.0f, -1.0f, -1.0f );

    return noColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFormationNames::appendFormationRangeWithColor( const QString& name, cvf::Color3f color, int kStartIdx, int kEndIdx )
{
    CVF_ASSERT( kStartIdx <= kEndIdx );

    int nameIdx = static_cast<int>( m_formationNames.size() );

    m_formationNames.push_back( name );

    if ( kEndIdx >= static_cast<int>( m_nameIndexPrKLayer.size() ) )
    {
        m_nameIndexPrKLayer.resize( kEndIdx + 1, -1 );
    }

    for ( int kIdx = kStartIdx; kIdx <= kEndIdx; ++kIdx )
    {
        m_nameIndexPrKLayer[kIdx] = nameIdx;
    }

    m_formationColors.push_back( color );
}
