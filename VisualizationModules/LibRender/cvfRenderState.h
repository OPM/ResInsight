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
#include "cvfOpenGLTypes.h"
#include "cvfColor4.h"
#include "cvfString.h"

namespace cvf {

class OpenGLContext;
class ShaderProgram;
class Sampler;
class Texture;


//==================================================================================================
//
// Base class for storing and changing OpenGL state attributes
//
//==================================================================================================
class RenderState : public Object
{
public:
    enum Type
    {
        BLENDING,               // Must start at 0, used for indexing in RenderStateTracker
        COLOR_MASK,             
        CULL_FACE,
        FRONT_FACE,
        DEPTH,
        POINT,
        POLYGON_MODE,
        POLYGON_OFFSET,
        TEXTURE_BINDINGS,

#ifndef CVF_OPENGL_ES
        LIGHTING_FF,            //Fixed function
        MATERIAL_FF,            //Fixed function
        NORMALIZE_FF,           //Fixed function
        TEXTURE_MAPPING_FF,     //Fixed function
#endif

        COUNT                   // Must be the last entry
    };

public:
    virtual ~RenderState();

    Type            type() const;
    virtual void    applyOpenGL(OpenGLContext* oglContext) const = 0;
    virtual bool    isFixedFunction() const;

protected:
    RenderState(Type type);

private:
    Type   m_stateType;
};



//==================================================================================================
//
// Controls OpenGL blending
//
//==================================================================================================
class Blending : public RenderState
{
public:
    enum Function
    {
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        SRC_ALPHA_SATURATE
    };

    enum Equation
    {
        FUNC_ADD, 
        FUNC_SUBTRACT,
        FUNC_REVERSE_SUBTRACT,
        MIN,                    // Unsupported on OpenGL ES
        MAX                     // Unsupported on OpenGL ES
    };

public:
    Blending();

    void            enableBlending(bool blend);
    void            setFunction(Function source, Function destination);
    void            setEquation(Equation equation);

    void            setFunctionSeparate(Function sourceRGB, Function destinationRGB, Function sourceAlpha, Function destinationAlpha);
    void            setEquationSeparate(Equation equationRGB, Equation equationAlpha);

    void            setBlendColor(Color4f blendColor);

    void            configureTransparencyBlending();

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    cvfGLenum       blendFuncOpenGL(Function func) const;
    cvfGLenum       blendEquationOpenGL(Equation eq) const;

private:
    bool        m_enableBlending;
    Function    m_funcSourceRGB;
    Function    m_funcDestinationRGB;
    Function    m_funcSourceAlpha;
    Function    m_funcDestinationAlpha;
    Equation    m_equationRGB;
    Equation    m_equationAlpha;
    Color4f     m_blendColor;
};


//==================================================================================================
//
// Encapsulate OpenGL glColorMask() function.
//
//==================================================================================================
class ColorMask : public RenderState
{
public:
    ColorMask(bool writeAllComponents = true);

    void            enable(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha);
    void            enableWriteAllComponents(bool writeAllComponents);

    bool            isRedEnabled() const;
    bool            isGreenEnabled() const;
    bool            isBlueEnabled() const;
    bool            isAlphaEnabled() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    bool        m_writeRed;
    bool        m_writeGreen;
    bool        m_writeBlue;
    bool        m_writeAlpha;
};



//==================================================================================================
//
// Encapsulate OpenGL glCullFace() and glEnable(GL_CULL_FACE) functions.
//
//==================================================================================================
class CullFace : public RenderState
{
public:
    enum Mode
    {
        BACK,           ///< Cull back facing polygons
        FRONT,          ///< Cull front facing polygons
        FRONT_AND_BACK  ///< No polygons are drawn, but other primitives such as points and lines are drawn
    };

public:
    CullFace(bool enableCulling = true, Mode faceMode = BACK);

    void            enable(bool enableCulling);
    bool            isEnabled() const;

    void            setMode(Mode faceMode);
    Mode            mode() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    bool        m_enableCulling;
    Mode        m_faceMode;
};


//==================================================================================================
//
// Encapsulate OpenGL glFrontFace(). 
//
//==================================================================================================
class FrontFace : public RenderState
{
public:
    enum Mode
    {
        CCW,          ///< Counterclockwise order (default)
        CW            ///< Clockwise order
    };

public:
    FrontFace(Mode faceMode = CCW);

