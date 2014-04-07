/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "cvfOverlayItem.h"
#include "cvfMatrix4.h"
#include "cvfColor3.h"
#include "cvfString.h"

namespace cvf {

class Font;

}

//==================================================================================================
//
// 
//
//==================================================================================================
class RivTernarySaturationOverlayItem : public cvf::OverlayItem
{
public:
    RivTernarySaturationOverlayItem(cvf::Font* font);
    ~RivTernarySaturationOverlayItem();

    virtual cvf::Vec2ui  sizeHint();

    virtual void    render(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size);
    virtual void    renderSoftware(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size);

    void            setSize(const cvf::Vec2ui& size);

    void            setAxisLabelsColor(const cvf::Color3f& color);

private:
    void render(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size, bool software);
    void renderAxisImmediateMode(cvf::OpenGLContext* oglContext);

private:
    cvf::Color3f        m_textColor;    // Text color 
    cvf::ref<cvf::Font> m_font;

    cvf::Vec2ui         m_size;         // Pixel size of draw area
};

