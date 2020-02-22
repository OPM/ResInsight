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
#include "cvfArray.h"

namespace cvf {

class TextureImage;
class RenderState;
class OpenGLContext;


//==================================================================================================
//
// Pre-rendered text character
//
//==================================================================================================
class Glyph : public Object
{
public:
    enum TextureFilter
    {
        NEAREST,
        LINEAR
    };

    Glyph(wchar_t character);
    virtual ~Glyph();

    wchar_t             character() const;

    void                setWidth(uint width);
    uint                width() const;
    void                setHeight(uint height);
    uint                height() const;

    void                setTextureImage(TextureImage* textureImage);
    const TextureImage* textureImage() const;
    const FloatArray*   textureCoordinates() const;

    void                setHorizontalBearingX(short bearing);
    short               horizontalBearingX() const;
    void                setHorizontalBearingY(short bearing);
    short               horizontalBearingY() const;
    void                setHorizontalAdvance(uint advance);
    uint                horizontalAdvance() const;

    void                setupAndBindTexture(OpenGLContext* oglContext, bool software);

    void                setMinFilter(TextureFilter filter);
    void                setMagFilter(TextureFilter filter);

private:
    wchar_t           m_character;          // Character this glyph is generated from

    // Size of the pre-rendered character in pixels. 
    // Note: May be smaller than the size of m_textureImage since this may contain alpha's and even more characters
    uint              m_width;
    uint              m_height;

    // For horizontal text layouts
    short             m_horizontalBearingX; // Horizontal distance from the current cursor position to the leftmost border of the glyph image's bounding box.
    short             m_horizontalBearingY; // Vertical distance from the current cursor position (on the baseline) to the topmost border of the glyph image's bounding box.
    uint              m_horizontalAdvance;  // Horizontal distance used to increment the pen position when the glyph is drawn as part of a string of text.

    // Texture info
    ref<TextureImage> m_textureImage;       // Pre-rendered image of m_character
    ref<FloatArray>   m_textureCoordinates; // Texture coordinates of where in the m_texgtureImage to find the given pre-rendered character
    ref<RenderState>  m_textureBindings;    // For shader based rendering this is a TextureBindings object, while software rendering uses RenderStateTextureMapping_FF instead

    // Texture filter options
    TextureFilter     m_minFilter;
    TextureFilter     m_magFilter;
};

} // namespace cvf
