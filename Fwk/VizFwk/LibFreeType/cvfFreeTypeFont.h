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
#include "cvfArray.h"
#include "cvfString.h"

#include <map>

namespace cvf {

class Glyph;
class TextureImage;
class FreeTypeFontInterface;


//==================================================================================================
//
// FreeType font class used to generate glyphs for a given character
//
//==================================================================================================
class FreeTypeFont : public Font
{
public:
    FreeTypeFont(uint dpiX = 96, uint dpiY = 96);
    virtual ~FreeTypeFont();

    // Overridden pure virtual functions
    virtual const String& name() const;
    virtual ref<Glyph> getGlyph(wchar_t character);
    virtual uint advance(wchar_t character, wchar_t nextCharacter);
    virtual bool isEmpty();

    void setSize(uint size);

    // Load/unload font
    bool load(const String& path);
    bool load(const UByteArray* data);
    bool load(const char** data, size_t numBlocks);
    void unload();

    // Library information
    static bool     version(int* major, int* minor, int* patch);
    static String   copyRightYear();
    static String   credit();

private:
    typedef std::map<wchar_t, ref<Glyph> > MapType;

    ref<FreeTypeFontInterface> m_fontInterface;
    MapType m_atlasMap;     // Atlas containing cached up Glyphs
    String  m_name;         
};

} // namespace cvf
