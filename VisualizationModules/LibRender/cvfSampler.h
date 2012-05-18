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

#include "cvfObject.h"

namespace cvf {


//==================================================================================================
//
// OpenGL 3.3/4.1 settings:
// 
// Currently handled:
//   TEXTURE_WRAP_S
//   TEXTURE_WRAP_T
//   TEXTURE_MIN_FILTER
//   TEXTURE_MAG_FILTER
// 
// TBI:
//   TEXTURE_WRAP_R, TEXTURE_BORDER_COLOR, TEXTURE_MIN_LOD, TEXTURE_MAX_LOD, TEXTURE_LOD_BIAS, 
//   TEXTURE_COMPARE_MODE, TEXTURE_COMPARE_FUNC
//
//==================================================================================================
class Sampler : public Object
{
public:
    enum WrapMode
    {
        REPEAT,          // OpenGL default   
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
        MIRRORED_REPEAT
    };

    enum Filter
    {
        NEAREST, 
        LINEAR,                 // Default mag filter in OpenGL
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,  // Default min filter in OpenGL
        LINEAR_MIPMAP_LINEAR
    };

public:
    Sampler();

    void setWrapMode(WrapMode wrapMode);
    void setWrapModeS(WrapMode wrapMode);
    void setWrapModeT(WrapMode wrapMode);
    void setMinFilter(Filter minFilter);
    void setMagFilter(Filter magFilter);

    WrapMode    wrapModeS() const;
    WrapMode    wrapModeT() const;
    Filter      minFilter() const;
    Filter      magFilter() const;

private:
    WrapMode    m_wrapModeS;
    WrapMode    m_wrapModeT;
    Filter      m_minFilter;
    Filter      m_magFilter;
};

}  // namespace cvf
