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
#include "cvfOpenGL.h"
#include "cvfColor4.h"
#include "cvfMath.h"
#include "cvfRenderState_FF.h"
#include "cvfTexture2D_FF.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateLighting_FF
/// \ingroup Render 
///
/// Encapsulate OpenGL glLightModel() and glEnable()/glDisable() with GL_LIGHTING
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glLightModel.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateLighting_FF::RenderStateLighting_FF(bool enableLighting)
:   RenderState(LIGHTING_FF),
    m_enableLighting(enableLighting),
    m_twoSided(false),
    m_localViewer(false),
    m_ambientIntensity(0.2f, 0.2f, 0.2f, 1.0f)
{
}


//--------------------------------------------------------------------------------------------------
/// Enables or disables fixed function lighting calculations
/// 
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml with GL_LIGHTING
//--------------------------------------------------------------------------------------------------
void RenderStateLighting_FF::enable(bool enableLighting)
{
    m_enableLighting = enableLighting;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateLighting_FF::enableTwoSided(bool enableTwoSided)
{
    m_twoSided = enableTwoSided;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateLighting_FF::enableLocalViewer(bool enableLocalViewer)
{
    m_localViewer = enableLocalViewer;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateLighting_FF::setAmbientIntensity(const Color3f& ambientIntensity)
{
    m_ambientIntensity.set(ambientIntensity, 1.0f);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RenderStateLighting_FF::isEnabled() const
{
    return m_enableLighting;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateLighting_FF::isTwoSidedEnabled() const
{
    return m_twoSided;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateLighting_FF::isLocalViewerEnabled() const
{
    return m_localViewer;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f RenderStateLighting_FF::ambientIntensity() const
{
    return m_ambientIntensity.toColor3f();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateLighting_FF::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enableLighting)  
    {
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, m_twoSided ? 1 : 0);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,  m_localViewer ? 1 : 0);

        CVF_ASSERT(m_ambientIntensity.isValid());
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, m_ambientIntensity.ptr());

        glEnable(GL_LIGHTING);
    }
    else
    {
        glDisable(GL_LIGHTING);
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateLighting_FF::isFixedFunction() const
{
    return true;
}



//==================================================================================================
///
/// \class cvf::RenderStateMaterial_FF
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor, initializes all values to OpenGL defaults
//--------------------------------------------------------------------------------------------------
RenderStateMaterial_FF::RenderStateMaterial_FF()
:   RenderState(MATERIAL_FF),
    m_ambient(0.2f, 0.2f, 0.2f),
    m_diffuse(0.8f, 0.8f, 0.8f),
    m_specular(0, 0, 0),
    m_emission(0, 0, 0),
    m_alpha(1.0f),
    m_shininess(0),
    m_enableColorMaterial(false)
{
}


//--------------------------------------------------------------------------------------------------
/// Constructor taking ambient and diffuse color
//--------------------------------------------------------------------------------------------------
RenderStateMaterial_FF::RenderStateMaterial_FF(const Color3f& ambientAndDiffuseColor)
:   RenderState(MATERIAL_FF),
    m_ambient(ambientAndDiffuseColor),
    m_diffuse(ambientAndDiffuseColor),
    m_specular(0, 0, 0),
    m_emission(0, 0, 0),
    m_alpha(1.0f),
    m_shininess(0),
    m_enableColorMaterial(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateMaterial_FF::RenderStateMaterial_FF(MaterialIdent materialIdent)
:   RenderState(MATERIAL_FF),
    m_specular(0, 0, 0),
    m_emission(0, 0, 0),
    m_alpha(1.0f),
    m_shininess(0),
    m_enableColorMaterial(false)
{
    switch (materialIdent)
    {
        case PURE_WHITE:		m_ambient.set(1.0f, 1.0f, 1.0f);  m_diffuse.set(1.0f, 1.0f, 1.0f);  break;
        case PURE_BLACK:		m_ambient.set(0.0f, 0.0f, 0.0f);  m_diffuse.set(0.0f, 0.0f, 0.0f);  break;
        case PURE_RED:		    m_ambient.set(1.0f, 0.0f, 0.0f);  m_diffuse.set(1.0f, 0.0f, 0.0f);  break;
        case PURE_GREEN:		m_ambient.set(0.0f, 1.0f, 0.0f);  m_diffuse.set(0.0f, 1.0f, 0.0f);  break;
        case PURE_BLUE:		    m_ambient.set(0.0f, 0.0f, 1.0f);  m_diffuse.set(0.0f, 0.0f, 1.0f);  break;
        case PURE_YELLOW:	    m_ambient.set(1.0f, 1.0f, 0.0f);  m_diffuse.set(1.0f, 1.0f, 0.0f);  break;
        case PURE_MAGENTA:	    m_ambient.set(1.0f, 0.0f, 1.0f);  m_diffuse.set(1.0f, 0.0f, 1.0f);  break;
        case PURE_CYAN:		    m_ambient.set(0.0f, 1.0f, 1.0f);  m_diffuse.set(0.0f, 1.0f, 1.0f);  break;

        case BRASS:             m_ambient.set(0.329412f, 0.223529f, 0.027451f);  m_diffuse.set(0.780392f, 0.568627f, 0.113725f);  m_specular.set(0.992157f, 0.941176f, 0.807843f);  m_alpha = 1.00f;  m_shininess = 27.8974f;  break;
        case BRONZE:            m_ambient.set(0.212500f, 0.127500f, 0.054000f);  m_diffuse.set(0.714000f, 0.428400f, 0.181440f);  m_specular.set(0.393548f, 0.271906f, 0.166721f);  m_alpha = 1.00f;  m_shininess = 25.6000f;  break;
        case POLISHED_BRONZE:   m_ambient.set(0.250000f, 0.148000f, 0.064750f);  m_diffuse.set(0.400000f, 0.236800f, 0.103600f);  m_specular.set(0.774597f, 0.458561f, 0.200621f);  m_alpha = 1.00f;  m_shininess = 76.8000f;  break;
        case CHROME:            m_ambient.set(0.250000f, 0.250000f, 0.250000f);  m_diffuse.set(0.400000f, 0.400000f, 0.400000f);  m_specular.set(0.774597f, 0.774597f, 0.774597f);  m_alpha = 1.00f;  m_shininess = 76.8000f;  break;
        case COPPER:            m_ambient.set(0.191250f, 0.073500f, 0.022500f);  m_diffuse.set(0.703800f, 0.270480f, 0.082800f);  m_specular.set(0.256777f, 0.137622f, 0.086014f);  m_alpha = 1.00f;  m_shininess = 12.8000f;  break;
        case POLISHED_COPPER:   m_ambient.set(0.229500f, 0.088250f, 0.027500f);  m_diffuse.set(0.550800f, 0.211800f, 0.066000f);  m_specular.set(0.580594f, 0.223257f, 0.069570f);  m_alpha = 1.00f;  m_shininess = 51.2000f;  break;
        case GOLD:              m_ambient.set(0.247250f, 0.199500f, 0.074500f);  m_diffuse.set(0.751640f, 0.606480f, 0.226480f);  m_specular.set(0.628281f, 0.555802f, 0.366065f);  m_alpha = 1.00f;  m_shininess = 51.2000f;  break;
        case POLISHED_GOLD:     m_ambient.set(0.247250f, 0.224500f, 0.064500f);  m_diffuse.set(0.346150f, 0.314300f, 0.090300f);  m_specular.set(0.797357f, 0.723991f, 0.208006f);  m_alpha = 1.00f;  m_shininess = 83.2000f;  break;
        case PEWTER:            m_ambient.set(0.105882f, 0.058824f, 0.113725f);  m_diffuse.set(0.427451f, 0.470588f, 0.541176f);  m_specular.set(0.333333f, 0.333333f, 0.521569f);  m_alpha = 1.00f;  m_shininess =  9.8462f;  break;
        case SILVER:            m_ambient.set(0.192250f, 0.192250f, 0.192250f);  m_diffuse.set(0.507540f, 0.507540f, 0.507540f);  m_specular.set(0.508273f, 0.508273f, 0.508273f);  m_alpha = 1.00f;  m_shininess = 51.2000f;  break;
        case POLISHED_SILVER:   m_ambient.set(0.231250f, 0.231250f, 0.231250f);  m_diffuse.set(0.277500f, 0.277500f, 0.277500f);  m_specular.set(0.773911f, 0.773911f, 0.773911f);  m_alpha = 1.00f;  m_shininess = 89.6000f;  break;
        case EMERALD:           m_ambient.set(0.021500f, 0.174500f, 0.021500f);  m_diffuse.set(0.075680f, 0.614240f, 0.075680f);  m_specular.set(0.633000f, 0.727811f, 0.633000f);  m_alpha = 0.55f;  m_shininess = 76.8000f;  break;
        case JADE:              m_ambient.set(0.135000f, 0.222500f, 0.157500f);  m_diffuse.set(0.540000f, 0.890000f, 0.630000f);  m_specular.set(0.316228f, 0.316228f, 0.316228f);  m_alpha = 0.95f;  m_shininess = 12.8000f;  break;
        case OBSIDIAN:          m_ambient.set(0.053750f, 0.050000f, 0.066250f);  m_diffuse.set(0.182750f, 0.170000f, 0.225250f);  m_specular.set(0.332741f, 0.328634f, 0.346435f);  m_alpha = 0.82f;  m_shininess = 38.4000f;  break;
        case PEARL:             m_ambient.set(0.250000f, 0.207250f, 0.207250f);  m_diffuse.set(1.000000f, 0.829000f, 0.829000f);  m_specular.set(0.296648f, 0.296648f, 0.296648f);  m_alpha = 0.92f;  m_shininess = 11.2640f;  break;
        case RUBY:              m_ambient.set(0.174500f, 0.011750f, 0.011750f);  m_diffuse.set(0.614240f, 0.041360f, 0.041360f);  m_specular.set(0.727811f, 0.626959f, 0.626959f);  m_alpha = 0.55f;  m_shininess = 76.8000f;  break;
        case TURQUOISE:         m_ambient.set(0.100000f, 0.187250f, 0.174500f);  m_diffuse.set(0.396000f, 0.741510f, 0.691020f);  m_specular.set(0.297254f, 0.308290f, 0.306678f);  m_alpha = 0.80f;  m_shininess = 12.8000f;  break;

        case BLACK_PLASTIC:     m_ambient.set(0.000000f, 0.000000f, 0.000000f);  m_diffuse.set(0.010000f, 0.010000f, 0.010000f);  m_specular.set(0.500000f, 0.500000f, 0.500000f);  m_alpha = 1.00f;  m_shininess = 32.0000f;  break;
        case CYAN_PLASTIC:      m_ambient.set(0.000000f, 0.100000f, 0.060000f);  m_diffuse.set(0.000000f, 0.509804f, 0.509804f);  m_specular.set(0.501961f, 0.501961f, 0.501961f);  m_alpha = 1.00f;  m_shininess = 32.0000f;  break;
        case GREEN_PLASTIC:     m_ambient.set(0.000000f, 0.000000f, 0.000000f);  m_diffuse.set(0.100000f, 0.350000f, 0.100000f);  m_specular.set(0.450000f, 0.550000f, 0.450000f);  m_alpha = 1.00f;  m_shininess = 32.0000f;  break;
        case RED_PLASTIC:       m_ambient.set(0.000000f, 0.000000f, 0.000000f);  m_diffuse.set(0.500000f, 0.000000f, 0.000000f);  m_specular.set(0.700000f, 0.600000f, 0.600000f);  m_alpha = 1.00f;  m_shininess = 32.0000f;  break;
        case WHITE_PLASTIC:     m_ambient.set(0.000000f, 0.000000f, 0.000000f);  m_diffuse.set(0.550000f, 0.550000f, 0.550000f);  m_specular.set(0.700000f, 0.700000f, 0.700000f);  m_alpha = 1.00f;  m_shininess = 32.0000f;  break;
        case YELLOW_PLASTIC:    m_ambient.set(0.000000f, 0.000000f, 0.000000f);  m_diffuse.set(0.500000f, 0.500000f, 0.000000f);  m_specular.set(0.600000f, 0.600000f, 0.500000f);  m_alpha = 1.00f;  m_shininess = 32.0000f;  break;
        case BLACK_RUBBER:      m_ambient.set(0.020000f, 0.020000f, 0.020000f);  m_diffuse.set(0.010000f, 0.010000f, 0.010000f);  m_specular.set(0.400000f, 0.400000f, 0.400000f);  m_alpha = 1.00f;  m_shininess = 10.0000f;  break;
        case CYAN_RUBBER:       m_ambient.set(0.000000f, 0.050000f, 0.050000f);  m_diffuse.set(0.400000f, 0.500000f, 0.500000f);  m_specular.set(0.040000f, 0.700000f, 0.700000f);  m_alpha = 1.00f;  m_shininess = 10.0000f;  break;
        case GREEN_RUBBER:      m_ambient.set(0.000000f, 0.050000f, 0.000000f);  m_diffuse.set(0.400000f, 0.500000f, 0.400000f);  m_specular.set(0.040000f, 0.700000f, 0.040000f);  m_alpha = 1.00f;  m_shininess = 10.0000f;  break;
        case RED_RUBBER:        m_ambient.set(0.050000f, 0.000000f, 0.000000f);  m_diffuse.set(0.500000f, 0.400000f, 0.400000f);  m_specular.set(0.700000f, 0.040000f, 0.040000f);  m_alpha = 1.00f;  m_shininess = 10.0000f;  break;
        case WHITE_RUBBER:      m_ambient.set(0.050000f, 0.050000f, 0.050000f);  m_diffuse.set(0.500000f, 0.500000f, 0.500000f);  m_specular.set(0.700000f, 0.700000f, 0.700000f);  m_alpha = 1.00f;  m_shininess = 10.0000f;  break;
        case YELLOW_RUBBER:     m_ambient.set(0.050000f, 0.050000f, 0.000000f);  m_diffuse.set(0.500000f, 0.500000f, 0.400000f);  m_specular.set(0.700000f, 0.700000f, 0.040000f);  m_alpha = 1.00f;  m_shininess = 10.0000f;  break;

        default:                CVF_FAIL_MSG("Unhandled MaterialIdent");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::setAmbientAndDiffuse(const Color3f& color)
{
    CVF_ASSERT(color.isValid());

    m_ambient = color;
    m_diffuse = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::setDiffuse(const Color3f& color)
{
    CVF_ASSERT(color.isValid());

    m_diffuse = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::setSpecular(const Color3f& color)
{
    CVF_ASSERT(color.isValid());

    m_specular = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::setEmission(const Color3f& color)
{
    CVF_ASSERT(color.isValid());

    m_emission = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::setAlpha(float alpha)
{
    m_alpha = alpha;
}


//--------------------------------------------------------------------------------------------------
/// Value that specifies the RGBA specular exponent of the material. 
/// Only values in the range 0 128 are accepted. The initial specular exponent for 
/// both front- and back-facing materials is 0
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::setShininess(float shininess)
{
    m_shininess = shininess;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f RenderStateMaterial_FF::frontAmbient() const
{
    return m_ambient;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f RenderStateMaterial_FF::frontDiffuse() const
{
    return m_diffuse;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f RenderStateMaterial_FF::frontSpecular() const
{
    return m_specular;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f RenderStateMaterial_FF::frontEmission() const
{
    return m_emission;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RenderStateMaterial_FF::frontAlpha() const
{
    return m_alpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RenderStateMaterial_FF::frontShininess() const
{
    return m_shininess;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::enableColorMaterial(bool enableColorMaterial)
{
    m_enableColorMaterial = enableColorMaterial;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateMaterial_FF::isColorMaterialEnabled() const
{
    return m_enableColorMaterial;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateMaterial_FF::applyOpenGL(OpenGLContext* oglContext) const
{
    Color4f ambient(m_ambient, m_alpha);
    Color4f diffuse(m_diffuse, m_alpha);
    Color4f emission(m_emission, m_alpha);
    Color4f specular(m_specular, m_alpha);
    
    CVF_ASSERT(ambient.isValid());
    CVF_ASSERT(diffuse.isValid());
    CVF_ASSERT(emission.isValid());
    CVF_ASSERT(specular.isValid());
    CVF_ASSERT(Math::valueInRange(m_shininess, 0.0f, 128.0f));
    
    if (m_enableColorMaterial)
    {
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
    }
    else
    {
        glDisable(GL_COLOR_MATERIAL);
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.ptr());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.ptr());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.ptr());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission.ptr());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_shininess);

    // Try this to be nice if lighting is off
    glColor3fv(diffuse.ptr());

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateMaterial_FF::isFixedFunction() const
{
    return true;
}




//==================================================================================================
///
/// \class cvf::RenderStateNormalize_FF
/// \ingroup Render
///
/// Controls normalization of normals in fixed function
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateNormalize_FF::RenderStateNormalize_FF(bool enableNormalization)
:   RenderState(NORMALIZE_FF),
    m_enable(enableNormalization)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateNormalize_FF::enable(bool enableNormalization)
{
    m_enable = enableNormalization;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateNormalize_FF::isEnabled() const
{
    return m_enable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateNormalize_FF::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enable)
    {
        glEnable(GL_NORMALIZE);
    }
    else
    {
        glEnable(GL_NORMALIZE);
    }

    CVF_CHECK_OGL(oglContext);
}



//==================================================================================================
///
/// \class cvf::RenderStateTextureMapping_FF
/// \ingroup Render
///
/// Enable 2D texturing for texture unit 0 and fixed function
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateTextureMapping_FF::RenderStateTextureMapping_FF(Texture2D_FF* texture)
:   RenderState(TEXTURE_MAPPING_FF),
    m_texture(texture),
    m_textureFunction(MODULATE),
    m_environmentMapping(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateTextureMapping_FF::~RenderStateTextureMapping_FF()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTextureMapping_FF::setTexture(Texture2D_FF* texture)
{
    m_texture = texture;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture2D_FF* RenderStateTextureMapping_FF::texture()
{
    return m_texture.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTextureMapping_FF::setTextureFunction(TextureFunction texFunc)
{
    m_textureFunction = texFunc;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateTextureMapping_FF::TextureFunction RenderStateTextureMapping_FF::textureFunction() const
{
    return m_textureFunction;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTextureMapping_FF::setEnvironmentMapping(bool environmentMapping)
{
    m_environmentMapping = environmentMapping;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateTextureMapping_FF::environmentMapping() const
{
    return m_environmentMapping;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTextureMapping_FF::setupTexture(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (m_texture.notNull())
    {
        if (m_texture->textureOglId() == 0)
        {
            if (oglContext->capabilities()->supportsOpenGL2())
            {
                glActiveTexture(GL_TEXTURE0);
            }

            m_texture->setupTexture(oglContext);

            CVF_CHECK_OGL(oglContext);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTextureMapping_FF::applyOpenGL(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (oglContext->capabilities()->supportsOpenGL2())
    {
        glActiveTexture(GL_TEXTURE0);
    }

    if (m_texture.notNull() && m_texture->textureOglId() != 0)
    {
        m_texture->bind(oglContext);
        m_texture->setupTextureParams(oglContext);

        cvfGLint oglTexFunc = GL_MODULATE;
        switch (m_textureFunction)
        {
            case MODULATE:  oglTexFunc = GL_MODULATE; break;
            case DECAL:     oglTexFunc = GL_DECAL; break;
        }

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, oglTexFunc);

        glEnable(GL_TEXTURE_2D);

        if (m_environmentMapping)
        {
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);

            GLint piSphereMap[1];
            piSphereMap[0] = GL_SPHERE_MAP;
            glTexGeniv(GL_S, GL_TEXTURE_GEN_MODE, piSphereMap);
            glTexGeniv(GL_T, GL_TEXTURE_GEN_MODE, piSphereMap);
        }
        else
        {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateTextureMapping_FF::isFixedFunction() const
{
    return true;
}

//==================================================================================================
///
/// \class cvf::RenderStateClipPlanes_FF
/// \ingroup Render 
///
/// Encapsulate OpenGL glClipPlane() and glEnable()/glDisable() with GL_CLIP_PLANE0 + idx
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateClipPlanes_FF::RenderStateClipPlanes_FF()
:   RenderState(CLIP_PLANES_FF)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateClipPlanes_FF::addPlane(const cvf::Plane& plane)
{
    m_clipPlanes.push_back(plane);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RenderStateClipPlanes_FF::planeCount() const
{
    return m_clipPlanes.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Plane& RenderStateClipPlanes_FF::plane(size_t index)
{
    CVF_ASSERT(index < m_clipPlanes.size());
    return m_clipPlanes[index];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateClipPlanes_FF::removeAllPlanes()
{
    m_clipPlanes.clear();
}


//--------------------------------------------------------------------------------------------------
/// Supports 6 clipping planes, as all implementations of OpenGL is required to have at least 6
//--------------------------------------------------------------------------------------------------
void RenderStateClipPlanes_FF::applyOpenGL(OpenGLContext* oglContext) const
{
    for (size_t idx = 0; idx < m_clipPlanes.size(); idx++)
    {
        if (idx > 5)
        {
            break;
        }

        // Set and enable plane
        GLdouble plane[4];

        plane[0] = m_clipPlanes[idx].A();
        plane[1] = m_clipPlanes[idx].B();
        plane[2] = m_clipPlanes[idx].C();
        plane[3] = m_clipPlanes[idx].D();

        glClipPlane((GLenum)(GL_CLIP_PLANE0 + idx), plane);
        glEnable((GLenum)(GL_CLIP_PLANE0 + idx));
    }

    for (size_t idx = m_clipPlanes.size(); idx < 6; idx++)
    {
        glDisable((GLenum)(GL_CLIP_PLANE0 + idx));
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateClipPlanes_FF::isFixedFunction() const
{
    return true;
}

} // namespace cvf
