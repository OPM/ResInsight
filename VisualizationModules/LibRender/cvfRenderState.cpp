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

#include "cvfBase.h"
#include "cvfAssert.h"
#include "cvfRenderState.h"
#include "cvfOpenGL.h"
#include "cvfUniform.h"
#include "cvfShaderProgram.h"
#include "cvfTexture.h"
#include "cvfSampler.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderState
/// \ingroup Render
///
/// Base class for classes encapsulating OpenGL state
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderState::RenderState(Type type)
    : m_stateType(type)
{
    CVF_ASSERT(type < COUNT);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderState::~RenderState()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderState::Type RenderState::type() const
{
    return m_stateType;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if this render state relies on Fixed Function OpenGL. 
/// 
/// The default implementation of this function returns false
//--------------------------------------------------------------------------------------------------
bool RenderState::isFixedFunction() const
{
    return false;
}



//==================================================================================================
///
/// \class cvf::Blending
/// \ingroup Render
///
/// Encapsulate OpenGL blending functions: glEnable(GL_BLEND), glBlendEquation(), glBlendEquationSeparate()
/// glBlendFunc(), glBlendFuncSeparate(), glBlendColor()
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendEquation.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendEquationSeparate.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendFunc.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendFuncSeparate.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendColor.xml
/// 
/// \todo
/// Add support for enable/disable blending per drawbuffer: glEnablei(GL_BLEND, drawBufferIndex)
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Blending::Blending()
:   RenderState(BLENDING),
    m_enableBlending(false),
    m_funcSourceRGB(ONE),
    m_funcDestinationRGB(ZERO),
    m_funcSourceAlpha(ONE),
    m_funcDestinationAlpha(ZERO),
    m_equationRGB(FUNC_ADD),
    m_equationAlpha(FUNC_ADD),
    m_blendColor(0, 0, 0, 0)
{
}


//--------------------------------------------------------------------------------------------------
/// glEnable(GL_BLEND) / glDisable(GL_BLEND)
//--------------------------------------------------------------------------------------------------
void Blending::enableBlending(bool blend/*, uint drawBufferIndex*/)
{
    m_enableBlending = blend;
}


//--------------------------------------------------------------------------------------------------
/// glBlendFunc()
//--------------------------------------------------------------------------------------------------
void Blending::setFunction(Function source, Function destination)
{
    m_funcSourceRGB         = source;
    m_funcSourceAlpha       = source;
    m_funcDestinationRGB    = destination;
    m_funcDestinationAlpha  = destination;
}


//--------------------------------------------------------------------------------------------------
/// glBlendEquation(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void Blending::setEquation(Equation eq)
{
    m_equationRGB   = eq;
    m_equationAlpha = eq;
}


//--------------------------------------------------------------------------------------------------
/// glBlendFuncSeparate(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void Blending::setFunctionSeparate(Function sourceRGB, Function destinationRGB, Function sourceAlpha, Function destinationAlpha)
{
    m_funcSourceRGB         = sourceRGB;
    m_funcDestinationRGB    = destinationRGB;
    m_funcSourceAlpha       = sourceAlpha;
    m_funcDestinationAlpha  = destinationAlpha;
}


//--------------------------------------------------------------------------------------------------
/// glBlendEquationSeparate(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void Blending::setEquationSeparate(Equation equationRGB, Equation equationAlpha)
{
    m_equationRGB   = equationRGB;
    m_equationAlpha = equationAlpha;
}


//--------------------------------------------------------------------------------------------------
/// glBlendColor(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void Blending::setBlendColor(Color4f blendColor)
{
    m_blendColor = blendColor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Blending::configureTransparencyBlending()
{
    m_enableBlending = true;
    setFunction(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Blending::applyOpenGL(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    /// As we do not care about specific support for OpenGL 1.4, 1.3 etc., everything that is not in 1.1
    /// will require at least support for our baseline (currently OpenGL 2.0)
    bool openGL2Support = oglContext->capabilities()->supportsOpenGL2();

    if (m_enableBlending)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if ((m_funcSourceRGB == m_funcSourceAlpha) && (m_funcDestinationRGB == m_funcDestinationAlpha))
    {
        glBlendFunc(blendFuncOpenGL(m_funcSourceRGB), blendFuncOpenGL(m_funcDestinationRGB));
    }
    else
    {
        if (openGL2Support)
        {
            glBlendFuncSeparate(blendFuncOpenGL(m_funcSourceRGB), blendFuncOpenGL(m_funcDestinationRGB), blendFuncOpenGL(m_funcSourceAlpha), blendFuncOpenGL(m_funcDestinationAlpha));
        }
        else
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support separate blend functions.");
        }
    }

    if (openGL2Support)
    {
        if (m_equationRGB == m_equationAlpha)
        {
            glBlendEquation(blendEquationOpenGL(m_equationRGB));
        }
        else
        {
            glBlendEquationSeparate(blendEquationOpenGL(m_equationRGB), blendEquationOpenGL(m_equationAlpha));
        }

        glBlendColor(m_blendColor.r(), m_blendColor.g(), m_blendColor.b(), m_blendColor.a());
    }
    else
    {
        // Only error reporting here
        if (m_equationRGB != FUNC_ADD ||
            m_equationRGB != m_equationAlpha)
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support blend equations.");
        }

        if (m_blendColor != Color4f(0, 0, 0, 0))
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support blend color.");
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum Blending::blendEquationOpenGL(Equation eq) const
{
    switch (eq)
    {
        case FUNC_ADD:              return GL_FUNC_ADD;
        case FUNC_SUBTRACT:         return GL_FUNC_SUBTRACT;
        case FUNC_REVERSE_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
#ifndef CVF_OPENGL_ES
        case MIN:                   return GL_MIN;
        case MAX:                   return GL_MAX;
#endif
    }

    CVF_FAIL_MSG("Unhandled blend equation");
    return 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum Blending::blendFuncOpenGL(Function func) const
{
    switch (func)
    {
        case ZERO:                      return GL_ZERO;
        case ONE:                       return GL_ONE;
        case SRC_COLOR:                 return GL_SRC_COLOR;
        case ONE_MINUS_SRC_COLOR:       return GL_ONE_MINUS_SRC_COLOR;
        case DST_COLOR:                 return GL_DST_COLOR;
        case ONE_MINUS_DST_COLOR:       return GL_ONE_MINUS_DST_COLOR;
        case SRC_ALPHA:                 return GL_SRC_ALPHA;
        case ONE_MINUS_SRC_ALPHA:       return GL_ONE_MINUS_SRC_ALPHA;
        case DST_ALPHA:                 return GL_DST_ALPHA;
        case ONE_MINUS_DST_ALPHA:       return GL_ONE_MINUS_DST_ALPHA;
        case CONSTANT_COLOR:            return GL_CONSTANT_COLOR;
        case ONE_MINUS_CONSTANT_COLOR:  return GL_ONE_MINUS_CONSTANT_COLOR;
        case CONSTANT_ALPHA:            return GL_CONSTANT_ALPHA;
        case ONE_MINUS_CONSTANT_ALPHA:  return GL_ONE_MINUS_CONSTANT_ALPHA;
        case SRC_ALPHA_SATURATE:        return GL_SRC_ALPHA_SATURATE;
    }

    CVF_FAIL_MSG("Unhandled blend func");
    return 0;
}



//==================================================================================================
///
/// \class cvf::ColorMask
/// \ingroup Render
///
/// Encapsulate OpenGL glColorMask() function.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glColorMask.xml
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ColorMask::ColorMask(bool writeAllComponents)
:   RenderState(COLOR_MASK),
    m_writeRed(writeAllComponents),
    m_writeGreen(writeAllComponents),
    m_writeBlue(writeAllComponents),
    m_writeAlpha(writeAllComponents)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ColorMask::enable(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha)
{
    m_writeRed = writeRed;
    m_writeGreen = writeGreen;
    m_writeBlue = writeBlue;
    m_writeAlpha = writeAlpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ColorMask::enableWriteAllComponents(bool writeAllComponents)
{
    m_writeRed = writeAllComponents;
    m_writeGreen = writeAllComponents;
    m_writeBlue = writeAllComponents;
    m_writeAlpha = writeAllComponents;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ColorMask::isRedEnabled() const
{
    return m_writeRed;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ColorMask::isGreenEnabled() const
{
    return m_writeGreen;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ColorMask::isBlueEnabled() const
{
    return m_writeBlue;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ColorMask::isAlphaEnabled() const
{
    return m_writeAlpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ColorMask::applyOpenGL(OpenGLContext* oglContext) const
{
    GLboolean writeRed   = m_writeRed   ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
    GLboolean writeGreen = m_writeGreen ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
    GLboolean writeBlue  = m_writeBlue  ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
    GLboolean writeAlpha = m_writeAlpha ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);

    glColorMask(writeRed, writeGreen, writeBlue, writeAlpha);

    CVF_CHECK_OGL(oglContext);
}



//==================================================================================================
///
/// \class cvf::CullFace
/// \ingroup Render
///
/// Encapsulate OpenGL glCullFace() and glEnable(GL_CULL_FACE) functions.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glCullFace.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CullFace::CullFace(bool enableCulling, Mode faceMode)
:   RenderState(CULL_FACE),
    m_enableCulling(enableCulling),
    m_faceMode(faceMode)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CullFace::enable(bool enableCulling)
{
    m_enableCulling = enableCulling;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CullFace::isEnabled() const
{
    return m_enableCulling;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CullFace::setMode(Mode faceMode)
{
    m_faceMode = faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CullFace::Mode CullFace::mode() const
{
    return m_faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CullFace::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enableCulling)
    {
        if (m_faceMode == BACK)    
        {
            glCullFace(GL_BACK);
        }
        else if (m_faceMode == FRONT)
        {
            glCullFace(GL_FRONT);
        }
        else
        {
            glCullFace(GL_FRONT_AND_BACK);
        }

        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    CVF_CHECK_OGL(oglContext);
}



//==================================================================================================
///
/// \class cvf::FrontFace
/// \ingroup Render
///
/// Encapsulate OpenGL glFrontFace() used to specify polygon winding. Used together with CullFace
/// render state and the gl_FrontFacing bultin shader input variable.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glFrontFace.xml
/// \sa http://www.opengl.org/sdk/docs/manglsl/xhtml/gl_FrontFacing.xml
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FrontFace::FrontFace(Mode faceMode)
:   RenderState(FRONT_FACE),
    m_faceMode(faceMode)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FrontFace::setMode(Mode faceMode)
{
    m_faceMode = faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FrontFace::Mode FrontFace::mode() const
{
    return m_faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FrontFace::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_faceMode == CW)    
    {
        glFrontFace(GL_CW);
    }
    else 
    {
        glFrontFace(GL_CCW);
    }

    CVF_CHECK_OGL(oglContext);
}



//==================================================================================================
///
/// \class cvf::Depth
/// \ingroup Render
///
/// Encapsulate OpenGL glEnable(GL_DEPTH_TEST), glDepthFunc() and glDepthMask() functions.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthFunc.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthMask.xml
/// 
/// \todo
/// Add support for glDepthRange() if needed.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Depth::Depth(bool depthTest, Function func, bool depthWrite)
:   RenderState(DEPTH)
{
    m_enableDepthTest = depthTest;
    m_depthFunc = func;
    m_enableDepthWrite = depthWrite;
}


//--------------------------------------------------------------------------------------------------
/// Specifies the depth comparison function.
/// 
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthFunc.xml
//--------------------------------------------------------------------------------------------------
void Depth::setFunction(Function func)
{
    m_depthFunc = func;
}


//--------------------------------------------------------------------------------------------------
/// Enable or disable depth testing and updating of the depth buffer.
/// 
/// \param enableTest  Specify true to enable testing against and updating of depth buffer.
/// 
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml with GL_DEPTH_TEST
/// 
/// From OpenGL docs: 
/// If enabled, do depth comparisons and update the depth buffer. Note that even if the depth buffer 
/// exists and the depth mask is non-zero, the depth buffer is not updated if the depth test is disabled
//--------------------------------------------------------------------------------------------------
void Depth::enableDepthTest(bool enableTest)
{
    m_enableDepthTest = enableTest;
}


//--------------------------------------------------------------------------------------------------
/// Enable or disable writing into the depth buffer
/// 
/// \param enableWrite  Specify true to enable writing to depth buffer, false to disable. The default is true.
/// 
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthMask.xml
//--------------------------------------------------------------------------------------------------
void Depth::enableDepthWrite(bool enableWrite)
{
    m_enableDepthWrite = enableWrite;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Depth::Function Depth::function() const
{
    return m_depthFunc;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Depth::isDepthTestEnabled() const
{
    return m_enableDepthTest;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Depth::isDepthWriteEnabled() const
{
    return m_enableDepthWrite;
}


//--------------------------------------------------------------------------------------------------
/// Specify the depth setting to OpenGL.
//--------------------------------------------------------------------------------------------------
void Depth::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enableDepthTest)
    {
        GLenum depthFuncOGL = depthFuncOpenGL();
        glDepthFunc(depthFuncOGL);

        GLboolean enableDepthWrite = m_enableDepthWrite ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
        glDepthMask(enableDepthWrite);

        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    
    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum Depth::depthFuncOpenGL() const
{
    switch (m_depthFunc)
    {
        case NEVER:     return GL_NEVER;
        case LESS:      return GL_LESS;
        case EQUAL:     return GL_EQUAL;
        case LEQUAL:    return GL_LEQUAL;
        case GREATER:   return GL_GREATER;
        case NOTEQUAL:  return GL_NOTEQUAL;
        case GEQUAL:    return GL_GEQUAL;
        case ALWAYS:    return GL_ALWAYS;
    }

    CVF_FAIL_MSG("Unhandled depth func");
    return 0;
}


//==================================================================================================
///
/// \class cvf::Point
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Point::Point(Mode sizeMode)
:   RenderState(POINT),
    m_sizeMode(sizeMode),
    m_pointSprite(false),
    m_pointSize(1.0f)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Point::setMode(Mode sizeMode)
{
    m_sizeMode = sizeMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Point::Mode Point::mode() const
{
    return m_sizeMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Point::enablePointSprite(bool enable)
{
    m_pointSprite = enable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Point::isPointSpriteEnabled() const
{
    return m_pointSprite;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Point::setSize(float pointSize)
{
    m_pointSize = pointSize;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Point::size() const
{
    return m_pointSize;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Point::applyOpenGL(OpenGLContext* oglContext) const
{
    // OpenGL ES does not support fixed point size
    // Point size is always specified using GLSL's gl_PointSize
#ifndef CVF_OPENGL_ES
    bool openGL2Support = oglContext->capabilities()->supportsOpenGL2();

    if (m_sizeMode == FIXED_SIZE)
    {
        if (openGL2Support)
        {
            glDisable(GL_PROGRAM_POINT_SIZE);
        }
    
        glPointSize(m_pointSize);
    }
    else
    {
        if (openGL2Support)
        {
            glEnable(GL_PROGRAM_POINT_SIZE);
        }
        else
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support program point size.");
        }
    }

    if (openGL2Support)
    {
        if (m_pointSprite) glEnable(GL_POINT_SPRITE);
        else               glDisable(GL_POINT_SPRITE);
    }
    else
    {
        if (m_pointSprite)
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support point sprites.");
        }
    }
#endif

    CVF_CHECK_OGL(oglContext);
}


//==================================================================================================
///
/// \class cvf::PolygonMode
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PolygonMode::PolygonMode(Mode frontAndBackFaceMode)
:   RenderState(POLYGON_MODE),
    m_frontFaceMode(frontAndBackFaceMode),
    m_backFaceMode(frontAndBackFaceMode)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonMode::set(Mode frontAndBackMode)
{
    m_frontFaceMode = frontAndBackMode;
    m_backFaceMode = frontAndBackMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonMode::setFrontFace(Mode mode)
{
    m_frontFaceMode = mode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonMode::setBackFace(Mode mode)
{
    m_backFaceMode = mode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PolygonMode::Mode PolygonMode::frontFace() const
{
    return m_frontFaceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PolygonMode::Mode PolygonMode::backFace() const
{
    return m_backFaceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonMode::applyOpenGL(OpenGLContext* oglContext) const
{
#ifndef CVF_OPENGL_ES
    if (m_frontFaceMode == m_backFaceMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, polygonModeOpenGL(m_frontFaceMode)); 
    }
    else
    {
        glPolygonMode(GL_FRONT, polygonModeOpenGL(m_frontFaceMode)); 
        glPolygonMode(GL_BACK, polygonModeOpenGL(m_backFaceMode)); 
    }
#endif

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum PolygonMode::polygonModeOpenGL(Mode mode)
{
    switch (mode)
    {
#ifndef CVF_OPENGL_ES
        case FILL:  return GL_FILL;
        case LINE:  return GL_LINE;
        case POINT: return GL_POINT;
        default:    CVF_FAIL_MSG("Unhandled polygon mode");
#endif
    }

    return 0;
}


//==================================================================================================
///
/// \class cvf::PolygonOffset
/// \ingroup Render
///
/// Encapsulate OpenGL glPolygonOffset() and glEnable()/glDisable() with GL_POLYGON_OFFSET_FILL/LINE/POINT
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glPolygonOffset.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
///
//==================================================================================================
PolygonOffset::PolygonOffset()
:   RenderState(POLYGON_OFFSET),
    m_factor(0.0f),
    m_units(0.0f),
    m_enableFillMode(false),
    m_enableLineMode(false),
    m_enablePointMode(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::enableFillMode(bool enableFill)
{
    m_enableFillMode = enableFill;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::enableLineMode(bool enableLine)
{
    m_enableLineMode = enableLine;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::enablePointMode(bool enablePoint)
{
    m_enablePointMode = enablePoint;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PolygonOffset::isFillModeEnabled() const
{
    return m_enableFillMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PolygonOffset::isLineModeEnabled() const
{
    return m_enableLineMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PolygonOffset::isPointModeEnabled() const
{
    return m_enablePointMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::setFactor(float factor)
{
    m_factor = factor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::setUnits(float units)
{
    m_units = units;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float PolygonOffset::factor() const
{
    return m_factor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float PolygonOffset::units() const
{
    return m_units;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::configurePolygonPositiveOffset()
{
    m_enableFillMode = true;
    m_enableLineMode = false;
    m_enablePointMode = false;
    m_factor = 1.0;
    m_units = 1.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::configureLineNegativeOffset()
{
    m_enableFillMode = false;
    m_enableLineMode = true;
    m_enablePointMode = false;
    m_factor = -1.0;
    m_units = -1.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PolygonOffset::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enableFillMode ||
        m_enableLineMode ||
        m_enablePointMode)
    {
        glPolygonOffset(m_factor, m_units);
    }

    if (m_enableFillMode)   glEnable(GL_POLYGON_OFFSET_FILL);
    else                    glDisable(GL_POLYGON_OFFSET_FILL);
        
#ifndef CVF_OPENGL_ES
    if (m_enableLineMode)   glEnable(GL_POLYGON_OFFSET_LINE);
    else                    glDisable(GL_POLYGON_OFFSET_LINE);

    if (m_enablePointMode)  glEnable(GL_POLYGON_OFFSET_POINT);
    else                    glDisable(GL_POLYGON_OFFSET_POINT);
#endif
    
    CVF_CHECK_OGL(oglContext);
}



//==================================================================================================
///
/// \class cvf::TextureBindings
/// \ingroup Render
///
/// \warning Requires OpenGL2 support
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureBindings::TextureBindings()
:   RenderState(TEXTURE_BINDINGS),
    m_bindingCount(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureBindings::TextureBindings(Texture* texture, Sampler* sampler, const char* samplerUniformName)
:   RenderState(TEXTURE_BINDINGS),
    m_bindingCount(0)
{
    addBinding(texture, sampler, samplerUniformName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureBindings::~TextureBindings()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureBindings::addBinding(Texture* texture, Sampler* sampler, const char* samplerUniformName)
{
    CVF_ASSERT(m_bindingCount < MAX_TEXTURE_UNITS - 1);
    CVF_ASSERT(texture && sampler && samplerUniformName);

    m_bindings[m_bindingCount].sampler = sampler;
    m_bindings[m_bindingCount].texture = texture;
    m_bindings[m_bindingCount].samplerUniformName = samplerUniformName;
    m_bindingCount++;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int TextureBindings::bindingCount() const
{
    return m_bindingCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture* TextureBindings::texture(int bindingIdx)
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].texture.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Texture* TextureBindings::texture(int bindingIdx) const
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].texture.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Sampler* TextureBindings::sampler(int bindingIdx) const
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].sampler.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Sampler* TextureBindings::sampler(int bindingIdx)
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].sampler.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String TextureBindings::samplerUniformName(int bindingIdx) const
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].samplerUniformName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureBindings::setupTextures(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    
    const OpenGLCapabilities* oglCaps = oglContext->capabilities();
    if (!oglCaps->supportsOpenGL2())
    {
        CVF_LOG_RENDER_ERROR(oglContext, "Context does not support texture setup using TextureBindings.");
        return;
    }

    glActiveTexture(GL_TEXTURE0);

    int i;
    for (i = 0; i < m_bindingCount; i++)
    {
        Texture* texture = m_bindings[i].texture.p();

        if (texture->textureOglId() == 0)
        {
            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i));
            texture->setupTexture(oglContext);
            CVF_CHECK_OGL(oglContext);
        }
        else
        {
            // Handle case where mipmap generation is enabled, but the mipmaps are not present
            // This will typically happen if the texture has been rendered to using an FBO
            // In that case the texture exists, but no mipmaps have yet been generated
            if (texture->isMipmapGenerationEnabled() && !texture->hasMipmap())
            {
                if (oglCaps->hasCapability(OpenGLCapabilities::GENERATE_MIPMAP_FUNC))
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i));
                    texture->generateMipmap(oglContext);
                    CVF_CHECK_OGL(oglContext);
                }
                else
                {
                    CVF_LOG_RENDER_ERROR(oglContext, "Context does not support explicit mipmap generation.");
                }
            }
        }
    }       
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureBindings::applyOpenGL(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    // The apply function needs to work for all contexts in its default state
    // so just return if no bindings have been set
    if (m_bindingCount == 0)
    {
        return;
    }

    if (!oglContext->capabilities()->supportsOpenGL2())
    {
        CVF_LOG_RENDER_ERROR(oglContext, "Context does not support TextureBinding application.");
        return;
    }
    
    int i;
    for (i = 0; i < m_bindingCount; i++)
    {
        const Texture* texture = m_bindings[i].texture.p();
        const Sampler* sampler = m_bindings[i].sampler.p();
        CVF_ASSERT(texture && sampler);

        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i));

        texture->bind(oglContext);
        texture->setupTextureParamsFromSampler(oglContext, *sampler);

        CVF_CHECK_OGL(oglContext);
    }   
}


//--------------------------------------------------------------------------------------------------
/// Specify the mapping between the sampler name in the shader program and the texture unit
/// This is done by providing an Int Uniform with the name of the sampler and the index of the 
/// texture unit.
//--------------------------------------------------------------------------------------------------
void TextureBindings::applySamplerTextureUnitUniforms(OpenGLContext* oglContext, ShaderProgram* shaderProgram) const
{
    CVF_ASSERT(shaderProgram);

    int i;
    for (i = 0; i < m_bindingCount; i++)
    {
        UniformInt uniform(m_bindings[i].samplerUniformName.toAscii().ptr(), i);
        shaderProgram->applyUniform(oglContext, uniform);
    }
}


} // namespace cvf

