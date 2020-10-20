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

#include "cvfArray.h"
#include "cvfMatrix4.h"
#include "cvfString.h"

namespace cvf {


//==================================================================================================
//
// Axis aligned bounding box
//
//==================================================================================================
class BoundingBox
{
public:
    BoundingBox();
    BoundingBox(const Vec3f& min, const Vec3f& max);
    BoundingBox(const Vec3d& min, const Vec3d& max);
    BoundingBox(const BoundingBox& other);

    BoundingBox& operator=(const BoundingBox& rhs);

    void reset();

    bool isValid() const;

    void add(const Vec3f& vertex);
    void add(const Vec3d& vertex);
    void add(const Vec3fArray& vertices);
    void add(const Vec3dArray& vertices);
    void add(const BoundingBox& bb);
    void addValid(const BoundingBox& bb);

    const Vec3d& min() const;
    const Vec3d& max() const;

    Vec3d       center() const;
    Vec3d       extent() const;
    double      radius() const;

    bool        contains(const Vec3d& point) const;
    bool        intersects(const BoundingBox& box) const;

    void        cornerVertices(Vec3d corners[8]) const;

    void                expand(double amount);
    void                transform(const Mat4d& matrix);
    const BoundingBox   getTransformed(const Mat4d& matrix) const;

    String              debugString() const;

private:
    Vec3d m_min;
    Vec3d m_max;
};

}
