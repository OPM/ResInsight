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


#include "cvfBase.h"
#include "cvfString.h"
#include "cvfShaderSourceRepository.h"
#include "cvfShaderSourceStrings.h"

#include <fstream>

namespace cvf {


//==================================================================================================
///
/// \class cvf::ShaderSourceRepository
/// \ingroup Render
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ShaderSourceRepository::ShaderSourceRepository()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ShaderSourceRepository::~ShaderSourceRepository()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ShaderSourceRepository::shaderSource(ShaderIdent shaderIdent)
{
    String shaderProg;

    CharArray rawSource;
    if (rawShaderSource(shaderIdent, &rawSource))
    {
        if (rawSource.size() > 0)
        {
#ifdef CVF_OPENGL_ES
            // Always version 100 on OpenGL ES
            shaderProg = "#version 100\nprecision highp float;\n";
#else
            // Default on desktop is GLSL 1.2 (OpenGL 2.1) unless the shader explicitly specifies a version
            if (rawSource[0] != '#')
            {
                shaderProg = "#version 120\n";
            }
#endif
    
            shaderProg += rawSource.ptr();
        }
    }

    return shaderProg;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char* ShaderSourceRepository::shaderIdentString(ShaderIdent shaderIdent) 
{
    #define CVF_IDENT_HANDLE_CASE(THE_ENUM) case THE_ENUM:  return #THE_ENUM;

    switch (shaderIdent)
    {
        CVF_IDENT_HANDLE_CASE(calcClipDistances);
        CVF_IDENT_HANDLE_CASE(calcShadowCoord);

        CVF_IDENT_HANDLE_CASE(src_Color);
        CVF_IDENT_HANDLE_CASE(src_TwoSidedColor);
        CVF_IDENT_HANDLE_CASE(src_Texture);
        CVF_IDENT_HANDLE_CASE(src_TextureGlobalAlpha);
        CVF_IDENT_HANDLE_CASE(src_TextureFromPointCoord);
        CVF_IDENT_HANDLE_CASE(src_TextureRectFromFragCoord_v33);
        CVF_IDENT_HANDLE_CASE(src_VaryingColorGlobalAlpha);
        
        CVF_IDENT_HANDLE_CASE(light_Phong);
        CVF_IDENT_HANDLE_CASE(light_PhongDual);
        CVF_IDENT_HANDLE_CASE(light_SimpleHeadlight);
        CVF_IDENT_HANDLE_CASE(light_Headlight);
        
        CVF_IDENT_HANDLE_CASE(checkDiscard_ClipDistances);

        CVF_IDENT_HANDLE_CASE(vs_Standard);
        CVF_IDENT_HANDLE_CASE(vs_EnvironmentMapping);
        CVF_IDENT_HANDLE_CASE(vs_FullScreenQuad);
        CVF_IDENT_HANDLE_CASE(vs_Minimal);
        CVF_IDENT_HANDLE_CASE(vs_MinimalTexture);
        CVF_IDENT_HANDLE_CASE(vs_VectorDrawer);
        CVF_IDENT_HANDLE_CASE(vs_DistanceScaledPoints);
        CVF_IDENT_HANDLE_CASE(vs_ParticleTraceComets);
        
        CVF_IDENT_HANDLE_CASE(fs_Standard);
        CVF_IDENT_HANDLE_CASE(fs_Shadow_v33);
        CVF_IDENT_HANDLE_CASE(fs_Unlit);
        CVF_IDENT_HANDLE_CASE(fs_Void);
        CVF_IDENT_HANDLE_CASE(fs_FixedColorMagenta);
        CVF_IDENT_HANDLE_CASE(fs_Text);
        CVF_IDENT_HANDLE_CASE(fs_VectorDrawer);
        CVF_IDENT_HANDLE_CASE(fs_CenterLitSpherePoints);
        CVF_IDENT_HANDLE_CASE(fs_ParticleTraceComets);
        CVF_IDENT_HANDLE_CASE(fs_GradientTopBottom);
        CVF_IDENT_HANDLE_CASE(fs_GradientTopMiddleBottom);
        CVF_IDENT_HANDLE_CASE(fs_HighlightStencilBlur_v33);
        CVF_IDENT_HANDLE_CASE(fs_HighlightStencilDraw);
        CVF_IDENT_HANDLE_CASE(fs_HighlightStencilMix_v33);
        CVF_IDENT_HANDLE_CASE(fs_GaussianBlur);
        CVF_IDENT_HANDLE_CASE(fs_HighlightMix);
        
        CVF_IDENT_HANDLE_CASE(gs_PassThroughTriangle_v33);
    }

    CVF_FAIL_MSG("Unhandled shader ident");
    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderSourceRepository::rawShaderSource(ShaderIdent shaderIdent, CharArray* rawSource)
{
    #define CVF_SOURCE_HANDLE_CASE(THE_ENUM) case THE_ENUM:  *rawSource = CharArray(THE_ENUM##_inl); return true; 

    switch (shaderIdent)
    {
        CVF_SOURCE_HANDLE_CASE(calcClipDistances);
        CVF_SOURCE_HANDLE_CASE(calcShadowCoord);

        CVF_SOURCE_HANDLE_CASE(src_Color);
        CVF_SOURCE_HANDLE_CASE(src_TwoSidedColor);
        CVF_SOURCE_HANDLE_CASE(src_Texture);
        CVF_SOURCE_HANDLE_CASE(src_TextureGlobalAlpha);
        CVF_SOURCE_HANDLE_CASE(src_TextureFromPointCoord);
        CVF_SOURCE_HANDLE_CASE(src_TextureRectFromFragCoord_v33);
        CVF_SOURCE_HANDLE_CASE(src_VaryingColorGlobalAlpha);

        CVF_SOURCE_HANDLE_CASE(light_Phong);
        CVF_SOURCE_HANDLE_CASE(light_PhongDual);
        CVF_SOURCE_HANDLE_CASE(light_SimpleHeadlight);
        CVF_SOURCE_HANDLE_CASE(light_Headlight);

        CVF_SOURCE_HANDLE_CASE(checkDiscard_ClipDistances);

        CVF_SOURCE_HANDLE_CASE(vs_Standard);
        CVF_SOURCE_HANDLE_CASE(vs_EnvironmentMapping);
        CVF_SOURCE_HANDLE_CASE(vs_FullScreenQuad);
        CVF_SOURCE_HANDLE_CASE(vs_Minimal);
        CVF_SOURCE_HANDLE_CASE(vs_MinimalTexture);
        CVF_SOURCE_HANDLE_CASE(vs_VectorDrawer);
        CVF_SOURCE_HANDLE_CASE(vs_DistanceScaledPoints);
        CVF_SOURCE_HANDLE_CASE(vs_ParticleTraceComets);

        CVF_SOURCE_HANDLE_CASE(fs_Standard);
        CVF_SOURCE_HANDLE_CASE(fs_Shadow_v33);
        CVF_SOURCE_HANDLE_CASE(fs_Unlit);
        CVF_SOURCE_HANDLE_CASE(fs_Void);
        CVF_SOURCE_HANDLE_CASE(fs_FixedColorMagenta);
        CVF_SOURCE_HANDLE_CASE(fs_Text);
        CVF_SOURCE_HANDLE_CASE(fs_VectorDrawer);
        CVF_SOURCE_HANDLE_CASE(fs_CenterLitSpherePoints);
        CVF_SOURCE_HANDLE_CASE(fs_ParticleTraceComets);
        CVF_SOURCE_HANDLE_CASE(fs_GradientTopBottom);
        CVF_SOURCE_HANDLE_CASE(fs_GradientTopMiddleBottom);
        CVF_SOURCE_HANDLE_CASE(fs_HighlightStencilBlur_v33);
        CVF_SOURCE_HANDLE_CASE(fs_HighlightStencilDraw);
        CVF_SOURCE_HANDLE_CASE(fs_HighlightStencilMix_v33);
        CVF_SOURCE_HANDLE_CASE(fs_GaussianBlur);
        CVF_SOURCE_HANDLE_CASE(fs_HighlightMix);

        CVF_SOURCE_HANDLE_CASE(gs_PassThroughTriangle_v33);
    }

    CVF_FAIL_MSG("Unhandled shader source ident");
    return false;
}

} // namespace cvf
