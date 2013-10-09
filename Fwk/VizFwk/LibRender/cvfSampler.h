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
        REPEAT,             // OpenGL default   
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
        MIRRORED_REPEAT
    };

    enum Filter
    {
        NEAREST,                ///< Nearest neighbor filtering on the base mip level
        LINEAR,                 ///< Linear filtering on the base mip level. (Default magnification filter in OpenGL)
        NEAREST_MIPMAP_NEAREST, ///< Selects nearest mip level and performs nearest neighbor filtering
        LINEAR_MIPMAP_NEAREST,  ///< Selects nearest mip level and performs linear filtering
        NEAREST_MIPMAP_LINEAR,  ///< Perform linear interpolation between mip levels and perform nearest neighbor filtering. (Default minifying filter in OpenGL)
        LINEAR_MIPMAP_LINEAR    ///< Perform linear interpolation between mip levels and perform linear filtering (trilinear mipmapping)
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
    WrapMode    m_wrapModeS;    ///< Wrap mode for texture coordinate S. Default is REPEAT
    WrapMode    m_wrapModeT;    ///< Wrap mode for texture coordinate T. Default is REPEAT
    Filter      m_minFilter;    ///< Minifying function. Default is NEAREST_MIPMAP_LINEAR
    Filter      m_magFilter;    ///< Magnification function. Default is LINEAR
};

}  // namespace cvf
