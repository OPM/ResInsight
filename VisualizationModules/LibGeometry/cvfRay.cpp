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
#include "cvfRay.h"
#include "cvfBoundingBox.h"
#include "cvfPlane.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Ray
/// \ingroup Geometry
///
/// A ray that can be used for intersection testing.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Ray::Ray()
:   m_origin(Vec3d::ZERO),
    m_direction(-Vec3d::Z_AXIS),
    m_minDistance(cvf::UNDEFINED_DOUBLE),
    m_maxDistance(cvf::UNDEFINED_DOUBLE),
    m_minDistanceSquared(cvf::UNDEFINED_DOUBLE),
    m_maxDistanceSquared(cvf::UNDEFINED_DOUBLE),
    m_distanceLimitedRay(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Ray::Ray(const Ray& other) : Object()
{
    m_origin = other.origin();
    m_direction = other.direction();
    m_minDistance = other.m_minDistance;
    m_maxDistance = other.m_maxDistance;
    m_minDistanceSquared = other.m_minDistanceSquared;
    m_maxDistanceSquared = other.m_maxDistanceSquared;
    m_distanceLimitedRay = other.m_distanceLimitedRay;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Ray::~Ray()
{
}


//--------------------------------------------------------------------------------------------------
/// Sets the origin (starting point) of the ray
//--------------------------------------------------------------------------------------------------
void Ray::setOrigin(const Vec3d& orig) 
{ 
    m_origin = orig; 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3d& Ray::origin() const 
{ 
    return m_origin; 
}


//--------------------------------------------------------------------------------------------------
/// Sets the direction of the ray
//--------------------------------------------------------------------------------------------------
void Ray::setDirection(const Vec3d& dir) 
{ 
    m_direction = dir; 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3d& Ray::direction() const 
{ 
    return m_direction; 
}


//--------------------------------------------------------------------------------------------------
/// Transforms the origin and direction with the given transformation matrix
//--------------------------------------------------------------------------------------------------
void Ray::transform(const Mat4d& matrix)
{
    m_origin.transformPoint(matrix);
    m_direction.transformVector(matrix);
}


//--------------------------------------------------------------------------------------------------
/// Returns this ray transformed with the given transformation matrix
//--------------------------------------------------------------------------------------------------
const Ray Ray::getTransformed(const Mat4d& matrix) const
{
    Ray ray(*this);
    ray.transform(matrix);

    return ray;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the ray intersects the triangle. 
/// 
/// intersectionPoint (if not NULL) will be set to the ray intersection point on the triangle.
//--------------------------------------------------------------------------------------------------
bool Ray::triangleIntersect(const Vec3d& v1, const Vec3d& v2, const Vec3d& v3, Vec3d* intersectionPoint) const
{
    Vec3d v12 = v2 - v1;
    Vec3d v13 = v3 - v1;

    Vec3d n = (v12 ^ v13).getNormalized();
    
    double det = n * direction();
    
    if (det == 0.0f)
    {
        return false;
    }

    double t = n * ((v1 - origin()) / det);
    
    if (t < 0)
    {
        return false;
    }

    Vec3d fp = origin() + direction()*t;
    Vec3d pts[] = { v1, v2, v3, v1 };
    
    int i;
    for(i = 0; i <  3; i++)
    {
        Vec3d bi_norm = -((pts[i+1]-pts[i]) ^ n).getNormalized();
        
        if (((fp-pts[i]) * bi_norm) < 0)
        {
            return false;
        }
    }

    // Distance filtering
    if (m_distanceLimitedRay)
    {
        double distanceSquared = origin().pointDistanceSquared(fp);

        if (m_minDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared < m_minDistanceSquared)
        {
            return false;   
        }

        if (m_maxDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared > m_maxDistanceSquared)
        {
            return false;   
        }
    }

    if (intersectionPoint) 
    {
        *intersectionPoint = fp;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the ray intersects the quad. 
/// 
/// intersectionPoint (if not NULL) will be set to the ray intersection point on the quad.
//--------------------------------------------------------------------------------------------------
bool Ray::quadIntersect(const Vec3d& v1, const Vec3d& v2, const Vec3d& v3, const Vec3d& v4, Vec3d* intersectionPoint) const
{
    Vec3d v12 = v2 - v1;
    Vec3d v13 = v3 - v1;

    Vec3d n = (v12 ^ v13).getNormalized();  // Quad Average normal

    double det = n * direction();  // Length of projection of Quad normal to ray direction

    if (det == 0.0f)
    {
        return false;  // Ray parallel to average quad plane. Not quite accurate if origin is close to quad 
    }

    double t = n * ((v1 - origin()) / det);

    if (t < 0)
    {
        return false;
    }

    Vec3d fp = origin() + direction()*t;
    Vec3d pts[] = { v1, v2, v3, v4, v1 };

    int i;
    for(i = 0; i <  4; i++)
    {
        Vec3d bi_norm = -((pts[i+1] - pts[i]) ^ n).getNormalized(); // - (Edge x Quad normal) : Unit Vector in quad plane from edge into the quad

        if (((fp - pts[i]) * bi_norm) < 0)
        {
            return false;
        }
    }

    // Distance filtering
    if (m_distanceLimitedRay)
    {
        double distanceSquared = origin().pointDistanceSquared(fp);

        if (m_minDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared < m_minDistanceSquared)
        {
            return false;   
        }

        if (m_maxDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared > m_maxDistanceSquared)
        {
            return false;   
        }
    }

    if (intersectionPoint) 
    {
        *intersectionPoint = fp;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the ray intersects the bounding box. 
/// 
/// intersectionPoint (if not NULL) will be set to the ray intersection point on the bounding box.
//--------------------------------------------------------------------------------------------------
bool Ray::boxIntersect(const BoundingBox& box, Vec3d* intersectionPoint) const
{
    if (!box.isValid()) return false;

    const int RIGHT = 0;
    const int LEFT = 1;
    const int MIDDLE = 2;


    // Find candidate planes; this loop can be avoided if rays cast all from the eye(assume perpsective view)
    bool inside = true;
    char quadrant[3];
    double candidatePlane[3];

    Vec3d min = box.min();
    Vec3d max = box.max();

    int i;
    for (i = 0; i < 3; i++)
    {
        if(m_origin[i] < min[i]) 
        {
            quadrant[i] = LEFT;
            candidatePlane[i] = min[i];
            inside = false;
        }
        else if (m_origin[i] > max[i]) 
        {
            quadrant[i] = RIGHT;
            candidatePlane[i] = max[i];
            inside = false;
        }
        else	
        {
            quadrant[i] = MIDDLE;
        }
    }

    // Ray origin inside bounding box
    if (inside)	
    {
        // Return intersection point
        if (intersectionPoint) *intersectionPoint = m_origin;

        return true;
    }

    // Calculate T distances to candidate planes
    double maxT[3];
    for (i = 0; i < 3; i++)
    {
        if (quadrant[i] != MIDDLE && m_direction[i] !=0.0f)
        {
            maxT[i] = (candidatePlane[i] - m_origin[i]) / m_direction[i];
        }
        else
        {
            maxT[i] = -1.0f;
        }
    }

    // Get largest of the maxT's for final choice of intersection
    int whichPlane = 0;
    for (i = 1; i < 3; i++)
    {
        if (maxT[whichPlane] < maxT[i]) whichPlane = i;
    }

    // Check final candidate actually inside box
    if (maxT[whichPlane] < 0.0f) return false;

    Vec3d hitPoint;

    for (i = 0; i < 3; i++)
    {
        if (whichPlane != i) 
        {
            hitPoint[i] = m_origin[i] + maxT[whichPlane] * m_direction[i];

            if (hitPoint[i] < min[i] || hitPoint[i] > max[i]) return false;
        } 
        else 
        {
            hitPoint[i] = candidatePlane[i];
        }
    }

    // Distance filtering
    if (m_distanceLimitedRay)
    {
        double distanceSquared = origin().pointDistanceSquared(hitPoint);

        if (m_minDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared < m_minDistanceSquared)
        {
            return false;   
        }

        if (m_maxDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared > m_maxDistanceSquared)
        {
            return false;   
        }
    }

    if (intersectionPoint)
    {
        *intersectionPoint = hitPoint;
    }

    // Ray hits box
    return true;				
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the ray intersects the ray
//--------------------------------------------------------------------------------------------------
bool Ray::planeIntersect(const Plane& plane, Vec3d* intersectionPoint) const
{
    Vec3d pn = plane.normal();
    double D = plane.D();

    double vd = pn*m_direction;

    // If vd > 0, the plane normal is pointing 'away' from the ray
    // We want the plane normal facing the ray so flip direction of the plane
    if (vd > 0) 
    {
        pn *= -1;
        D *= -1;
        vd *= -1;
    }

    // No intersection ray and plane normal are parallel 
    if (Math::abs(vd) > std::numeric_limits<double>::epsilon())
    {
        double v0 = -(pn*m_origin + D);
        double t = v0/vd;
        if (t >= 0)
        {
            // Distance filtering
            if (m_distanceLimitedRay)
            {
                double distanceSquared = origin().pointDistanceSquared(m_origin + t*m_direction);

                if (m_minDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared < m_minDistanceSquared)
                {
                    return false;   
                }

                if (m_maxDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD && distanceSquared > m_maxDistanceSquared)
                {
                    return false;   
                }
            }

            if (intersectionPoint)
            {
                *intersectionPoint = m_origin + t*m_direction;
            }

            return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Ray::debugString() const
{
    String str = "Ray: ";
    str += " origin: x=" + String::number(m_origin.x()) + " y=" + String::number(m_origin.y()) + " z=" + String::number(m_origin.z());
    str += " direction: x=" + String::number(m_direction.x()) + " y=" + String::number(m_direction.y()) + " z=" + String::number(m_direction.z());
    str += " minDist: " + String::number(m_minDistance);
    str += " maxDist: " + String::number(m_maxDistance);

    return str;
}


//--------------------------------------------------------------------------------------------------
/// Set the minimum distance (from the origin) that the ray will hit. 
/// 
/// Useful for limiting the ray when picking on models with clipping planes.
/// Set to tsv::UNDEFINED_DOUBLE
//--------------------------------------------------------------------------------------------------
void Ray::setMinimumDistance(double distance)
{
    m_minDistance = distance;

    if (m_minDistance >= 0 && m_minDistance < cvf::UNDEFINED_DOUBLE_THRESHOLD)
    {
        m_minDistanceSquared = m_minDistance*m_minDistance;
    }
    else
    {
        m_minDistanceSquared = cvf::UNDEFINED_DOUBLE;
    }

    m_distanceLimitedRay = (m_minDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD) || (m_maxDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD);
}


//--------------------------------------------------------------------------------------------------
/// Set the maximum distance (from the origin) that the ray will hit. 
/// 
/// Useful for limiting the ray when picking on models with clipping planes
//--------------------------------------------------------------------------------------------------
void Ray::setMaximumDistance(double distance)
{
    m_maxDistance = distance;

    if (m_maxDistance >= 0 && m_maxDistance < cvf::UNDEFINED_DOUBLE_THRESHOLD)
    {
        m_maxDistanceSquared = m_maxDistance*m_maxDistance;
    }
    else
    {
        m_maxDistanceSquared = cvf::UNDEFINED_DOUBLE;
    }

    m_distanceLimitedRay = (m_minDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD) || (m_maxDistanceSquared < cvf::UNDEFINED_DOUBLE_THRESHOLD);
}


//--------------------------------------------------------------------------------------------------
/// Minimum distance (from the origin) that the ray will hit. 
//--------------------------------------------------------------------------------------------------
double Ray::minimumDistance() const
{
    return m_minDistance;
}


//--------------------------------------------------------------------------------------------------
/// Maximum distance (from the origin) that the ray will hit
//--------------------------------------------------------------------------------------------------
double Ray::maximumDistance() const
{
    return m_maxDistance;
}

} // namespace cvf
