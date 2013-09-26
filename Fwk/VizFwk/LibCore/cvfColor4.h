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

#include "cvfColor3.h"

namespace cvf {

class Color4ub;

//==================================================================================================
//
// Class for RGBA colors
//
//==================================================================================================
class Color4f
{
public:
    Color4f();
    Color4f(float r, float g, float b, float alpha);
    Color4f(const Color4f& other);
    Color4f(const Color3f& rgbColor, float alpha);
    explicit Color4f(const Color3f& rgbColor);
    explicit Color4f(Color3::ColorIdent colorIdent);
    explicit Color4f(const Color4ub& other);

    Color4f&        operator=(const Color4f& rhs);
    bool            operator==(const Color4f& rhs) const;
    bool            operator!=(const Color4f& rhs) const;

    const float&    r() const;
    const float&    g() const;
    const float&    b() const;
    const float&    a() const;
    float&          r();
    float&          g();
    float&          b();
    float&          a();

    void            set(float r, float g, float b, float alpha);
    void            set(const Color3f& rgbColor, float alpha);
    void            set(const Color3f& rgbColor);

    bool            isValid() const;
    const float*    ptr() const;

    Color3f         toColor3f() const;

private:
    float m_rgba[4];
};


//==================================================================================================
//
// Unsigned byte RGBA colors
//
//==================================================================================================
class Color4ub 
{
public:
    Color4ub();
    Color4ub(ubyte r, ubyte g, ubyte b, ubyte a);
    Color4ub(const Color4ub& other);
    Color4ub(const Color3ub& rgbColor, ubyte a);
    explicit Color4ub(const Color3ub& rgbColor);
    explicit Color4ub(Color3::ColorIdent colorIdent);
    explicit Color4ub(const Color4f& other);

    Color4ub&       operator=(const Color4ub& rhs);
    bool            operator==(const Color4ub& rhs) const;
    bool            operator!=(const Color4ub& rhs) const;

    ubyte           r() const;
    ubyte           g() const;
    ubyte           b() const;
    ubyte           a() const;
    ubyte&          r();
    ubyte&          g();
    ubyte&          b();
    ubyte&          a();
    void            set(ubyte r, ubyte g, ubyte b, ubyte a);

    const ubyte*    ptr() const;

private:
    ubyte m_rgba[4];
};

}
