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
#include "cvfColor4.h"
#include "cvfColor3.h"
#include "cvfMath.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Color4f
/// \ingroup Core
///
/// Class for storing and manipulating 4-component RGBA colors
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor, initializes object to RGB=0.0 and Alpha=1.0
//--------------------------------------------------------------------------------------------------
Color4f::Color4f()
{
    m_rgba[0] = 0.0f;
    m_rgba[1] = 0.0f;
    m_rgba[2] = 0.0f;
    m_rgba[3] = 1.0f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4f::Color4f(float r, float g, float b, float alpha)
{
    m_rgba[0] = r;
    m_rgba[1] = g;
    m_rgba[2] = b;
    m_rgba[3] = alpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4f::Color4f(const Color4f& other)
{
    m_rgba[0] = other.m_rgba[0];
    m_rgba[1] = other.m_rgba[1];
    m_rgba[2] = other.m_rgba[2];
    m_rgba[3] = other.m_rgba[3];
}


//--------------------------------------------------------------------------------------------------
/// Construct from 3 component RGB color. Alpha value will be set to 1.0
//--------------------------------------------------------------------------------------------------
Color4f::Color4f(const Color3f& rgbColor)
{
    m_rgba[0] = rgbColor.r();
    m_rgba[1] = rgbColor.g();
    m_rgba[2] = rgbColor.b();
    m_rgba[3] = 1.0f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4f::Color4f(const Color3f& rgbColor, float alpha)
{
    m_rgba[0] = rgbColor.r();
    m_rgba[1] = rgbColor.g();
    m_rgba[2] = rgbColor.b();
    m_rgba[3] = alpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4f::Color4f(Color3::ColorIdent colorIdent)
{
    Color3f rgb(colorIdent);
    m_rgba[0] = rgb.r();
    m_rgba[1] = rgb.g();
    m_rgba[2] = rgb.b();
    m_rgba[3] = 1.0f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4f::Color4f(const Color4ub& other)
{
    m_rgba[0] = static_cast<float>(other.r())/255.0f;
    m_rgba[1] = static_cast<float>(other.g())/255.0f;
    m_rgba[2] = static_cast<float>(other.b())/255.0f;
    m_rgba[3] = static_cast<float>(other.a())/255.0f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4f& Color4f::operator=(const Color4f& rhs)
{
    m_rgba[0] = rhs.m_rgba[0];
    m_rgba[1] = rhs.m_rgba[1];
    m_rgba[2] = rhs.m_rgba[2];
    m_rgba[3] = rhs.m_rgba[3];

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Color4f::operator==(const Color4f& rhs) const
{
    if (m_rgba[0] == rhs.m_rgba[0] &&
        m_rgba[1] == rhs.m_rgba[1] &&
        m_rgba[2] == rhs.m_rgba[2] &&
        m_rgba[3] == rhs.m_rgba[3])
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
bool Color4f::operator!=(const Color4f& rhs) const
{
    return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Sets all color components
//--------------------------------------------------------------------------------------------------
void Color4f::set(float r, float g, float b, float alpha)
{
    m_rgba[0] = r;
    m_rgba[1] = g;
    m_rgba[2] = b;
    m_rgba[3] = alpha;
}


//--------------------------------------------------------------------------------------------------
/// Set from 3-component RGB color and separate alpha factor
//--------------------------------------------------------------------------------------------------
void Color4f::set(const Color3f& rgbColor, float alpha)
{
    m_rgba[0] = rgbColor.r();
    m_rgba[1] = rgbColor.g();
    m_rgba[2] = rgbColor.b();
    m_rgba[3] = alpha;
}


//--------------------------------------------------------------------------------------------------
/// Set from 3-component RGB color. Alpha will be set to 1.0
//--------------------------------------------------------------------------------------------------
void Color4f::set(const Color3f& rgbColor)
{
    m_rgba[0] = rgbColor.r();
    m_rgba[1] = rgbColor.g();
    m_rgba[2] = rgbColor.b();
    m_rgba[3] = 1.0f;
}


//--------------------------------------------------------------------------------------------------
/// Query whether this color is valid
/// 
/// For a color to be considered valid, all the component values must be in the range 0.0 to 1.0
//--------------------------------------------------------------------------------------------------
bool Color4f::isValid() const
{
    if (!Math::valueInRange(m_rgba[0], 0.0f, 1.0f)) return false;
    if (!Math::valueInRange(m_rgba[1], 0.0f, 1.0f)) return false;
    if (!Math::valueInRange(m_rgba[2], 0.0f, 1.0f)) return false;
    if (!Math::valueInRange(m_rgba[3], 0.0f, 1.0f)) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const float* Color4f::ptr() const
{
    return m_rgba;
}


//--------------------------------------------------------------------------------------------------
/// Convert to 3-component RGB color by dropping the alpha value
//--------------------------------------------------------------------------------------------------
Color3f Color4f::toColor3f() const
{
    return Color3f(m_rgba[0], m_rgba[1], m_rgba[2]);
}



//==================================================================================================
///
/// \class cvf::Color4ub
/// \ingroup Core
///
/// Class for storing unsigned byte RGBA colors
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor, initializes object to RGB=0.0 and Alpha=1.0
//--------------------------------------------------------------------------------------------------
Color4ub::Color4ub()
{
    m_rgba[0] = 0;
    m_rgba[1] = 0;
    m_rgba[2] = 0;
    m_rgba[3] = 255;
}

//--------------------------------------------------------------------------------------------------
/// Constructor with initialization of all components
//--------------------------------------------------------------------------------------------------
Color4ub::Color4ub(ubyte r, ubyte g, ubyte b, ubyte a)
{
    m_rgba[0] = r;
    m_rgba[1] = g;
    m_rgba[2] = b;
    m_rgba[3] = a;
}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
Color4ub::Color4ub(const Color4ub& other)
{
    m_rgba[0] = other.m_rgba[0];
    m_rgba[1] = other.m_rgba[1];
    m_rgba[2] = other.m_rgba[2];
    m_rgba[3] = other.m_rgba[3];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4ub::Color4ub(const Color3ub& rgbColor, ubyte a)
{
    m_rgba[0] = rgbColor.r();
    m_rgba[1] = rgbColor.g();
    m_rgba[2] = rgbColor.b();
    m_rgba[3] = a;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4ub::Color4ub(const Color3ub& rgbColor)
{
    m_rgba[0] = rgbColor.r();
    m_rgba[1] = rgbColor.g();
    m_rgba[2] = rgbColor.b();
    m_rgba[3] = 255;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4ub::Color4ub(Color3::ColorIdent colorIdent)
{
    Color3ub color3(colorIdent);

    set(color3.r(), color3.g(), color3.b(), 255);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color4ub::Color4ub(const Color4f& other)
{
    m_rgba[0] = static_cast<ubyte>(other.r()*255.0f);
    m_rgba[1] = static_cast<ubyte>(other.g()*255.0f);
    m_rgba[2] = static_cast<ubyte>(other.b()*255.0f);
    m_rgba[3] = static_cast<ubyte>(other.a()*255.0f);
}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
Color4ub& Color4ub::operator=(const Color4ub& rhs)
{
    m_rgba[0] = rhs.m_rgba[0];
    m_rgba[1] = rhs.m_rgba[1];
    m_rgba[2] = rhs.m_rgba[2];
    m_rgba[3] = rhs.m_rgba[3];

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Color4ub::operator==(const Color4ub& rhs) const
{
    if (m_rgba[0] == rhs.m_rgba[0] &&
        m_rgba[1] == rhs.m_rgba[1] &&
        m_rgba[2] == rhs.m_rgba[2] &&
        m_rgba[3] == rhs.m_rgba[3])
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
bool Color4ub::operator!=(const Color4ub& rhs) const
{
    return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Sets all color components
//--------------------------------------------------------------------------------------------------
void Color4ub::set(ubyte r, ubyte g, ubyte b, ubyte a)
{
    m_rgba[0] = r;
    m_rgba[1] = g;
    m_rgba[2] = b;
    m_rgba[3] = a;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ubyte* Color4ub::ptr() const
{
    return m_rgba;
}

} // namespace cvf
