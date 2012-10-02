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

namespace cvf {

class OpenGLContext;


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
        DEPTH,
        FRONT_FACE,
        LINE,
        POINT,
        POLYGON_MODE,
        POLYGON_OFFSET,
        STENCIL,
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




}  // namespace cvf
