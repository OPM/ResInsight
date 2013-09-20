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

#include "cvfObject.h"
#include "cvfFont.h"
#include "cvfGlyph.h"
#include "cvfString.h"
#include <map>

namespace cvf {

class TextureImage;
class Glyph;


//==================================================================================================
//
// Fixed atlas font class used to generate glyphs for a given character
//
//==================================================================================================
class FixedAtlasFont : public Font
{
public:
    enum FontSize
    {
        STANDARD,           // 8pt
        LARGE               // 16pt
    };

public:
    FixedAtlasFont(FontSize size);
    virtual ~FixedAtlasFont();

    virtual const String& name() const;
    virtual ref<Glyph> getGlyph(wchar_t character);
    virtual uint advance(wchar_t character, wchar_t nextCharacter);
	virtual bool isEmpty();

private:
    // Load/unload font
    bool load(const char* name, size_t numGlyphs, 
        const short* horizontalBearingsX, const short* horizontalBearingsY, 
        const uint* horizontalAdvances, const uint* characterWidths, const uint* characterHeightss,
        const size_t textureImageWidth, const size_t textureImageHeight,
        const size_t numTextureImageDataBlockCount, const char** textureImageData);
    void unload();

private:
    String             m_name;
    size_t             m_numGlyphs;
    std::vector<short> m_horizontalBearingsX;
    std::vector<short> m_horizontalBearingsY;
    std::vector<uint>  m_horizontalAdvances;
    std::vector<uint>  m_characterWidths;
    std::vector<uint>  m_characterHeights;

    ref<TextureImage>  m_textureImage;

    // Glyph cache
    typedef std::map<size_t, ref<Glyph> > MapType;
    MapType m_atlasMap;
};

} // namespace cvf
