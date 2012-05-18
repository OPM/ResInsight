//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfObject.h"
#include "cvfPlane.h"

#include <map>

namespace cvf {

class BoundingBox;


//=================================================================================================
// 
// Frustum class
// 
//=================================================================================================
class Frustum : public Object
{
public:
    /// Sides of the frustum
    enum Side
    {
        BOTTOM = 0, // Index based -> Must start with zero
        TOP,
        LEFT,
        RIGHT,
        FRONT,
        BACK,
        COUNT   ///< Number of sides
    };

public:
    Frustum();
    Frustum(const Frustum& other);
    ~Frustum();

    const   Frustum& operator=(const Frustum& other);
    bool    operator==(const Frustum& other) const;
    bool    operator!=(const Frustum& other) const;

    void    setPlane(Side side, const Plane& plane);
    Plane   plane(Side side) const;

    void    transform(const Mat4d& matrix);

    bool    isOutside(const Vec3d& point) const;
    bool    isOutside(const BoundingBox& bbox) const;

private:
    std::map<int, Plane> m_planes;
};

}
