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
#include "cvfOpenGLCapabilities.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::OpenGLCapabilities
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLCapabilities::OpenGLCapabilities()
:   m_capabilityFlags(0),
    m_openGLMajorVersion(1),
    m_supportsFixedFunction(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLCapabilities::OpenGLCapabilities(const OpenGLCapabilities& other)
:   Object()
{
    *this = other;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLCapabilities::~OpenGLCapabilities()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OpenGLCapabilities& OpenGLCapabilities::operator=(const OpenGLCapabilities& rhs)
{
    m_capabilityFlags = rhs.m_capabilityFlags;
    m_openGLMajorVersion = rhs.m_openGLMajorVersion;
    m_supportsFixedFunction = rhs.m_supportsFixedFunction;

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLCapabilities::operator==(const OpenGLCapabilities& rhs) const
{
    if (m_capabilityFlags == rhs.m_capabilityFlags &&
        m_openGLMajorVersion == rhs.m_openGLMajorVersion &&
        m_supportsFixedFunction == rhs.m_supportsFixedFunction)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLCapabilities::operator!=(const OpenGLCapabilities& rhs) const
{
    return !(*this == rhs);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLCapabilities::hasCapability(Capability capability) const
{
    return (m_capabilityFlags & capability) ? true : false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLCapabilities::addCapablity(Capability capability)
{
    m_capabilityFlags |= capability;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLCapabilities::removeCapablity(Capability capability)
{
    m_capabilityFlags &= (~capability);
}


//--------------------------------------------------------------------------------------------------
/// Do we have baseline OpenGL support. Currently this is defined to OpenGL 2.0 support
//--------------------------------------------------------------------------------------------------
bool OpenGLCapabilities::supportsOpenGL2() const
{
    return (m_openGLMajorVersion >= 2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLCapabilities::supportsOpenGLVer(uint majorVer) const
{
    return (m_openGLMajorVersion >= majorVer);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLCapabilities::supportsFixedFunction() const
{
    return m_supportsFixedFunction;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLCapabilities::configureOpenGLSupport(uint openGLMajorVer)
{
    m_openGLMajorVersion = openGLMajorVer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLCapabilities::setSupportsFixedFunction(bool fixedFunction)
{
    m_supportsFixedFunction = fixedFunction;
}

} // namespace cvf

