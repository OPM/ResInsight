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


namespace cvf {


//==================================================================================================
///
/// \class cvf::Rect
/// \ingroup Core
///
/// 
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Rect<T>::Rect()
{
    m_minPos.setZero();
    m_width = 0;
    m_height = 0;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Rect<T>::Rect(T minX, T minY, T width, T height)
{
    m_minPos.set(minX, minY);
    m_width = width;
    m_height = height;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Rect<T>::Rect(const Vector2<T>& min, T width, T height)
{
    m_minPos = min;
    m_width = width;
    m_height = height;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Rect<T>::Rect(const Rect& rect)
{
    m_minPos = rect.min();
    m_width = rect.width();
    m_height = rect.height();
}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
template<typename T>
Rect<T>& Rect<T>::operator=(const Rect& rhs)
{
    m_minPos = rhs.min();
    m_width = rhs.max().x() - rhs.min().x();
    m_height = rhs.max().y() - rhs.min().y();

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Vector2<T> Rect<T>::min() const
{
    return m_minPos;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Vector2<T> Rect<T>::max() const
{
    Vector2<T> max(m_minPos);
    max.x() += m_width;
    max.y() += m_height;

    return max;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
T Rect<T>::width() const
{
    return m_width;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
T Rect<T>::height() const
{
    return m_height;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Vector2<T> Rect<T>::center() const
{
    Vector2<T> center(m_minPos);
    center.x() += m_width / 2.0;
    center.y() += m_height / 2.0;

    return center;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void Rect<T>::setMin(const Vector2<T>& min)
{
    m_minPos = min;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void Rect<T>::setWidth(T width)
{
    m_width = width;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void Rect<T>::setHeight(T height)
{
    m_height = height;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
bool Rect<T>::isValid() const
{
    return (m_width > 0.0) && (m_height > 0.0);
}


//--------------------------------------------------------------------------------------------------
/// Normalizes the rectangle
/// 
/// Ensures that the rectangle has a non-negative width and height. If width or height is negative,
/// the corresponding min component will be moved.
//--------------------------------------------------------------------------------------------------
template <typename T>
void Rect<T>::normalize()
{
    if (m_width < 0.0)
    {
        m_width = -m_width;
        m_minPos.x() -= m_width;
    }

    if (m_height < 0.0)
    {
        m_height = -m_height;
        m_minPos.y() -= m_height;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void Rect<T>::include(const Vector2<T>& coord)
{
    if (!isValid())
    {
        Vector2<T> diff = coord - m_minPos;
        m_width = diff.x();
        m_height = diff.y();
        return;
    }

    if (coord.x() < m_minPos.x())
    {
        T deltaX = m_minPos.x() - coord.x();
        m_minPos.x() -= deltaX;
        m_width += deltaX;
    }
    else if (coord.x() > m_minPos.x() + m_width)
    {
        T deltaX = coord.x() - (m_minPos.x() + m_width);
        m_width += deltaX;
    }

    if (coord.y() < m_minPos.y())
    {
        T deltaY = m_minPos.y() - coord.y();
        m_minPos.y() -= deltaY;
        m_height += deltaY;
    }
    else if (coord.y() > m_minPos.y() + m_height)
    {
        T deltaY = coord.y() - (m_minPos.y() + m_height);
        m_height += deltaY;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void Rect<T>::include(const Rect& rect)
{
    if (!rect.isValid()) return;

    T left = m_minPos.x();
    T right = m_minPos.x();
    if (m_width < 0.0)
    {
        left += m_width;
    }
    else
    {
        right += m_width;
    }

    if (rect.width() < 0.0) 
    {
        left = CVF_MIN(left, rect.min().x() + rect.width());
        right = CVF_MAX(right, rect.min().x());
    } 
    else 
    {
        left = CVF_MIN(left, rect.min().x());
        right = CVF_MAX(right, rect.min().x() + rect.width());
    }

    T bottom = m_minPos.y();
    T top = m_minPos.y();
    if (m_height < 0.0)
    {
        bottom += m_height;
    }
    else
    {
        top += m_height;
    }

    if (rect.height() < 0.0) 
    {
        bottom = CVF_MIN(bottom, rect.min().y() + rect.height());
        top = CVF_MAX(top, rect.min().y());
    } 
    else 
    {
        bottom = CVF_MIN(bottom, rect.min().y());
        top = CVF_MAX(top, rect.min().y() + rect.height());
    }

    m_minPos.set(left, bottom);
    m_width = right - left;
    m_height = top - bottom;
}


//--------------------------------------------------------------------------------------------------
/// Check if the rectangle contains the specified coordinate
/// 
/// Returns true if the point is inside or on the edge of the rectangle; otherwise returns false.
//--------------------------------------------------------------------------------------------------
template <typename T>
bool Rect<T>::contains(const Vector2<T>& coord) const
{
    T left = m_minPos.x();
    T right = m_minPos.x();
    if (m_width < 0.0)
    {
        left += m_width;
    }
    else
    {
        right += m_width;
    }

    if (left == right) 
    {
        // null rect
        return false;
    }

    if (coord.x() < left || coord.x() > right)
    {
        return false;
    }

    T bot = m_minPos.y();
    T top = m_minPos.y();
    if (m_height < 0.0)
    {
        bot += m_height;
    }
    else
    {
        top += m_height;
    }

    if (bot == top) 
    {
        // null rect
        return false;
    }

    if (coord.y() < bot || coord.y() > top)
    {
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
bool Rect<T>::intersects(const Rect& rect) const
{
    T left1 = m_minPos.x();
    T right1 = m_minPos.x();
    if (m_width < 0.0)
    {
        left1 += m_width;
    }
    else
    {
        right1 += m_width;
    }

    if (left1 == right1) 
    {
        // null rect
        return false;
    }

    T left2 = rect.min().x();
    T right2 = rect.min().x();
    if (rect.width() < 0.0)
    {
        left2 += rect.width();
    }
    else
    {
        right2 += rect.width();
    }

    if (left2 == right2) 
    {
        // null rect
        return false;
    }

    if (left1 >= right2 || left2 >= right1)
    {
        return false;
    }

    T bot1 = m_minPos.y();
    T top1 = m_minPos.y();
    if (m_height < 0.0)
    {
        bot1 += m_height;
    }
    else
    {
        top1 += m_height;
    }

    if (bot1 == top1) 
    {
        // null rect
        return false;
    }

    T bot2 = rect.min().y();
    T top2 = rect.min().y();
    if (rect.height() < 0.0)
    {
        bot2 += rect.height();
    }
    else
    {
        top2 += rect.height();
    }

    if (bot2 == top2) 
    {
        // null rect
        return false;
    }

    if (bot1 >= top2 || bot2 >= top1)
    {
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void Rect<T>::translate(const Vector2<T>& offset)
{
    m_minPos += offset;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
Rect<T> Rect<T>::fromMinMax(const Vector2<T>& min, const Vector2<T>& max)
{
    // Enforce min/max - otherwise we'll get bogus results for unsigned types
    CVF_ASSERT(min.x() <= max.x());
    CVF_ASSERT(min.y() <= max.y());

    Rect<T> rect(min, max.x() - min.x(), max.y() - min.y());
    return rect;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
bool Rect<T>::segmentIntersect(const Vec2d& p1, const Vec2d& p2, Vec2d* intersect1, Vec2d* intersect2)
{
    // Uses Liang-Barsky line clipping algorithm
    // See http://stackoverflow.com/questions/99353/how-to-test-if-a-line-segment-intersects-an-axis-aligned-rectange-in-2d
    CVF_ASSERT(intersect1);
    CVF_ASSERT(intersect2);

    if (!isValid()) return false;

    double u1 = 0.0;
    double u2 = 1.0;
    double dx = p2.x() - p1.x();
    double dy;

    Vec2d maxPos = max();

    if (clipTest(-dx, p1.x() - m_minPos.x(), &u1, &u2))
    {
        if (clipTest(dx, maxPos.x() - p1.x(), &u1, &u2))
        {
            dy = p2.y() - p1.y();
            if (clipTest (-dy, p1.y() - m_minPos.y(), &u1, &u2))
            {
                if (clipTest (dy, maxPos.y() - p1.y(), &u1, &u2))
                {
                    *intersect1 = p1;
                    *intersect2 = p2;
                    if (u2 < 1.0)
                    {
                        intersect2->x() = p1.x() + u2 * dx;
                        intersect2->y() = p1.y() + u2 * dy;
                    }
                    if (u1 > 0.0)
                    {
                        intersect1->x() += u1 * dx;
                        intersect1->y() += u1 * dy;
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
bool Rect<T>::clipTest(double p, double q, double *u1, double *u2)
{
    // Uses Liang-Barsky line clipping algorithm
    // See also http://stackoverflow.com/questions/99353/how-to-test-if-a-line-segment-intersects-an-axis-aligned-rectange-in-2d
    CVF_ASSERT(u1);
    CVF_ASSERT(u2);

    bool retval = true;

    if (p < 0.0)
    {
        double r = q / p;
        if (r > *u2)
        {
            retval = false;
        }
        else if (r > *u1)
        {
            *u1 = r;
        }
    }
    else if (p > 0.0)
    {
        double r = q / p;
        if (r < *u1)
        {
            retval = false;
        }
        else if (r < *u2)
        {
            *u2 = r;
        }
    }
    else
    {
        // p = 0, so line is parallel to this clipping edge 
        if(q < 0.0)
        {
            // Line is outside clipping edge
            retval = false;
        }
    }

    return retval;
}


}  // namespace cvf

