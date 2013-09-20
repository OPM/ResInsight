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
#include "cvfRenderStateTextureBindings.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"
#include "cvfUniform.h"
#include "cvfShaderProgram.h"
#include "cvfTexture.h"
#include "cvfSampler.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateTextureBindings
/// \ingroup Render
///
/// \warning Requires OpenGL2 support
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateTextureBindings::RenderStateTextureBindings()
:   RenderState(TEXTURE_BINDINGS),
    m_bindingCount(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateTextureBindings::RenderStateTextureBindings(Texture* texture, Sampler* sampler, const char* samplerUniformName)
:   RenderState(TEXTURE_BINDINGS),
    m_bindingCount(0)
{
    addBinding(texture, sampler, samplerUniformName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateTextureBindings::~RenderStateTextureBindings()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTextureBindings::addBinding(Texture* texture, Sampler* sampler, const char* samplerUniformName)
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
int RenderStateTextureBindings::bindingCount() const
{
    return m_bindingCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture* RenderStateTextureBindings::texture(int bindingIdx)
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].texture.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Texture* RenderStateTextureBindings::texture(int bindingIdx) const
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].texture.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Sampler* RenderStateTextureBindings::sampler(int bindingIdx) const
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].sampler.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Sampler* RenderStateTextureBindings::sampler(int bindingIdx)
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].sampler.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String RenderStateTextureBindings::samplerUniformName(int bindingIdx) const
{
    CVF_ASSERT(bindingIdx >= 0 && bindingIdx < m_bindingCount);
    return m_bindings[bindingIdx].samplerUniformName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTextureBindings::setupTextures(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    
    const OpenGLCapabilities* oglCaps = oglContext->capabilities();
    if (!oglCaps->supportsOpenGL2())
    {
        CVF_LOG_RENDER_ERROR(oglContext, "Context does not support texture setup using RenderStateTextureBindings.");
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
void RenderStateTextureBindings::applyOpenGL(OpenGLContext* oglContext) const
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
void RenderStateTextureBindings::applySamplerTextureUnitUniforms(OpenGLContext* oglContext, ShaderProgram* shaderProgram) const
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