    void            setMode(Mode faceMode);
    Mode            mode() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    Mode        m_faceMode;
};


//==================================================================================================
//
// Encapsulate OpenGL glEnable(GL_DEPTH_TEST), glDepthFunc() and glDepthMask() functions.
//
//==================================================================================================
class Depth : public RenderState
{
public:
    enum Function
    {
        NEVER,      ///< Never passes
        LESS,       ///< Passes if the incoming depth value is less than the stored depth value. This is the OpenGL default.
        EQUAL,      ///< Passes if the incoming depth value is equal to the stored depth value.
        LEQUAL,     ///< Passes if the incoming depth value is less than or equal to the stored depth value.
        GREATER,    ///< Passes if the incoming depth value is greater than the stored depth value.
        NOTEQUAL,   ///< Passes if the incoming depth value is not equal to the stored depth value.
        GEQUAL,     ///< Passes if the incoming depth value is greater than or equal to the stored depth value.
        ALWAYS      ///< Always passes.
    };

public:
    Depth(bool depthTest = true, Function func = LESS, bool depthWrite = true);

    void            setFunction(Function func);
    void            enableDepthTest(bool enableTest);
    void            enableDepthWrite(bool enableWrite);
    
    Function        function() const;
    bool            isDepthTestEnabled() const;
    bool            isDepthWriteEnabled() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    cvfGLenum       depthFuncOpenGL() const;

private:
    Function    m_depthFunc;
    bool        m_enableDepthTest;
    bool        m_enableDepthWrite;
};


//==================================================================================================
//
// Controls OpenGL point size, glPointSize() and glEnable()/glDisable() with GL_PROGRAM_POINT_SIZE
//
//==================================================================================================
class Point : public RenderState
{
public:
    enum Mode
    {
        FIXED_SIZE,     ///< Fixed diameter of raserized points (as specified by point size/glPointSize())
        PROGRAM_SIZE    ///< Point size will be specified using GLSL and the gl_PointSize built-in variable
    };

public:
    Point(Mode sizeMode = FIXED_SIZE);

    void            setMode(Mode sizeMode);
    Mode            mode() const;
    void            enablePointSprite(bool enable);
    bool            isPointSpriteEnabled() const;
    void            setSize(float pointSize);
    float           size() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    Mode    m_sizeMode;
    bool    m_pointSprite;
    float   m_pointSize;
};


//==================================================================================================
//
// Controls OpenGL polygon rasterization mode, glPolygonMode() 
//
//==================================================================================================
class PolygonMode : public RenderState
{
public:
    enum Mode
    {
        FILL,   ///< The interior of the polygons is filled
        LINE,   ///< Boundary edges of the polygons are drawn as line segments
        POINT   ///< Polygon vertices that are marked as the start of a boundary edge are drawn as points
    };

public:
    PolygonMode(Mode frontAndBackFaceMode = FILL);

    void            set(Mode frontAndBackMode);
    void            setFrontFace(Mode mode);
    void            setBackFace(Mode mode);
    Mode            frontFace() const;
    Mode            backFace() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    static cvfGLenum    polygonModeOpenGL(Mode mode);

private:
    Mode    m_frontFaceMode;
    Mode    m_backFaceMode;
};


//==================================================================================================
//
// Encapsulate OpenGL glPolygonOffset() and glEnable()/glDisable() with GL_POLYGON_OFFSET_FILL/LINE/POINT
//
//==================================================================================================
class PolygonOffset : public RenderState
{
public:
    PolygonOffset();

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


//==================================================================================================
//
// Binds a texture and a sampler to a texture unit
//
//==================================================================================================
class TextureBindings : public RenderState
{
public:
    TextureBindings();
    TextureBindings(Texture* texture, Sampler* sampler, const char* samplerUniformName);
    ~TextureBindings();

    void            addBinding(Texture* texture, Sampler* sampler, const char* samplerUniformName);
    int             bindingCount() const;

    Texture*        texture(int bindingIdx);
    const Texture*  texture(int bindingIdx) const;
    Sampler*        sampler(int bindingIdx);
    const Sampler*  sampler(int bindingIdx) const;
    String          samplerUniformName(int bindingIdx) const;
    
    virtual void    applyOpenGL(OpenGLContext* oglContext) const;
    void            setupTextures(OpenGLContext* oglContext);
    void            applySamplerTextureUnitUniforms(OpenGLContext* oglContext, ShaderProgram* shaderProgram) const;

public:
    static const int MAX_TEXTURE_UNITS = 16;

private:
    struct BindingEntry
    {
        ref<Texture>    texture;     
        ref<Sampler>    sampler;  
        String          samplerUniformName; 
    };

    BindingEntry    m_bindings[MAX_TEXTURE_UNITS];
    int             m_bindingCount;
};


}  // namespace cvf
