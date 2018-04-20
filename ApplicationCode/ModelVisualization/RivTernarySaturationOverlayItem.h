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

#include "cafTitledOverlayFrame.h"
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
class RivTernarySaturationOverlayItem : public caf::TitledOverlayFrame
{
public:
    explicit RivTernarySaturationOverlayItem(cvf::Font* font);
    ~RivTernarySaturationOverlayItem();
    virtual void    computeLayoutAndExtents(const cvf::Vec2ui& size) override;

    void setRangeText(const cvf::String& soilRange, const cvf::String& sgasRange, const cvf::String& swatRange);

    void            setAxisLabelsColor(const cvf::Color3f& color);

private:
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
    cvf::String         m_soilRange;
    cvf::String         m_sgasRange;
    cvf::String         m_swatRange;
};

