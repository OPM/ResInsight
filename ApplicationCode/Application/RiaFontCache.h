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

#include "cafFixedAtlasFont.h"
#include "cafFontTools.h"

#include "cvfObject.h"

#include <map>

namespace caf
{
template <typename T>
class AppEnum;
}

class RimSummaryCaseCollection;

//==================================================================================================
///
//==================================================================================================
class RiaFontCache
{
public:
    using FontSize     = caf::FontTools::FontSize;
    using FontSizeEnum = caf::FontTools::FontSizeEnum;

    static cvf::ref<caf::FixedAtlasFont> getFont( FontSize fontSize );
    static cvf::ref<caf::FixedAtlasFont> getFont( int pointSize );
    static FontSize                      legacyEnumToPointSize( int enumValue );
    static void                          clear();

private:
    static std::map<caf::FixedAtlasFont::FontSize, cvf::ref<caf::FixedAtlasFont>> ms_fonts;
};
