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
#include "cvfFrustum.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Frustum
/// \ingroup Geometry
///
/// Class defining a frustum
///
/// The frustum consists of 6 \link Plane Planes\endlink
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor
//--------------------------------------------------------------------------------------------------
Frustum::Frustum()
{

}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
Frustum::Frustum(const Frustum& other) 
:   Object()
{ 
    *this = other;
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
Frustum::~Frustum()
{

}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
const Frustum& Frustum::operator=(const Frustum& rhs)
{ 
    m_planes.clear();

    std::map<int, Plane>::const_iterator it;
    for (it = rhs.m_planes.begin(); it != rhs.m_planes.end(); it++)
    {
        m_planes[it->first] = it->second;
    }

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Equality operator
//--------------------------------------------------------------------------------------------------
bool Frustum::operator==(const Frustum& rhs) const
{
    if (m_planes.size() != rhs.m_planes.size()) return false;

    std::map<int, Plane>::const_iterator thisIt, rhsIt;

    size_t i;
    for (i = 0; i < m_planes.size(); i++)
    {
        int sideIdx = static_cast<int>(i);

        thisIt = m_planes.find(sideIdx);
        rhsIt = rhs.m_planes.find(sideIdx);

        if (thisIt->first != rhsIt->first) return false;
        if (thisIt->second != rhsIt->second) return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Inequality operator
//--------------------------------------------------------------------------------------------------
bool Frustum::operator!=(const Frustum& rhs) const
{
    return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Set one of the planes building up the frustum
//--------------------------------------------------------------------------------------------------
void Frustum::setPlane(Side side, const Plane& plane)
{
    CVF_ASSERT(side < COUNT);
    CVF_ASSERT(plane.isValid());

    m_planes[side] = plane;
}


//--------------------------------------------------------------------------------------------------
/// Get one given Plane of the frustum.
///
/// \return The Plane queried or an invalid Plane if it does not exist.
//--------------------------------------------------------------------------------------------------
Plane Frustum::plane(Side side) const
{
    std::map<int, Plane>::const_iterator it = m_planes.find(side);
    
    return (it != m_planes.end()) ? it->second : Plane();
}


//--------------------------------------------------------------------------------------------------
/// Transforms all planes in the frustum with the given matrix
//--------------------------------------------------------------------------------------------------
void Frustum::transform(const Mat4d& matrix)
{
    std::map<int, Plane>::iterator it;
    for (it = m_planes.begin(); it != m_planes.end(); it++)
    {
        it->second.transform(matrix);
    }    
}


//--------------------------------------------------------------------------------------------------
///  Test point to see if it's outside the frustum or not
///
/// \return true if outside or exactly on the boundary. false if inside.
//--------------------------------------------------------------------------------------------------
bool Frustum::isOutside(const Vec3d& point) const
{
    CVF_ASSERT(m_planes.size() == 6);

    std::map<int, Plane>::const_iterator it;
    for (it = m_planes.begin(); it != m_planes.end(); it++)
    {
        if (it->second.distanceSquared(point) < 0.0)
        {
            return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
///  Test bounding box to see if it's outside the frustum or not
///
/// \return true if outside or exactly on the boundary. false if inside.
//--------------------------------------------------------------------------------------------------
bool Frustum::isOutside(const BoundingBox& bbox) const
{
    CVF_ASSERT(m_planes.size() == 6);

    const Vec3d& boxMin = bbox.min();
    const Vec3d& boxMax = bbox.max();

    Vec3d point;
    Vec3d planeNormal;

    std::map<int, Plane>::const_iterator it;
    for (it = m_planes.begin(); it != m_planes.end(); it++)
    {
        planeNormal = it->second.normal();

        point.x() = (planeNormal.x() <= 0.0) ? boxMin.x() : boxMax.x();
        point.y() = (planeNormal.y() <= 0.0) ? boxMin.y() : boxMax.y();
        point.z() = (planeNormal.z() <= 0.0) ? boxMin.z() : boxMax.z();

        if (it->second.distanceSquared(point) < 0.0)
        {
            return true;
        }
    }

    return false;
}


} // namespace cvf

