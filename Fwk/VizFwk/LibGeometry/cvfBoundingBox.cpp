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
#include "cvfBoundingBox.h"
#include "cvfString.h"

#include <limits>

namespace cvf {



//==================================================================================================
///
/// \class cvf::BoundingBox
/// \ingroup Geometry
///
/// The BoundingBox class implements an axis-aligned bounding box.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox::BoundingBox()
{
    reset();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox::BoundingBox(const Vec3d& min, const Vec3d& max)
    : m_min(min), m_max(max)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox::BoundingBox(const Vec3f& min, const Vec3f& max)
    : m_min(min), m_max(max)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox::BoundingBox(const BoundingBox& other)
:   m_min(other.m_min),
    m_max(other.m_max)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox& BoundingBox::operator=(const BoundingBox& rhs)
{
    m_min = rhs.m_min;
    m_max = rhs.m_max;

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Initialize bounding box
//--------------------------------------------------------------------------------------------------
void BoundingBox::reset()
{
    const double maxDouble = std::numeric_limits<double>::max();

    m_max.set(-maxDouble, -maxDouble, -maxDouble);
    m_min.set( maxDouble,  maxDouble,  maxDouble);
}


//--------------------------------------------------------------------------------------------------
/// Returns false if no input has been given
//--------------------------------------------------------------------------------------------------
bool BoundingBox::isValid() const
{
    if (m_min.x() <= m_max.x() && 
        m_min.y() <= m_max.y() &&
        m_min.z() <= m_max.z())
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
void BoundingBox::add(const Vec3d& point)
{
    if (point.x() < m_min.x()) m_min.x() = point.x();
    if (point.y() < m_min.y()) m_min.y() = point.y();
    if (point.z() < m_min.z()) m_min.z() = point.z();

    if (point.x() > m_max.x()) m_max.x() = point.x();
    if (point.y() > m_max.y()) m_max.y() = point.y();
    if (point.z() > m_max.z()) m_max.z() = point.z();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoundingBox::add(const Vec3f& point)
{
    if (point.x() < m_min.x()) m_min.x() = point.x();
    if (point.y() < m_min.y()) m_min.y() = point.y();
    if (point.z() < m_min.z()) m_min.z() = point.z();

    if (point.x() > m_max.x()) m_max.x() = point.x();
    if (point.y() > m_max.y()) m_max.y() = point.y();
    if (point.z() > m_max.z()) m_max.z() = point.z();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoundingBox::add(const Vec3dArray& points)
{
    size_t i;
    for (i = 0; i < points.size(); i++)
    {
        add(points[i]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoundingBox::add(const Vec3fArray& points)
{
    size_t i;
    for (i = 0; i < points.size(); i++)
    {
        add(points[i]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoundingBox::add(const BoundingBox& bb)
{
    if (bb.isValid())
    {
        add(bb.min());
        add(bb.max());
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void BoundingBox::addValid(const BoundingBox& bb)
{
	if (bb.m_min.x() < m_min.x()) m_min.x() = bb.m_min.x();
	if (bb.m_min.y() < m_min.y()) m_min.y() = bb.m_min.y();
	if (bb.m_min.z() < m_min.z()) m_min.z() = bb.m_min.z();

	if (bb.m_max.x() > m_max.x()) m_max.x() = bb.m_max.x();
	if (bb.m_max.y() > m_max.y()) m_max.y() = bb.m_max.y();
	if (bb.m_max.z() > m_max.z()) m_max.z() = bb.m_max.z();
}

//--------------------------------------------------------------------------------------------------
/// Computes center of the bounding box
//--------------------------------------------------------------------------------------------------
Vec3d BoundingBox::center() const
{
    CVF_TIGHT_ASSERT(isValid());

    return (m_min + m_max) / 2.0;
}


//--------------------------------------------------------------------------------------------------
/// Computes the total size of the bounding box
//--------------------------------------------------------------------------------------------------
Vec3d BoundingBox::extent() const
{
    CVF_TIGHT_ASSERT(isValid());

    return (m_max - m_min);
}


//--------------------------------------------------------------------------------------------------
/// Compute radius as half the length of the box's diagonal
//--------------------------------------------------------------------------------------------------
double BoundingBox::radius() const
{
    CVF_TIGHT_ASSERT(isValid());

    return extent().length() / 2.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3d& BoundingBox::min() const
{
    CVF_TIGHT_ASSERT(isValid());

    return m_min;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3d& BoundingBox::max() const
{
    CVF_TIGHT_ASSERT(isValid());

    return m_max;
}


//--------------------------------------------------------------------------------------------------
/// Check if the bounding box contains the specified point
/// 
/// Note that a point on the box's surface is classified as being contained
//--------------------------------------------------------------------------------------------------
bool BoundingBox::contains(const Vec3d& point) const
{
    CVF_TIGHT_ASSERT(isValid());

    if (point.x() >= m_min.x() && point.x() <= m_max.x() &&
        point.y() >= m_min.y() && point.y() <= m_max.y() &&
        point.z() >= m_min.z() && point.z() <= m_max.z())
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
bool BoundingBox::intersects(const BoundingBox& box) const
{
    CVF_TIGHT_ASSERT(isValid());
    CVF_TIGHT_ASSERT(box.isValid());

    if (m_max.x() < box.m_min.x() || m_min.x() > box.m_max.x())  return false;
    if (m_max.y() < box.m_min.y() || m_min.y() > box.m_max.y())  return false;
    if (m_max.z() < box.m_min.z() || m_min.z() > box.m_max.z())  return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Get corner points of box
/// 
/// \param	corners  Array of Vec3d. Must be allocated 8 vectors long.
/// 
/// <PRE>
///        7---------6                
///       /|        /|     |z       
///      / |       / |     | / y    
///     4---------5  |     |/       
///     |  3------|--2     *---x    
///     | /       | /           
///     |/        |/            
///     0---------1    </PRE>
///
//--------------------------------------------------------------------------------------------------
void BoundingBox::cornerVertices(Vec3d corners[8]) const
{
    corners[0].set(m_min.x(), m_min.y(), m_min.z());
    corners[1].set(m_max.x(), m_min.y(), m_min.z());
    corners[2].set(m_max.x(), m_max.y(), m_min.z());
    corners[3].set(m_min.x(), m_max.y(), m_min.z());
    corners[4].set(m_min.x(), m_min.y(), m_max.z());
    corners[5].set(m_max.x(), m_min.y(), m_max.z());
    corners[6].set(m_max.x(), m_max.y(), m_max.z());
    corners[7].set(m_min.x(), m_max.y(), m_max.z());
}


//--------------------------------------------------------------------------------------------------
/// Expands the bounding box by the given amount in all three directions
/// 
/// If a bounding box is expanded by 2, the bounding box's size will increase by 2 in each direction
/// \sa extent()
//--------------------------------------------------------------------------------------------------
void BoundingBox::expand(double amount)
{
    double half = amount/2;
    m_min.x() -= half;
    m_min.y() -= half;
    m_min.z() -= half;
    m_max.x() += half;
    m_max.y() += half;
    m_max.z() += half;
}


//--------------------------------------------------------------------------------------------------
/// Transform the min and max coordinate with the given transformation matrix
//--------------------------------------------------------------------------------------------------
void BoundingBox::transform(const Mat4d& matrix)
{
	// Check if box is invalid, and don't transform if so
	if (!isValid()) return;

	BoundingBox newBox;
    newBox.reset();

    Vec3d node;
    
    node.set(m_min.x(), m_min.y(), m_min.z());
    node.transformPoint(matrix);
    newBox.add(node);

    node.set(m_max.x(), m_min.y(), m_min.z());
    node.transformPoint(matrix);
    newBox.add(node);

    node.set(m_max.x(), m_max.y(), m_min.z());
    node.transformPoint(matrix);
    newBox.add(node);

    node.set(m_min.x(), m_max.y(), m_min.z());
    node.transformPoint(matrix);
    newBox.add(node);

    node.set(m_min.x(), m_min.y(), m_max.z());
    node.transformPoint(matrix);
    newBox.add(node);

    node.set(m_max.x(), m_min.y(), m_max.z());
    node.transformPoint(matrix);
    newBox.add(node);

    node.set(m_max.x(), m_max.y(), m_max.z());
    node.transformPoint(matrix);
    newBox.add(node);

    node.set(m_min.x(), m_max.y(), m_max.z());
    node.transformPoint(matrix);
    newBox.add(node);

	*this = newBox;
}


//--------------------------------------------------------------------------------------------------
/// Returns this BoundingBox transformed with the given transformation matrix
//--------------------------------------------------------------------------------------------------
const BoundingBox BoundingBox::getTransformed(const Mat4d& matrix) const
{
    BoundingBox box(*this);
    box.transform(matrix);

    return box;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String BoundingBox::debugString() const
{
    String str = "BoundingBox:";
    str += " min: x=" + String::number(m_min.x()) + " y=" + String::number(m_min.y()) + " z=" + String::number(m_min.z());
    str += " max: x=" + String::number(m_max.x()) + " y=" + String::number(m_max.y()) + " z=" + String::number(m_max.z());
    return str;
}


} // namespace cvf

