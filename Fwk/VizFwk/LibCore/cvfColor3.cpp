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
#include "cvfColor3.h"
#include "cvfMath.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Color3
/// \ingroup Core
///
/// Abstract base class for 3-component RGB colors. 
///
/// Currently, this class only contains the ColorIdent enums,
//==================================================================================================



//==================================================================================================
///
/// \class cvf::Color3f
/// \ingroup Core
///
/// Class for storing and manipulating 3-component RGB floating point colors
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor, initializes all color components to 0.0
//--------------------------------------------------------------------------------------------------
Color3f::Color3f()
{
    m_rgb[0] = 0.0f;
    m_rgb[1] = 0.0f;
    m_rgb[2] = 0.0f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f::Color3f(float r, float g, float b)
{
    m_rgb[0] = r;
    m_rgb[1] = g;
    m_rgb[2] = b;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f::Color3f(const Color3f& other)
{
    m_rgb[0] = other.m_rgb[0];
    m_rgb[1] = other.m_rgb[1];
    m_rgb[2] = other.m_rgb[2];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f::Color3f(ColorIdent colorIdent)
{
    Color3ub byteColor(colorIdent);
    *this = Color3f::fromByteColor(byteColor.r(), byteColor.g(), byteColor.b());
}


//--------------------------------------------------------------------------------------------------
/// Explicit conversion constructor from byte color
//--------------------------------------------------------------------------------------------------
Color3f::Color3f(const Color3ub& byteColor)
{
    *this = Color3f::fromByteColor(byteColor.r(), byteColor.g(), byteColor.b());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f& Color3f::operator=(const Color3f& rhs)
{
    m_rgb[0] = rhs.m_rgb[0];
    m_rgb[1] = rhs.m_rgb[1];
    m_rgb[2] = rhs.m_rgb[2];

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f& Color3f::operator=(ColorIdent colorIdent)
{
    return operator=(Color3f(colorIdent));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Color3f::operator==(const Color3f& rhs) const
{
    if (m_rgb[0] == rhs.m_rgb[0] &&
        m_rgb[1] == rhs.m_rgb[1] &&
        m_rgb[2] == rhs.m_rgb[2])
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
bool Color3f::operator!=(const Color3f& rhs) const
{
    return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Sets all color components
//--------------------------------------------------------------------------------------------------
void Color3f::set(float r, float g, float b)
{
    m_rgb[0] = r;
    m_rgb[1] = g;
    m_rgb[2] = b;
}


//--------------------------------------------------------------------------------------------------
/// Query whether this color is valid
/// 
/// For a color to be considered valid, all the component values must be in the range 0.0 to 1.0
//--------------------------------------------------------------------------------------------------
bool Color3f::isValid() const
{
    if (!Math::valueInRange(m_rgb[0], 0.0f, 1.0f)) return false;
    if (!Math::valueInRange(m_rgb[1], 0.0f, 1.0f)) return false;
    if (!Math::valueInRange(m_rgb[2], 0.0f, 1.0f)) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const float* Color3f::ptr() const
{
    return m_rgb;
}

//--------------------------------------------------------------------------------------------------
/// Get red component as byte color (unsigned char)
//--------------------------------------------------------------------------------------------------
unsigned char Color3f::rByte() const
{
    int val = static_cast<int>(m_rgb[0]*255.0f + 0.5f);
    
    if      (val < 0)   return 0;
    else if (val > 255) return 255;
    else                return static_cast<unsigned char>(val);
}


//--------------------------------------------------------------------------------------------------
/// Get green component as byte color (unsigned char)
//--------------------------------------------------------------------------------------------------
unsigned char Color3f::gByte() const
{
    int val = static_cast<int>(m_rgb[1]*255.0f + 0.5f);

    if      (val < 0)   return 0;
    else if (val > 255) return 255;
    else                return static_cast<unsigned char>(val);
}


//--------------------------------------------------------------------------------------------------
/// Get blue component as a byte color (unsigned char)
//--------------------------------------------------------------------------------------------------
unsigned char Color3f::bByte() const
{
    int val = static_cast<int>(m_rgb[2]*255.0f + 0.5f);

    if      (val < 0)   return 0;
    else if (val > 255) return 255;
    else                return static_cast<unsigned char>(val);
}



//--------------------------------------------------------------------------------------------------
/// Static function to construct a Color3f object from byte RGB values.
/// 
/// All the values parameter values should be in the range 0-255. The returned object will always
/// have its components clamped to the range 0.0 to 1.0
//--------------------------------------------------------------------------------------------------
Color3f Color3f::fromByteColor(ubyte r, ubyte g, ubyte b)
{
    // Constructor sets all elements to 0
    Color3f c;

    c.m_rgb[0] = static_cast<float>(r)/255.0f;
    c.m_rgb[1] = static_cast<float>(g)/255.0f;
    c.m_rgb[2] = static_cast<float>(b)/255.0f;

    return c;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator < (const Color3f& color1, const Color3f& color2)
{
    for (int i = 0; i < 3; i++)
    {
        if (color1.m_rgb[i] > color2.m_rgb[i])
            return false;
        else if (color1.m_rgb[i] < color2.m_rgb[i])
            return true;
    }
    return false;
}


//==================================================================================================
///
/// \class cvf::Color3ub
/// \ingroup Core
///
/// Class for storing unsigned byte RGB colors
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor, initializes all color components to 0
//--------------------------------------------------------------------------------------------------
Color3ub::Color3ub()
{
    m_rgb[0] = 0;
    m_rgb[1] = 0;
    m_rgb[2] = 0;
}

//--------------------------------------------------------------------------------------------------
/// Constructor with initialization of all components
//--------------------------------------------------------------------------------------------------
Color3ub::Color3ub(ubyte r, ubyte g, ubyte b)
{
    m_rgb[0] = r;
    m_rgb[1] = g;
    m_rgb[2] = b;
}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
Color3ub::Color3ub(const Color3ub& other)
{
    m_rgb[0] = other.m_rgb[0];
    m_rgb[1] = other.m_rgb[1];
    m_rgb[2] = other.m_rgb[2];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub::Color3ub(ColorIdent colorIdent)
{
    switch (colorIdent)
    {
        case RED:           set(255,   0,   0);  break;
        case GREEN:         set(  0, 255,   0);  break;
        case BLUE:          set(  0,   0, 255);  break;
        case YELLOW:        set(255, 255,   0);  break;
        case CYAN:          set(  0, 255, 255);  break;
        case MAGENTA:       set(255,   0, 255);  break;

        case WHITE:         set(255, 255, 255);  break;
        case BLACK:         set(  0,   0,   0);  break;
        case LIGHT_GRAY:    set(192, 192, 192);  break;
        case GRAY:          set(128, 128, 128);  break;
        case DARK_GRAY:     set( 64,  64,  64);  break;

        case BROWN:         set(165,  42,  42);  break;
        case CRIMSON:       set(220,  20,  60);  break;
        case DARK_BLUE:     set(  0,   0, 139);  break;
        case DARK_YELLOW:   set(139, 139,   0);  break;
        case DARK_CYAN:     set(  0, 139, 139);  break;
        case DARK_GREEN:    set(  0, 100,   0);  break;
        case DARK_MAGENTA:  set(139,   0, 139);  break;
        case DARK_ORANGE:   set(255, 140,   0);  break;
        case DARK_RED:      set(139,   0,   0);  break;
        case DARK_VIOLET:   set(148,   0, 211);  break;
        case DEEP_PINK:     set(255,  20, 147);  break;
        case FOREST_GREEN:  set( 34, 139,  34);  break;
        case GOLD:          set(255, 215,   0);  break;
        case GREEN_YELLOW:  set(173, 255,  47);  break;
        case INDIGO:        set( 75,   0, 130);  break;
        case OLIVE:         set(128, 128,   0);  break;
        case ORANGE:        set(255, 165,   0);  break;
        case ORANGE_RED:    set(255,  69,   0);  break;
        case ORCHID:        set(218, 112, 214);  break;
        case PINK:          set(255, 192, 203);  break;
        case PURPLE:        set(128,   0, 128);  break;
        case SEA_GREEN:     set( 46, 139,  87);  break;
        case SKY_BLUE:      set(135, 206, 235);  break;
        case VIOLET:        set(238, 130, 238);  break;
        case YELLOW_GREEN:  set(154, 205,  50);  break;
        case CEETRON:       set( 81, 134, 148);  break;

        default:            set(0, 0, 0);
                            CVF_FAIL_MSG("Unknown ColorIdent");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub::Color3ub(const Color3f& other)
{
    m_rgb[0] = static_cast<ubyte>(other.r()*255.0f);
    m_rgb[1] = static_cast<ubyte>(other.g()*255.0f);
    m_rgb[2] = static_cast<ubyte>(other.b()*255.0f);
}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
Color3ub& Color3ub::operator=(const Color3ub& rhs)
{
    m_rgb[0] = rhs.m_rgb[0];
    m_rgb[1] = rhs.m_rgb[1];
    m_rgb[2] = rhs.m_rgb[2];

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Assignment from ColorIdent 
//--------------------------------------------------------------------------------------------------
Color3ub& Color3ub::operator=(ColorIdent colorIdent)
{
    return operator=(Color3ub(colorIdent));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Color3ub::operator==(const Color3ub& rhs) const
{
    if (m_rgb[0] == rhs.m_rgb[0] &&
        m_rgb[1] == rhs.m_rgb[1] &&
        m_rgb[2] == rhs.m_rgb[2])
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
bool Color3ub::operator!=(const Color3ub& rhs) const
{
    return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Sets all color components
//--------------------------------------------------------------------------------------------------
void Color3ub::set(ubyte r, ubyte g, ubyte b)
{
    m_rgb[0] = r;
    m_rgb[1] = g;
    m_rgb[2] = b;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ubyte* Color3ub::ptr() const
{
    return m_rgb;
}

} // namespace cvf

