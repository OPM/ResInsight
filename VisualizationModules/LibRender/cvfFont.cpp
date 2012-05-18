//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cvfBase.h"
#include "cvfFont.h"
#include "cvfGlyph.h"


namespace cvf {

//==================================================================================================
///
/// \class cvf::Font
/// \ingroup Render
///
/// Pure virtual font base class used to generate glyphs for a given character.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
Font::Font()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
Font::~Font()
{

}


//--------------------------------------------------------------------------------------------------
/// Get the extent (width and height) of the given text with this font in pixels
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui Font::textExtent(const String& text)
{
    Vec2ui textBB(0,0);

    int minHeight = std::numeric_limits<int>::max();
    int maxHeight = std::numeric_limits<int>::min();

    size_t numCharacters = text.size();
    for (size_t j = 0; j < numCharacters; j++)
    {
        wchar_t character = text[j];
        ref<Glyph> glyph = getGlyph(character);

        // Find bottom and top with regards to baseline (Y = 0)
        int minY = static_cast<int>(glyph->horizontalBearingY()) - static_cast<int>(glyph->height());
        int maxY = glyph->horizontalBearingY();

        if (minHeight > minY) minHeight = minY;
        if (maxHeight < maxY) maxHeight = maxY;

        uint charWidth  = 0;

        if (j < (numCharacters - 1))
        {
            charWidth = advance(character, text[j + 1]);
        }
        else
        {
            charWidth  = glyph->width() + glyph->horizontalBearingX();
        }

        textBB.x() += charWidth;
    }

    if (maxHeight < minHeight)
    {
        return Vec2ui(0,0);
    }

    textBB.y() = static_cast<uint>(maxHeight - minHeight);

    return textBB;
}

}  // namespace cvf
