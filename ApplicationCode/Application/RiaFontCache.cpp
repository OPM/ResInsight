/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaFontCache.h"

#include "RiaGuiApplication.h"

#include "cafAppEnum.h"
#include "cafFixedAtlasFont.h"

#include <QDesktopWidget>
#include <cmath>

namespace caf
{
template <>
void RiaFontCache::FontSizeType::setUp()
{
    addItem( RiaFontCache::FONT_SIZE_8, "8", "8" );
    addItem( RiaFontCache::FONT_SIZE_10, "10", "10" );
    addItem( RiaFontCache::FONT_SIZE_12, "12", "12" );
    addItem( RiaFontCache::FONT_SIZE_14, "14", "14" );
    addItem( RiaFontCache::FONT_SIZE_16, "16", "16" );
    addItem( RiaFontCache::FONT_SIZE_24, "24", "24" );
    addItem( RiaFontCache::FONT_SIZE_32, "32", "32" );

    setDefault( RiaFontCache::FONT_SIZE_8 );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FixedAtlasFont::FontSize mapToAtlasFontSize( RiaFontCache::FontSize fontSize )
{
    switch ( fontSize )
    {
        case RiaFontCache::FONT_SIZE_8:
            return caf::FixedAtlasFont::POINT_SIZE_8;
        case RiaFontCache::FONT_SIZE_10:
            return caf::FixedAtlasFont::POINT_SIZE_10;
        case RiaFontCache::FONT_SIZE_12:
            return caf::FixedAtlasFont::POINT_SIZE_12;
        case RiaFontCache::FONT_SIZE_14:
            return caf::FixedAtlasFont::POINT_SIZE_14;
        case RiaFontCache::FONT_SIZE_16:
            return caf::FixedAtlasFont::POINT_SIZE_16;
        case RiaFontCache::FONT_SIZE_24:
            return caf::FixedAtlasFont::POINT_SIZE_24;
        case RiaFontCache::FONT_SIZE_32:
            return caf::FixedAtlasFont::POINT_SIZE_32;
        default:
            return caf::FixedAtlasFont::POINT_SIZE_16;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RiaFontCache::FontSize, cvf::ref<caf::FixedAtlasFont>> RiaFontCache::ms_fonts;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<caf::FixedAtlasFont> RiaFontCache::getFont( FontSize size )
{
    if ( ms_fonts.count( size ) == 0 )
    {
        auto newFont = new caf::FixedAtlasFont( mapToAtlasFontSize( size ) );
        ms_fonts.insert( std::make_pair( size, newFont ) );
    }
    return ms_fonts[size];
}

//--------------------------------------------------------------------------------------------------
/// In the 2019 releases the font size was stored as an enum value rather than actual size
/// Use this method for legacy conversion
//--------------------------------------------------------------------------------------------------
RiaFontCache::FontSize RiaFontCache::legacyEnumToPointSize( int enumValue )
{
    switch ( enumValue )
    {
        case 0:
            return FONT_SIZE_8;
        case 1:
            return FONT_SIZE_10;
        case 2:
            return FONT_SIZE_12;
        case 3:
            return FONT_SIZE_14;
        case 4:
            return FONT_SIZE_16;
        case 5:
            return FONT_SIZE_24;
        case 6:
            return FONT_SIZE_32;
        default:
            return FONT_SIZE_8;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFontCache::FontSize RiaFontCache::fontSizeEnumFromPointSize( int pointSize )
{
    std::vector<FontSize> allValues =
        {FONT_SIZE_8, FONT_SIZE_10, FONT_SIZE_12, FONT_SIZE_14, FONT_SIZE_16, FONT_SIZE_24, FONT_SIZE_32};

    FontSize closestEnumValue = FONT_SIZE_8;
    int      closestDiff      = std::numeric_limits<int>::max();
    for ( FontSize enumValue : allValues )
    {
        int diff = std::abs( (int)enumValue - pointSize );
        if ( diff < closestDiff )
        {
            closestEnumValue = enumValue;
            closestDiff      = diff;
        }
    }
    return closestEnumValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaFontCache::pointSizeToPixelSize( int pointSize )
{
    auto app = RiaGuiApplication::instance();
    if ( app )
    {
        int    dpi    = app->desktop()->logicalDpiX();
        double inches = pointSize / 72.0;
        return static_cast<int>( std::ceil( inches * dpi ) );
    }
    return pointSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaFontCache::pixelSizeToPointSize( int pixelSize )
{
    auto app = RiaGuiApplication::instance();
    if ( app )
    {
        int    dpi    = app->desktop()->logicalDpiX();
        double inches = pixelSize / dpi;
        return static_cast<int>( std::ceil( inches * 72.0 ) );
    }
    return pixelSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFontCache::clear()
{
    ms_fonts.clear();
}
