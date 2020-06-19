//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once

#include "cvfBase.h"
#include "cvfFont.h"
#include "cvfGlyph.h"
#include "cvfObject.h"
#include "cvfString.h"
#include <map>

namespace cvf
{
class TextureImage;
class Glyph;

} // namespace cvf

namespace caf
{
//==================================================================================================
//
// Fixed atlas font class used to generate glyphs for a given character
//
//==================================================================================================
class FixedAtlasFont : public cvf::Font
{
public:
    enum FontSize
    {
        POINT_SIZE_6, // 6pt
        POINT_SIZE_8, // 8pt
        POINT_SIZE_10,
        POINT_SIZE_12,
        POINT_SIZE_14,
        POINT_SIZE_16, // 16pt
        POINT_SIZE_24,
        POINT_SIZE_32
    };

public:
    explicit FixedAtlasFont( FontSize size );
    ~FixedAtlasFont() override;

    const cvf::String&   name() const override;
    cvf::ref<cvf::Glyph> getGlyph( wchar_t character ) override;
    cvf::uint            advance( wchar_t character, wchar_t nextCharacter ) override;
    bool                 isEmpty() override;

private:
    // Load/unload font
    bool load( const char*      name,
               size_t           numGlyphs,
               const short*     horizontalBearingsX,
               const short*     horizontalBearingsY,
               const cvf::uint* horizontalAdvances,
               const cvf::uint* characterWidths,
               const cvf::uint* characterHeightss,
               const size_t     textureImageWidth,
               const size_t     textureImageHeight,
               const size_t     numTextureImageDataBlockCount,
               const char**     textureImageData );
    void unload();

private:
    cvf::String            m_name;
    size_t                 m_numGlyphs;
    std::vector<short>     m_horizontalBearingsX;
    std::vector<short>     m_horizontalBearingsY;
    std::vector<cvf::uint> m_horizontalAdvances;
    std::vector<cvf::uint> m_characterWidths;
    std::vector<cvf::uint> m_characterHeights;

    cvf::ref<cvf::TextureImage> m_textureImage;

    // Glyph cache
    typedef std::map<size_t, cvf::ref<cvf::Glyph>> MapType;
    MapType                                        m_atlasMap;
};

} // namespace caf
