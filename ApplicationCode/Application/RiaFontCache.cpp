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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RiaFontCache::FontSize, cvf::ref<caf::FixedAtlasFont>> RiaFontCache::ms_fonts;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<caf::FixedAtlasFont> RiaFontCache::getFont(FontSize size)
{
    if (ms_fonts.count(size) == 0)
    {
        auto newFont = new caf::FixedAtlasFont(mapToAtlasFontSize(size));
        ms_fonts.insert(std::make_pair(size, newFont));
    }
    return ms_fonts[size];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FixedAtlasFont::FontSize RiaFontCache::mapToAtlasFontSize(FontSize fontSize)
{
    switch (fontSize)
    {
    case FONT_SIZE_8:   return caf::FixedAtlasFont::POINT_SIZE_8;
    case FONT_SIZE_10:  return caf::FixedAtlasFont::POINT_SIZE_10;
    case FONT_SIZE_12:  return caf::FixedAtlasFont::POINT_SIZE_12;
    case FONT_SIZE_14:  return caf::FixedAtlasFont::POINT_SIZE_14;
    case FONT_SIZE_16:  return caf::FixedAtlasFont::POINT_SIZE_16;
    case FONT_SIZE_24:  return caf::FixedAtlasFont::POINT_SIZE_24;
    case FONT_SIZE_32:  return caf::FixedAtlasFont::POINT_SIZE_32;
    default:            return caf::FixedAtlasFont::POINT_SIZE_16;
    }
}
