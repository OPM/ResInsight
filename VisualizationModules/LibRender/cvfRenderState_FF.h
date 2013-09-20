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

#include "cvfRenderState.h"
#include "cvfColor4.h"
#include "cvfPlane.h"

namespace cvf {

class Texture2D_FF;


//==================================================================================================
//
// Encapsulates  OpenGL's glLightModel() and glEnable()/glDisable() with GL_LIGHTING
//
//==================================================================================================
class RenderStateLighting_FF : public RenderState
{
public:
    RenderStateLighting_FF(bool enableLighting = true);

    void            enable(bool enableLighting);
    bool            isEnabled() const;

    void            enableTwoSided(bool enableTwoSided);
    void            enableLocalViewer(bool enableLocalViewer);
    void            setAmbientIntensity(const Color3f& ambientIntensity);

    bool            isTwoSidedEnabled() const;
    bool            isLocalViewerEnabled() const;
    Color3f         ambientIntensity() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;
    virtual bool    isFixedFunction() const;

private:
    bool    m_enableLighting;   // Master enable/disable switch for fixed function OpenGL lighting
    bool	m_twoSided;	        // Two sided lighting
    bool	m_localViewer;		// Determines how specular reflection angles are computed
    Color4f m_ambientIntensity; // The global ambient light intensity
};



//==================================================================================================
//
// Encapsulates OpenGL glMaterial() state
//
//==================================================================================================
class RenderStateMaterial_FF : public RenderState
{
public:
    enum MaterialIdent
    {
        // Simple materials with just ambient and diffuse set. All other parameters are left at defaults
        PURE_WHITE,	        ///< White material
        PURE_BLACK,	        ///< Black material
        PURE_RED,		    ///< Red material
        PURE_GREEN,	        ///< Green material
        PURE_BLUE,	        ///< Blue material
        PURE_YELLOW,	    ///< Yellow material
        PURE_MAGENTA,	    ///< Magenta material
        PURE_CYAN,	        ///< Cyan material

        // Parameters for common materials, based on table presented in SIGGRAPH 99, Lighting and Shading Techniques for Interactive Applications.
        // Sets specular and shininess in addition to ambient and diffuse, and may also include transparency (alpha value)
        BRASS,			    ///< Brass			
        BRONZE,			    ///< Bronze			
        POLISHED_BRONZE,    ///< Polished Bronze	
        CHROME,			    ///< Chrome			
        COPPER,			    ///< Copper			
        POLISHED_COPPER,	///< Polished Copper	
        GOLD,				///< Gold			
        POLISHED_GOLD,	    ///< Polished Gold	
        PEWTER,			    ///< Pewter			
        SILVER,			    ///< Silver			
        POLISHED_SILVER,	///< Polished Silver	
        EMERALD,			///< Emerald			
        JADE,				///< Jade			
        OBSIDIAN,			///< Obsidian		
        PEARL,			    ///< Pearl			
        RUBY,				///< Ruby			
        TURQUOISE,		    ///< Turquoise		
        BLACK_PLASTIC, 	    ///< Black Plastic	
        CYAN_PLASTIC, 	    ///< Cyan Plastic 
        GREEN_PLASTIC, 	    ///< Green Plastic
        RED_PLASTIC, 		///< Red Plastic 
        WHITE_PLASTIC, 	    ///< White Plastic 
        YELLOW_PLASTIC,	    ///< Yellow Plastic 
        BLACK_RUBBER, 	    ///< Black Rubber 
        CYAN_RUBBER, 		///< Cyan Rubber 
        GREEN_RUBBER, 	    ///< Green Rubber 
        RED_RUBBER, 		///< Red Rubber
        WHITE_RUBBER, 	    ///< White Rubber
        YELLOW_RUBBER 	    ///< Yellow Rubber
    };

public:
    RenderStateMaterial_FF();
    explicit RenderStateMaterial_FF(const Color3f& ambientAndDiffuseColor);
    explicit RenderStateMaterial_FF(MaterialIdent materialIdent);

    void            setAmbientAndDiffuse(const Color3f& color);
    void            setDiffuse(const Color3f& color);
    void            setSpecular(const Color3f& color);
    void            setEmission(const Color3f& color);
    void            setAlpha(float alpha);
    void            setShininess(float shininess);

    Color3f         frontAmbient() const;
    Color3f         frontDiffuse() const;
    Color3f         frontSpecular() const;
    Color3f         frontEmission() const;
    float           frontAlpha() const;
    float           frontShininess() const;

    void            enableColorMaterial(bool enableColorMaterial);
    bool            isColorMaterialEnabled() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;
    virtual bool    isFixedFunction() const;

private:
    Color3f m_ambient;
    Color3f m_diffuse;
    Color3f m_specular;
    Color3f m_emission;
    float   m_alpha;                // Alpha, 1.0 is no transparency, 0.0 completely transparent
    float   m_shininess;            // Range 0.0 to 128.0
    bool    m_enableColorMaterial;
};


//==================================================================================================
//
// Controls normalization of normals in fixed function
//
//==================================================================================================
class RenderStateNormalize_FF : public RenderState
{
public:
    RenderStateNormalize_FF(bool enableNormalization = true);

    void            enable(bool enableNormalization);
    bool            isEnabled() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    bool        m_enable;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class RenderStateTextureMapping_FF : public RenderState
{
public:
    enum TextureFunction
    {
        MODULATE,
        DECAL
    };

public:
    RenderStateTextureMapping_FF(Texture2D_FF* texture = NULL);
    ~RenderStateTextureMapping_FF();

    void            setTexture(Texture2D_FF* texture);
    Texture2D_FF*   texture();
    void            setTextureFunction(TextureFunction texFunc);
    TextureFunction textureFunction() const;
    void            setEnvironmentMapping(bool environmentMapping);
    bool            environmentMapping() const;

    void            setupTexture(OpenGLContext* oglContext);
    virtual void    applyOpenGL(OpenGLContext* oglContext) const;
    virtual bool    isFixedFunction() const;

private:
    ref<Texture2D_FF>   m_texture;     
    TextureFunction     m_textureFunction;
    bool                m_environmentMapping;
};




//==================================================================================================
//
// 
//
//==================================================================================================
class RenderStateClipPlanes_FF : public RenderState
{
public:
    RenderStateClipPlanes_FF();

    void            addPlane(const cvf::Plane& plane);
    size_t          planeCount() const;
    const Plane&    plane(size_t index);
    void            removeAllPlanes();

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;
    virtual bool    isFixedFunction() const;

private:
    std::vector<Plane>  m_clipPlanes;
};

}  // namespace cvf
