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

#pragma once

#include <cvfBase.h>
#include <cafFixedAtlasFont.h>

#include <map>

class RimSummaryCaseCollection;

//==================================================================================================
/// 
//==================================================================================================
class RiaFontCache
{
public:
    enum FontSize
    {
        FONT_SIZE_8,
        FONT_SIZE_10,
        FONT_SIZE_12,
        FONT_SIZE_14,
        FONT_SIZE_16,
        FONT_SIZE_24,
        FONT_SIZE_32
    };

    static cvf::ref<caf::FixedAtlasFont> getFont(FontSize size);

private:
    static caf::FixedAtlasFont::FontSize mapToAtlasFontSize(FontSize fontSize);

    static std::map<FontSize, cvf::ref<caf::FixedAtlasFont>> ms_fonts;
};
