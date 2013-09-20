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

namespace cvf {



//==================================================================================================
//
// 
//
//==================================================================================================
class OpenGLCapabilities : public Object
{
public:
    enum Capability
    {
        FRAMEBUFFER_OBJECT      = 0x00000001,   // Supports framebuffer objects
        GENERATE_MIPMAP_FUNC    = 0x00000002,   // Is the glGenerateMipmap() function supported. (req OGL 3.0 or ARB_framebuffer_object)
        TEXTURE_FLOAT           = 0x00000004,   // Floating-point internal texture formats. (req OGL 3.0 or ARB_texture_float)
        TEXTURE_RG              = 0x00000008,   // One- and two- component internal texture formats. (req. OGL 3.0 or ARB_texture_rg)
        TEXTURE_RECTANGLE       = 0x00000010    // Rectangular textures (req. OGL 3.1 or GL_ARB_texture_rectangle
    };

public:
    OpenGLCapabilities();
    OpenGLCapabilities(const OpenGLCapabilities& other);
    ~OpenGLCapabilities();

    OpenGLCapabilities& operator=(const OpenGLCapabilities& rhs);
    bool                operator==(const OpenGLCapabilities& rhs) const;
    bool                operator!=(const OpenGLCapabilities& rhs) const;

    bool    hasCapability(Capability capability) const;
    void    addCapablity(Capability capability);
    void    removeCapablity(Capability capability);

    bool    supportsOpenGL2() const; 
    bool    supportsOpenGLVer(uint majorVer) const; 
    bool    supportsFixedFunction() const;
    void    configureOpenGLSupport(uint openGLMajorVer);
    void    setSupportsFixedFunction(bool fixedFunction);


private:
    uint        m_capabilityFlags;          // Bitwise or combination of the capabilities
    uint        m_openGLMajorVersion;       // The supported OpenGL version, default to 1.1. (Currently our baseline support requires OpenGL 2.0)
    bool        m_supportsFixedFunction;    // Is fixed function supported?
};


}


