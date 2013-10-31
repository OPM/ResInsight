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

#include "cvfOverlayItem.h"
#include "cvfMatrix4.h"
#include "cvfColor3.h"
#include "cvfString.h"

namespace cvf {

class Font;
class TextDrawer;

//==================================================================================================
//
// Overlay text box
//
//==================================================================================================
class OverlayTextBox : public OverlayItem
{
public:
    OverlayTextBox(Font* font);
    ~OverlayTextBox();

    virtual Vec2ui  sizeHint();

    virtual void    render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);
    virtual void    renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);

    void            setText(const String& text);
    void            setPixelSize(const Vec2ui& size);
    void            setSizeToFitText();

    void            setTextColor(const Color3f& color);
    void            setBackgroundColor(const Color3f& color);
    void            setBorderColor(const Color3f& color);
    void            setDrawBackground(bool drawBackground);
    void            setDrawBorder(bool drawBorder);

    String          text() const;
    Color3f         textColor() const;
    Color3f         backgroundColor() const;
    Color3f         borderColor() const;
    bool            drawBackground() const;
    bool            drawBorder() const;


private:
    void render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software);
    void renderBackgroundAndBorder(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software);

private:
    Vec2ui              m_size;
    String              m_text;
    ref<TextDrawer>     m_textDrawer;
    ref<Font>           m_font;

    bool                m_drawBackground;
    bool                m_drawBorder;
    Color3f             m_textColor;
    Color3f             m_backgroundColor;  
    Color3f             m_borderColor;      
};

}
