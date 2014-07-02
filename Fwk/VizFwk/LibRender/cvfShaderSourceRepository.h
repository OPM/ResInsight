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
#include "cvfString.h"

namespace cvf {

class CharArray;


//==================================================================================================
//
// 
//
//==================================================================================================
class ShaderSourceRepository : public Object
{
public:
    enum ShaderIdent
    {
        calcClipDistances,
        calcShadowCoord,

        src_Color,
        src_TwoSidedColor,
        src_Texture,
        src_TextureGlobalAlpha,
        src_TextureFromPointCoord,
        src_TextureRectFromFragCoord_v33,
        src_VaryingColorGlobalAlpha,

        light_Phong,
        light_PhongDual,
        light_SimpleHeadlight,
        light_Headlight,

        checkDiscard_ClipDistances,

        vs_Standard,
        vs_EnvironmentMapping,
        vs_FullScreenQuad,
        vs_Minimal,
        vs_MinimalTexture,
        vs_VectorDrawer,
        vs_DistanceScaledPoints,
        vs_ParticleTraceComets,

        fs_Standard,
        fs_Shadow_v33,
        fs_Unlit,
        fs_Void,
        fs_FixedColorMagenta,
        fs_Text,
        fs_VectorDrawer,
        fs_CenterLitSpherePoints,
        fs_ParticleTraceComets,
        fs_GradientTopBottom,
        fs_GradientTopMiddleBottom,
        fs_HighlightStencilBlur_v33,
        fs_HighlightStencilDraw,
        fs_HighlightStencilMix_v33,
        fs_GaussianBlur,
        fs_HighlightMix,

        gs_PassThroughTriangle_v33
    };

public:
    ShaderSourceRepository();
    virtual ~ShaderSourceRepository();

    String              shaderSource(ShaderIdent shaderIdent);
    static const char*  shaderIdentString(ShaderIdent shaderIdent);

protected:
    virtual bool    rawShaderSource(ShaderIdent shaderIdent, CharArray* rawSource);
};

}
