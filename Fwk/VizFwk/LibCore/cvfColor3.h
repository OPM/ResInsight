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

#include "cvfBase.h"


namespace cvf {

// Forward declarations
class Color3ub;



//==================================================================================================
//
// Abstract base class for RGB colors
//
//==================================================================================================
class Color3
{
public:
    enum ColorIdent
    {
        RED,			///< Pure red (255, 0, 0)
        GREEN,			///< Pure greed (0, 255, 0)
        BLUE,			///< Pure blue (0, 0, 255)
        YELLOW,		    ///< Yellow (255, 255, 0)
        CYAN,			///< Cyan (0, 255, 255)
        MAGENTA,		///< Magenta (255, 0, 255)

        WHITE,			///< White
        BLACK,			///< Black
        LIGHT_GRAY,		///< Light gray (192, 192, 192)
        GRAY,			///< Gray (128, 128, 128)
        DARK_GRAY,		///< Dark gray (64, 64, 64)

        BROWN,			///< Brown (165, 42, 42)
        CRIMSON,		///< Crimson (220, 20, 60)
        DARK_BLUE,		///< Dark blue (0, 0, 139)
        DARK_YELLOW,    ///< Dark yellow (139, 139, 0)
        DARK_CYAN,		///< Dark cyan (0, 139, 139)
        DARK_GREEN,		///< Dark green (0, 100, 0)
        DARK_MAGENTA,	///< Dark magenta (139, 0, 139)
        DARK_ORANGE,	///< Dark orange (255, 140, 0)
        DARK_RED,		///< Dark red (139, 0, 0)
        DARK_VIOLET,	///< Dark violet (148, 0, 211)
        DEEP_PINK,		///< Deep pink (255, 20, 147)
        FOREST_GREEN,	///< Forest green (34, 139, 34)
        GOLD,			///< Gold (255, 215, 0)
        GREEN_YELLOW,   ///< Green yellow (173, 255, 47)
        INDIGO,		    ///< Indigo (75, 0, 130)
        OLIVE,			///< Olive (128, 128, 0)
        ORANGE,		    ///< Orange (255, 165, 0)
        ORANGE_RED,		///< Orange red (255, 69, 0)
        ORCHID,		    ///< Orchid (218, 112, 214)
        PINK,			///< Pink (255, 192, 203)
        PURPLE,		    ///< Purple (128, 0, 128)
        SEA_GREEN,		///< Sea green (46, 139, 87)
        SKY_BLUE,		///< Sky blue (135, 206, 235)
        VIOLET,		    ///< Violet (238, 130, 238)
        YELLOW_GREEN, 	///< Yellow green (154, 205, 50)

        CEETRON         ///< Ceetron Color (81, 134, 148)
    };
};


//==================================================================================================
//
// Floating point RGB colors
//
//==================================================================================================
class Color3f : public Color3
{
public:
    Color3f();
    Color3f(float r, float g, float b);
    Color3f(const Color3f& other);
    Color3f(ColorIdent colorIdent);
    explicit Color3f(const Color3ub& other);

    Color3f&        operator=(const Color3f& rhs);
    Color3f&        operator=(ColorIdent colorIdent);
    bool            operator==(const Color3f& rhs) const;
    bool            operator!=(const Color3f& rhs) const;

    const float&    r() const           { return m_rgb[0]; }        ///< Returns the red color component
    const float&    g() const           { return m_rgb[1]; }        ///< Returns the green color component
    const float&    b() const           { return m_rgb[2]; }        ///< Returns the blue color component
    float&          r()                 { return m_rgb[0]; }        ///< Get modifiable reference to the red color component, used for setting the component.
    float&          g()                 { return m_rgb[1]; }        ///< Get modifiable reference to the green color component, used for setting the component
    float&          b()                 { return m_rgb[2]; }        ///< Get modifiable reference to the blue color component, used for setting the component

    void            set(float r, float g, float b);

    bool            isValid() const;
    const float*    ptr() const;

    ubyte           rByte() const;
    ubyte           gByte() const;
    ubyte           bByte() const;

    static Color3f  fromByteColor(ubyte r, ubyte g, ubyte b);

    friend bool operator < (const Color3f& color1, const Color3f& color2);

private:
    float m_rgb[3];
};

bool operator < (const Color3f& color1, const Color3f& color2);


//==================================================================================================
//
// Unsigned byte RGB colors
//
//==================================================================================================
class Color3ub : public Color3
{
public:
    Color3ub();
    Color3ub(ubyte r, ubyte g, ubyte b);
    Color3ub(const Color3ub& other);
    Color3ub(ColorIdent colorIdent);
    explicit Color3ub(const Color3f& other);

    Color3ub&       operator=(const Color3ub& rhs);
    Color3ub&       operator=(ColorIdent colorIdent);
    bool            operator==(const Color3ub& rhs) const;
    bool            operator!=(const Color3ub& rhs) const;

    ubyte           r() const           { return m_rgb[0]; }        ///< Returns the red color component
    ubyte           g() const           { return m_rgb[1]; }        ///< Returns the green color component
    ubyte           b() const           { return m_rgb[2]; }        ///< Returns the blue color component
    ubyte&          r()                 { return m_rgb[0]; }        ///< Get modifiable reference to the red color component.
    ubyte&          g()                 { return m_rgb[1]; }        ///< Get modifiable reference to the green color component.
    ubyte&          b()                 { return m_rgb[2]; }        ///< Get modifiable reference to the blue color component.

    void            set(ubyte r, ubyte g, ubyte b);

    const ubyte*    ptr() const;

private:
    ubyte m_rgb[3];
};

}

