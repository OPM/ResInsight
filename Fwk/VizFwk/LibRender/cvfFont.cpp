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
/// 
//--------------------------------------------------------------------------------------------------
float Font::lineSpacing()
{
    ref<Glyph> glyph = getGlyph(L'A');

    float spacing = cvf::Math::floor(static_cast<float>(glyph->height())*1.75f);

    return spacing;
}


//--------------------------------------------------------------------------------------------------
/// Get the extent (width and height) of the given text with this font in pixels
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui Font::textExtent(const String& text)
{
    std::vector<cvf::String> lines = text.split("\n");

    float maxLineWidth = 0;
    uint textHeight = 0;
    uint lineSpacing = static_cast<uint>(this->lineSpacing());
    for (size_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx)
    {
        String line = lines[lineIdx];
        size_t numCharacters = line.size();
        float lineWidth = 0;

        for (size_t j = 0; j < numCharacters; ++j)
        {
            wchar_t character = line[j];

            // Jump to the next character in the string, if any
            if (j < (numCharacters - 1))
            {
                float advance = static_cast<float>(this->advance(character, text[j + 1]));
                lineWidth += advance;
            }
            else
            {
                ref<Glyph> glyph = getGlyph(character);
                
                lineWidth += static_cast<float>(glyph->width()) + static_cast<float>(glyph->horizontalBearingX());
            }
        }

        maxLineWidth = CVF_MAX(lineWidth, maxLineWidth);

        if (lineIdx == 0)
        {
            ref<Glyph> glyph = getGlyph(L'A');
            textHeight += glyph->height();
        }
        else
        {
            textHeight += lineSpacing;
        }
    }

    return Vec2ui(static_cast<uint>(maxLineWidth), textHeight);
}

//     Vec2ui textBB(0,0);
// 
//     int minHeight = std::numeric_limits<int>::max();
//     int maxHeight = std::numeric_limits<int>::min();
// 
//     size_t numCharacters = text.size();
//     for (size_t j = 0; j < numCharacters; j++)
//     {
//         wchar_t character = text[j];
//         ref<Glyph> glyph = getGlyph(character);
// 
//         // Find bottom and top with regards to baseline (Y = 0)
//         int minY = static_cast<int>(glyph->horizontalBearingY()) - static_cast<int>(glyph->height());
//         int maxY = glyph->horizontalBearingY();
// 
//         if (minHeight > minY) minHeight = minY;
//         if (maxHeight < maxY) maxHeight = maxY;
// 
//         uint charWidth  = 0;
// 
//         if (j < (numCharacters - 1))
//         {
//             charWidth = advance(character, text[j + 1]);
//         }
//         else
//         {
//             charWidth  = glyph->width() + glyph->horizontalBearingX();
//         }
// 
//         textBB.x() += charWidth;
//     }
// 
//     if (maxHeight < minHeight)
//     {
//         return Vec2ui(0,0);
//     }
// 
//     textBB.y() = static_cast<uint>(maxHeight - minHeight);
// 
//     return textBB;

}  // namespace cvf
