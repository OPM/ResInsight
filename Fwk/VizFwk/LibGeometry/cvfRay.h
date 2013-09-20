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

#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfMatrix4.h"
#include "cvfString.h"

namespace cvf {

class BoundingBox;
class Plane;


//==================================================================================================
//
// Ray
//
//==================================================================================================
class Ray : public Object
{
public:
    Ray();
    Ray(const Ray& other);
    ~Ray();

    void            setOrigin(const Vec3d& orig);
    const Vec3d&    origin() const;
    void            setDirection(const Vec3d& dir);
    const Vec3d&    direction() const;

    void            setMinimumDistance(double distance);
    double          minimumDistance() const;
    void            setMaximumDistance(double distance);
    double          maximumDistance() const;

    void            transform(const Mat4d& matrix);
    const Ray       getTransformed(const Mat4d& matrix) const;

    bool            triangleIntersect(const Vec3d& v1, const Vec3d& v2, const Vec3d& v3, Vec3d* intersectionPoint = NULL) const;
    bool            quadIntersect(const Vec3d& v1, const Vec3d& v2, const Vec3d& v3, const Vec3d& v4, Vec3d* intersectionPoint = NULL) const;
    bool            boxIntersect(const BoundingBox& box, Vec3d* intersectionPoint = NULL) const;
    bool            planeIntersect(const Plane& plane, Vec3d* intersectionPoint = NULL) const;

    String          debugString() const;

private:
    Vec3d       m_origin;		        ///< Starting point of ray
    Vec3d		m_direction;		    ///< Vector specifying ray direction
    double      m_minDistance;          ///< Minimum distance for a hit
    double      m_maxDistance;          ///< Maximum distance for a hit
    double      m_minDistanceSquared;   
    double      m_maxDistanceSquared;   
    bool        m_distanceLimitedRay;
};

}
