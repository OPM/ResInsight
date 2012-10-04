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

#pragma once

#include "cvfRenderState.h"
#include "cvfOpenGLTypes.h"
#include "cvfColor4.h"

namespace cvf {



//==================================================================================================
//
// Encapsulate OpenGL glPolygonOffset() and glEnable()/glDisable() with GL_POLYGON_OFFSET_FILL/LINE/POINT
//
//==================================================================================================
class RenderStatePolygonOffset : public RenderState
{
public:
    RenderStatePolygonOffset();

    void            enableFillMode(bool enableFill);
    void            enableLineMode(bool enableLine);
    void            enablePointMode(bool enablePoint);
    bool            isFillModeEnabled() const;
    bool            isLineModeEnabled() const;
    bool            isPointModeEnabled() const;

    void            setFactor(float factor);
    void            setUnits(float units);
    float           factor() const;
    float           units() const;

    void            configurePolygonPositiveOffset();
    void            configureLineNegativeOffset();

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    float   m_factor;               // Default value is 0.0
    float   m_units;                // Default value is 0.0
    bool    m_enableFillMode;
    bool    m_enableLineMode;
    bool    m_enablePointMode;
};


}  // namespace cvf
