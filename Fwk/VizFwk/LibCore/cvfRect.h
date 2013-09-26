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
#include "cvfVector2.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
template <typename T>
class Rect
{
public:
    Rect();
    Rect(T minX, T minY, T width, T height);
    Rect(const Vector2<T>& min, T width, T height);
    Rect(const Rect& rect);

    Rect&   operator=(const Rect& rhs);

    Vector2<T>  min() const;
    Vector2<T>  max() const;
    T           width() const;
    T           height() const;
    Vector2<T>  center() const;

    void        setMin(const Vector2<T>& min);
    void        setWidth(T width);
    void        setHeight(T height);

    bool        isValid() const;
    void        normalize();

    void        include(const Vector2<T>& coord);
    void        include(const Rect& rect);

    bool        contains(const Vector2<T>& coord) const;
    bool        intersects(const Rect& rect) const;

    void        translate(const Vector2<T>& offset);

    bool        segmentIntersect(const Vec2d& p1, const Vec2d& p2, Vec2d* intersect1, Vec2d* intersect2);

    static Rect fromMinMax(const Vector2<T>& min, const Vector2<T>& max);

private:
    static bool clipTest(double p, double q, double *u1, double *u2);

private:
    Vector2<T>  m_minPos;               ///< Position of left lower corner
    T           m_width;                ///< Width
    T           m_height;               ///< Height
};

typedef Rect<float>     Rectf;  ///< A rect with float components
typedef Rect<double>    Rectd;  ///< A rect with double components
typedef Rect<int>       Recti;  ///< A rect with integer components
typedef Rect<uint>      Rectui; ///< A rect with unsigned integer components

}

#include "cvfRect.inl"
