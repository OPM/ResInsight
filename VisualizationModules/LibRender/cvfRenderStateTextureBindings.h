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

#include "cvfRenderState.h"
#include "cvfString.h"

namespace cvf {

class Sampler;
class Texture;
class ShaderProgram;


//==================================================================================================
//
// Binds a texture and a sampler to a texture unit
//
//==================================================================================================
class RenderStateTextureBindings : public RenderState
{
public:
    RenderStateTextureBindings();
    RenderStateTextureBindings(Texture* texture, Sampler* sampler, const char* samplerUniformName);
    ~RenderStateTextureBindings();

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
