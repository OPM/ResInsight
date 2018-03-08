/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
#include "cvfColor4.h"
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
    explicit RivTernarySaturationOverlayItem(cvf::Font* font);
    ~RivTernarySaturationOverlayItem();

    void setRangeText(const cvf::String& soilRange, const cvf::String& sgasRange, const cvf::String& swatRange);

    void            setSize(const cvf::Vec2ui& size);
    void            setAxisLabelsColor(const cvf::Color3f& color);
    void            setTitle(const cvf::String& title);

    void            enableBackground(bool enable);
    void            setBackgroundColor(const cvf::Color4f& backgroundColor); 
    void            setBackgroundFrameColor(const cvf::Color4f& backgroundFrameColor);

private:
    cvf::Vec2ui     sizeHint() override;
    void            render(cvf::OpenGLContext* oglContext, 
                           const cvf::Vec2i& position, 
                           const cvf::Vec2ui& size) override;
    void            renderSoftware(cvf::OpenGLContext* oglContext, 
                                   const cvf::Vec2i& position, 
                                   const cvf::Vec2ui& size) override;

    void            renderGeneric(cvf::OpenGLContext* oglContext, 
                                  const cvf::Vec2i& position, 
                                  const cvf::Vec2ui& size, 
                                  bool software);
    void            renderAxisImmediateMode(float upperY, float lowerBoundY, float border,  cvf::OpenGLContext* oglContext);

private:
    cvf::Color3f        m_textColor;    // Text color 
    cvf::ref<cvf::Font> m_font;
    
    bool                m_isBackgroundEnabled;
    cvf::Color4f        m_backgroundColor;
    cvf::Color4f        m_backgroundFrameColor;

    cvf::String         m_soilRange;
    cvf::String         m_sgasRange;
    cvf::String         m_swatRange;

    cvf::Vec2ui         m_size;         // Pixel size of draw area

    std::vector<cvf::String> m_titleStrings;
};

