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
#include "cvfPlane.h"
#include "cvfMath.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Plane
/// \ingroup Core
///
/// Class defining a plane in space
///
/// The class describes a plane by the equation: \f$Ax + By + Cz + D = 0\f$
/// The plane's normal is defined by the coefficients \f$[A, B, C]\f$
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor
///
/// The plane will be set to invalid
//--------------------------------------------------------------------------------------------------
Plane::Plane()
{
    // Invalidate plane coefficients 
    // Note: Coefficients set to zero will still comply with the line equation but
    //       the normal is of zero length which is considered an invalid state in this class.
    m_A = m_B = m_C = m_D = 0.0;
}


//--------------------------------------------------------------------------------------------------
/// Constructor with plane equation coefficients
///
/// Define a plane by the plane equation coefficients.
/// Setting at least the A, B and C coefficients to zero will effectually set the plane to invalid.
//--------------------------------------------------------------------------------------------------
Plane::Plane(double A, double B, double C, double D)
{ 
    m_A = A;
    m_B = B;
    m_C = C;
    m_D = D;
}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
Plane::Plane(const Plane& other) 
:   Object()
{ 
    *this = other;
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
Plane::~Plane()
{

}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
const Plane& Plane::operator=(const Plane& rhs)
{ 
    m_A = rhs.m_A; 
    m_B = rhs.m_B; 
    m_C = rhs.m_C; 
    m_D = rhs.m_D; 

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Equality operator
///
/// Check if the two planes are identically defined. Note, equal planes may be defined differently
/// and are as such considered not to be identical to each other. (Different plane coefficients)
//--------------------------------------------------------------------------------------------------
bool Plane::operator==(const Plane& rhs) const
{
    if (m_A != rhs.m_A) return false;
    if (m_B != rhs.m_B) return false;
    if (m_C != rhs.m_C) return false;
    if (m_D != rhs.m_D) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Inequality operator
///
/// Check if the two planes are not identically defined. Note, equal planes may be defined differently
/// and are as such considered not to be identical to each other. (Different plane coefficients)
//--------------------------------------------------------------------------------------------------
bool Plane::operator!=(const Plane& rhs) const
{
    return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Is the plane definition valid or not
///
/// The plane is considered invalid if the normal is of zero length
//--------------------------------------------------------------------------------------------------
bool Plane::isValid() const
{
    return !((m_A == 0.0) && (m_B == 0.0) && (m_C == 0.0));
}


//--------------------------------------------------------------------------------------------------
/// Define a plane by the plane equation coefficients
///
/// \param     A   Plane coefficient A
/// \param     B   Plane coefficient B
/// \param     C   Plane coefficient C
/// \param     D   Plane coefficient D
///
/// The coefficients defines a plane by the equation: \f$Ax + By + Cz + D = 0\f$
/// The plane coefficients \f$[A, B, C]\f$ defines the normal to the plane
//--------------------------------------------------------------------------------------------------
void Plane::set(double A, double B, double C, double D)
{ 
    CVF_ASSERT(!(A == 0.0 && B == 0.0 && C == 0.0));

    m_A = A; 
    m_B = B; 
    m_C = C; 
    m_D = D; 
}


//--------------------------------------------------------------------------------------------------
/// Compute the plane equation from the given point and normal
///
/// \param     point   Point on the plane
/// \param     normal  The plane's normal
//--------------------------------------------------------------------------------------------------
bool Plane::setFromPointAndNormal(const Vec3d& point, const Vec3d& normal)
{
    if (normal.isZero()) return false;

    m_A = normal.x();
    m_B = normal.y();
    m_C = normal.z();
    m_D = -(normal*point);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Compute the plane equation from the given three points
///
/// \param     p1   First point on the plane
/// \param     p2   Second point on the plane
/// \param     p3   Third point on the plane
///
/// \return true if successfully set. false if points are on the same line.
///
/// The three points must be different from each other and cannot be on the same line in space
//--------------------------------------------------------------------------------------------------
bool Plane::setFromPoints(const Vec3d& p1, const Vec3d& p2, const Vec3d& p3)
{
    Vec3d v1 = p2 - p1;
    Vec3d v2 = p3 - p1;
    Vec3d normal = v1 ^ v2;

    if (normal.isZero()) return false;

    setFromPointAndNormal(p1, normal);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Get the normal of the plane
///
/// The normal may not be normalized
//--------------------------------------------------------------------------------------------------
Vec3d Plane::normal() const
{
    Vec3d normal = Vec3d(m_A, m_B, m_C);
    CVF_ASSERT(!normal.isZero());

    return normal;
}


//--------------------------------------------------------------------------------------------------
/// Get a point located on the plane
//--------------------------------------------------------------------------------------------------
Vec3d Plane::pointInPlane() const
{
    double distToOrigin = distanceToOrigin();
    Vec3d point(m_A, m_B, m_C);
    point *= distToOrigin;

    return point;
}


//--------------------------------------------------------------------------------------------------
/// Flip the plane
//--------------------------------------------------------------------------------------------------
void Plane::flip()
{
    m_A *= -1.0;
    m_B *= -1.0;
    m_C *= -1.0;
    m_D *= -1.0;
}


//--------------------------------------------------------------------------------------------------
/// Transforms the plane with the given homogeneous transformation \a matrix
//--------------------------------------------------------------------------------------------------
void Plane::transform(const Mat4d& matrix)
{
    Vec3d n = normal();
    Vec3d point = pointInPlane();

    n.transformVector(matrix);
    point.transformPoint(matrix);

    setFromPointAndNormal(point, n);
}


//--------------------------------------------------------------------------------------------------
/// Get the distance from the point to the plane
//--------------------------------------------------------------------------------------------------
double Plane::distance(const Vec3d& point) const
{
    double factor = Math::sqrt(m_A*m_A + m_B*m_B + m_C*m_C);
    CVF_ASSERT(factor > 0.0);

    double dist = distanceSquared(point) / factor;

    return dist;
}


//--------------------------------------------------------------------------------------------------
/// Get the square of the distance from the point to the plane
///
/// The square of the distance is relatively fast to compute (no \a sqrt) and is useful for determine
/// which side the point is on. To obtain the actual distance, divide by \f$\sqrt{(A^2 + B^2 + C^2)}\f$
/// or use the distance() function directly.
//--------------------------------------------------------------------------------------------------
double Plane::distanceSquared(const Vec3d& point) const
{
    return  m_A*point.x() + m_B*point.y() + m_C*point.z() + m_D;
}


//--------------------------------------------------------------------------------------------------
/// Get the distance from the plane to the origin
//--------------------------------------------------------------------------------------------------
double Plane::distanceToOrigin() const
{
    double factor = m_A*m_A + m_B*m_B + m_C*m_C;
    CVF_ASSERT(factor > 0.0);

    return -m_D/factor;
}


//--------------------------------------------------------------------------------------------------
/// Project the given vector onto the plane
///
/// \param     vector   Vector to be projected
/// \param     projectedVector   Projected vector to be returned by pointer
/// 
/// \return true if successfully projected.
///         false if the given \a vector is parallel with the plane's normal
//--------------------------------------------------------------------------------------------------
bool Plane::projectVector(const Vec3d& vector, Vec3d* projectedVector) const
{
    CVF_ASSERT(projectedVector);

    Vec3d n = normal();
    Vec3d tmp = n ^ vector;
    Vec3d k = tmp ^ n;
    double length = k.length();

    if (length <= 0.0) return false;

    k *= 1.0/length;
    length = vector*k;
    *projectedVector = k;
    *projectedVector *= length; 

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Project the given point onto the plane
//--------------------------------------------------------------------------------------------------
Vec3d Plane::projectPoint(const Vec3d& point) const
{
    Vec3d pip = pointInPlane();
    
    // Create vector from point in plane to node
    Vec3d vector = point - pip;

    // Project vector to find node in plane
    Vec3d projectedPoint;
    if (projectVector(vector, &projectedPoint))
    {
        projectedPoint += pip;
    }
    else
    {
        // The <point - pip> vector is parallel with the normal vector, use the point in plane as the projected
        projectedPoint = pip;
    }

    return projectedPoint;
}


//--------------------------------------------------------------------------------------------------
/// Find intersection line between two planes
///
/// \param  other       The other plane to find intersection line with
/// \param  point       Point on line
/// \param  direction   Normalized direction of line
/// 
/// \return true if success. false if direction is zero -> no point of intersection exists 
//--------------------------------------------------------------------------------------------------
bool Plane::intersect(const Plane& other, Vec3d* point, Vec3d* direction) const
{
    // Note: Ripped from Graphics Gems III page 233.

    CVF_ASSERT(point);

    double invdet = UNDEFINED_DOUBLE;  

    Vec3d normal1 = this->normal();
    Vec3d normal2 = other.normal();
    Vec3d vector = normal1 ^ normal2;
    Vec3d dir2(vector.x()*vector.x(), vector.y()*vector.y(), vector.z()*vector.z());

    // Todo: VTFIsGreater(...) and VTFeqZero(..) replacement
    if ((dir2.z() > dir2.y()) && (dir2.z() > dir2.x()) && (dir2.z() != 0.0))
    {
        // then get a point on the XY plane 
        invdet = 1.0 / vector.z();

        // solve < pl1.x() * xpt.x() + pl1.y() * xpt.y() = - pl1.w > < pl2.x() * xpt.x() + pl2.y() * xpt.y() = - pl2.w > 
        point->x() = m_B*other.m_D - other.m_B*m_D;
        point->y() = other.m_A*m_D - m_A*other.m_D;
        point->z() = 0.0;
    }
    // Todo: VTFIsGreater(...) and VTFeqZero(..) replacement
    else if ((dir2.y() > dir2.x()) && (dir2.y() != 0.0))
    {
        // then get a point on the XZ plane 
        invdet = 1.0f / vector.y();

        // solve < pl1.x() * xpt.x() + pl1.z() * xpt.z() = -pl1.w > < pl2.x() * xpt.x() + pl2.z() * xpt.z() = -pl2.w >
        point->x() = other.m_C*m_D - m_C*other.m_D;
        point->y() = 0.0;
        point->z() = m_A*other.m_D - other.m_A*m_D;
    }
    // Todo: VTFeqZero(..) replacement
    else if (dir2.x() != 0.0)
    {
        // then get a point on the YZ plane 
        invdet = 1.0 / vector.x();

        // solve < pl1.y() * xpt.y() + pl1.z() * xpt.z() = - pl1.w > < pl2.y() * xpt.y() + pl2.z() * xpt.z() = - pl2.w > 
        point->x() = 0.0;
        point->y() = m_C*other.m_D - other.m_C*m_D;
        point->z() = other.m_B*m_D - m_B*other.m_D;
    }
    else 
    {
        // direction is zero, then no point of intersection exists
        return false;
    }

    *point *= invdet;

    if (direction)
    {
        *direction = vector;
        direction->normalize();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Find intersection between a line segment and a plane
///
/// \param a            Start of line segment
/// \param b            End of line segment
/// \param intersection Returns intersection point if not NULL
///
/// \return True if line segment intersects the plane
//--------------------------------------------------------------------------------------------------
bool Plane::intersect(const Vec3d& a, const Vec3d& b, Vec3d* intersection) const
{
    // From Real-Time Collision Detection by Christer Eriscon, published by Morgen Kaufmann Publishers, (c) 2005 Elsevier Inc

    // Compute the t value for the directed line ab intersecting the plane
    Vec3d ab = b - a;
    double t = (-m_D - (normal() * a)) / (normal() * ab);

    // If t in [0..1] compute and return intersection point
    if (t >= 0.0 && t <= 1.0)
    {
        if (intersection)
        {
            *intersection = a + t * ab;
        }

        return true;
    }

    return false;
}



//--------------------------------------------------------------------------------------------------
/// Clip a triangle against this plane
///
/// Clip the triangle given by parameters a, b and c against this plane. The vertices of the 
/// resulting clipped polygon (triangle or quad) will be returned in \a clippedPolygon. Since the
/// clipped polygon may be a quad, the \a clippedPolygon array must have room for at least 4 elements.
///
/// \return The number of resulting vertices that are populated in \a clippedPolygon. Will be 0, 3 or 4.
//--------------------------------------------------------------------------------------------------
size_t Plane::clipTriangle(const Vec3d& a, const Vec3d& b, const Vec3d& c, Vec3d clippedPolygon[4]) const
{
    // Except for the trivial cases where all vertices are in front 
    // or behind plane, these are the permutations
    //
    // Single vertex on positive side of plane 
    // => return a triangle                                
    //
    //  +\   /\c               /\c   /+          /\c        .
    //    \ /  \              /  \  /       +   /  \   +    .
    //     \    \            /    \/        ---/----\---    .
    //    / \    \          /     /\          /      \      .
    //  a/___\____\b      a/_____/__\b      a/________\b    .
    //       +\                 /+
    //
    // Two vertices vertex on positive side of plane 
    // => return a quad
    //
    //       /\c           \+  /\c               /\c  +/    .   
    //      /  \            \ /  \              /  \  /     .  
    //  ___/____\___         \    \            /    \/      .
    //  + /      \ +        / \    \          /     /\      .
    //  a/________\b      a/___\____\b      a/_____/__\b    .        
    //                          \+               +/

    bool onPosSide[3];
    onPosSide[0] = distanceSquared(a) >= 0 ? true : false;
    onPosSide[1] = distanceSquared(b) >= 0 ? true : false;
    onPosSide[2] = distanceSquared(c) >= 0 ? true : false;
    const int numPositiveVertices = (onPosSide[0] ? 1 : 0) + (onPosSide[1] ? 1 : 0) + (onPosSide[2] ? 1 : 0);
    
    // The entire triangle is on the negative side
    // Clip everything
    if (numPositiveVertices == 0)
    {
        return 0;
    }

    // All triangle vertices are on the positive side
    // Return the same triangle
    if (numPositiveVertices == 3) 
    {
        clippedPolygon[0] = a;
        clippedPolygon[1] = b;
        clippedPolygon[2] = c;
        return 3;
    }

    // Handle case where a single vertex is on the positive side
    // Will result in the return of a single clipped triangle
    if (numPositiveVertices == 1)
    {
        if (onPosSide[0])
        {
            clippedPolygon[0] = a;
            intersect(a, b, &clippedPolygon[1]);
            intersect(a, c, &clippedPolygon[2]);
        }
        else if (onPosSide[1])
        {
            clippedPolygon[0] = b;
            intersect(b, c, &clippedPolygon[1]);
            intersect(b, a, &clippedPolygon[2]);
        }
        else
        {
            CVF_ASSERT(onPosSide[2]);
            clippedPolygon[0] = c;
            intersect(c, a, &clippedPolygon[1]);
            intersect(c, b, &clippedPolygon[2]);
        }

        return 3;
    }
    else
    {
        CVF_ASSERT(numPositiveVertices == 2);
        if (onPosSide[0] && onPosSide[1])
        {
            // a & b are on positive side
            clippedPolygon[0] = a;
            clippedPolygon[1] = b;
            intersect(b, c, &clippedPolygon[2]);
            intersect(a, c, &clippedPolygon[3]);
        }
        else if (onPosSide[1] && onPosSide[2])
        {
            // b & c are on positive side
            clippedPolygon[0] = b;
            clippedPolygon[1] = c;
            intersect(c, a, &clippedPolygon[2]);
            intersect(b, a, &clippedPolygon[3]);
        }
        else
        {
            // c && a are on positive side
            CVF_ASSERT(onPosSide[2] && onPosSide[0]);
            clippedPolygon[0] = c;
            clippedPolygon[1] = a;
            intersect(a, b, &clippedPolygon[2]);
            intersect(c, b, &clippedPolygon[3]);
        }

        return 4;
    }
}


//--------------------------------------------------------------------------------------------------
/// Classify where the point is located relative to the plane
///
/// \return Plane::FRONT if the point is located on the side the plane normal is pointing\n
///         Plane::BACK if the point is located on the opposite side the plane normal is pointing\n
///         Plane::ON if the point is located in the plane
///         
//--------------------------------------------------------------------------------------------------
Plane::Side Plane::side(const Vec3d& point) const
{
    double d = distanceSquared(point);

    if (d > 0.0)
    {
        return FRONT;
    }
    else if (d < 0.0)
    {
        return BACK;
    }
    else
    {
        return ON;
    }
}


//--------------------------------------------------------------------------------------------------
/// Classify where the points are located relative to the plane
/// 
/// \param  points  Points to test for location relative the plane
///  
/// \return Plane::FRONT if points are either Plane::FRONT or Plane::ON\n
///         Plane::BACK if points are either Plane::BACK or Plane::ON\n
///         Plane::ON if all points are Plane::ON\n
///         Plane::BOTH if points are located on both sides
//--------------------------------------------------------------------------------------------------
Plane::Side Plane::side(const Vec3dArray& points) const
{
    // Code taken from
    // http://code.google.com/p/papervision3d/source/browse/trunk/as3/trunk/src/org/papervision3d/core/math/util/ClassificationUtil.as

    cvf::uint frontCount = 0;
    cvf::uint backCount = 0;

    for (size_t i = 0; i < points.size(); i++)
    {
        Side s = side(points[i]);

        if (s == FRONT)
        {
            frontCount++;
        }
        else if (s == BACK)
        {
            backCount++;
        }
    }
    
    if (frontCount > 0 && backCount == 0)
    {
        return FRONT;
    }
    else if (frontCount == 0 && backCount > 0)
    {
        return BACK;
    }
    else if (frontCount > 0 && backCount > 0)
    {
        return BOTH;
    }
    else
    {
        return ON;
    }
}


} // namespace cvf

