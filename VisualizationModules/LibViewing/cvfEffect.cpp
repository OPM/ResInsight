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
#include "cvfEffect.h"
#include "cvfTexture.h"
#include "cvfShaderProgram.h"
#include "cvfUniformSet.h"
#include "cvfRenderStateSet.h"
#include "cvfRenderStateTextureBindings.h"

#ifndef CVF_OPENGL_ES 
#include "cvfRenderState_FF.h"
#include "cvfTexture2D_FF.h"
#endif

namespace cvf {



//==================================================================================================
///
/// \class cvf::Effect
/// \ingroup Viewing
///
/// The effect class defines how a drawable should be rendered. (Shader program, uniforms, OpenGL state etc)
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
Effect::Effect()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Effect::~Effect()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Effect::setShaderProgram(ShaderProgram* shaderProgram)
{
    m_shaderProgram = shaderProgram;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ShaderProgram* Effect::shaderProgram()
{
    return m_shaderProgram.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ShaderProgram* Effect::shaderProgram() const
{
    return m_shaderProgram.p();
}


//--------------------------------------------------------------------------------------------------
/// Set or add a uniform
/// 
/// Any existing uniform with the same name will be replaced
//--------------------------------------------------------------------------------------------------
void Effect::setUniform(Uniform* uniform)
{
    CVF_ASSERT(uniform);

    if (m_uniformSet.isNull())
    {
        m_uniformSet = new UniformSet;
    }

    CVF_ASSERT(m_uniformSet.notNull());
    m_uniformSet->setUniform(uniform);
}


//--------------------------------------------------------------------------------------------------
/// Set all uniforms contained in \a sourceUniformSet
//--------------------------------------------------------------------------------------------------
void Effect::setUniformsFromUniformSet(UniformSet* sourceUniformSet)
{
    size_t uniformCount = sourceUniformSet->count();
    for (size_t i = 0; i < uniformCount; i++)
    {
        setUniform(sourceUniformSet->uniform(i));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Effect::setUniformSet(UniformSet* uniformSet)
{
    m_uniformSet = uniformSet;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const UniformSet* Effect::uniformSet() const
{
    return m_uniformSet.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformSet* Effect::uniformSet()
{
    return m_uniformSet.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Effect::setRenderState(RenderState* renderState)
{
    if (m_renderStateSet.isNull())
    {
        m_renderStateSet = new RenderStateSet;
    }

    CVF_ASSERT(m_renderStateSet.notNull());
    m_renderStateSet->setRenderState(renderState);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderState* Effect::renderStateOfType(RenderState::Type type)
{
    if (m_renderStateSet.notNull())
    {
        return m_renderStateSet->renderStateOfType(type);
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RenderState* Effect::renderStateOfType(RenderState::Type type) const
{
    if (m_renderStateSet.notNull())
    {
        return m_renderStateSet->renderStateOfType(type);
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Effect::removeRenderState(RenderState::Type type)
{
    if (m_renderStateSet.notNull())
    {
        RenderState* rs = m_renderStateSet->renderStateOfType(type);
        if (rs)
        {
            m_renderStateSet->removeRenderState(rs);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Effect::setRenderStateSet(RenderStateSet* renderStateSet)
{
    m_renderStateSet = renderStateSet;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateSet* Effect::renderStateSet()
{
    return m_renderStateSet.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RenderStateSet* Effect::renderStateSet() const
{
    return m_renderStateSet.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Effect::deleteOrReleaseOpenGLResources(OpenGLContext* oglContext)
{
    if (m_shaderProgram.notNull())
    {
        m_shaderProgram->deleteProgram(oglContext);

        uint numShaders = m_shaderProgram->shaderCount();
        uint i;
        for (i = 0; i < numShaders; i++)
        {
            Shader* shader = m_shaderProgram->shader(i);
            shader->deleteShader(oglContext);
        }
    }

    if (m_renderStateSet.notNull())
    {
        RenderStateTextureBindings* textureBindings = static_cast<RenderStateTextureBindings*>(m_renderStateSet->renderStateOfType(RenderState::TEXTURE_BINDINGS));
        if (textureBindings)
        {
            int bindingCount = textureBindings->bindingCount();
            int i;
            for (i = 0; i < bindingCount; i++)
            {
                Texture* texture = textureBindings->texture(i);
                texture->deleteTexture(oglContext);
            }
        }

#ifndef CVF_OPENGL_ES
        RenderStateTextureMapping_FF* textureMapping = static_cast<RenderStateTextureMapping_FF*>(m_renderStateSet->renderStateOfType(RenderState::TEXTURE_MAPPING_FF));
        if (textureMapping)
        {
            Texture2D_FF* texture = textureMapping->texture();
            if (texture)
            {
                texture->deleteTexture(oglContext);
            }
        }
#endif
    }
}


} // namespace cvf

