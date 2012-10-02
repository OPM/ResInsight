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

namespace cvf {



//==================================================================================================
//
// Controls OpenGL linewidth and aliasing, glLineWidth() and glEnable()/glDisable() with GL_LINE SMOOTH
//
//==================================================================================================
class RenderStateLine : public RenderState
{
public:
    RenderStateLine(float lineWidth = 1.0f);

    void            setWidth(float lineWidth);
    float           width() const;
    void            enableSmooth(bool enableSmooth);
    bool            isSmoothEnabled() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    float   m_lineWidth;
    bool    m_smooth;
};


}  // namespace cvf
