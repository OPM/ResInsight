//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cvfPrimitiveTests.h"

#include <limits>

namespace cvf {



//==================================================================================================
///
/// \class cvf::PrimitiveTests
/// \ingroup Geometry
///
/// 
///
//==================================================================================================



//--------------------------------------------------------------------------------------------------
/// Calculate intersection between the lines p1p2 and p3p4
//--------------------------------------------------------------------------------------------------
bool PrimitiveTests::intersectLines(const Vec2d& p1, const Vec2d& p2, const Vec2d& p3, const Vec2d& p4, Vec2d* isect)
{
    // See Paul Bourke, Intersection point of two lines in 2 dimensions

    const double epsilon = std::numeric_limits<double>::epsilon();

    const double denom  = (p4.y()-p3.y())*(p2.x()-p1.x()) - (p4.x()-p3.x())*(p2.y()-p1.y());
    const double numera = (p4.x()-p3.x())*(p1.y()-p3.y()) - (p4.y()-p3.y())*(p1.x()-p3.x());
    const double numerb = (p2.x()-p1.x())*(p1.y()-p3.y()) - (p2.y()-p1.y())*(p1.x()-p3.x());
                                                    
    // Are the lines coincident? 
    if (cvf::Math::abs(numera) < epsilon && 
        cvf::Math::abs(numerb) < epsilon && 
        cvf::Math::abs(denom)  < epsilon) 
    {
        isect->x() = (p1.x() + p2.x()) / 2;
        isect->y() = (p1.y() + p2.y()) / 2;
        return true;
    }

    // Are the lines parallel?
    if (cvf::Math::abs(denom) < epsilon) 
    {
        isect->setZero();
        return false;
    }

    const double ta = numera/denom;
//     const double tb = numerb/denom;
// 
//     // Is the intersection along the the segments ?
//     if (ta < 0 || ta > 1 || tb < 0 || tb > 1) 
//     {
//         isect->setZero();
//         return false;
//     }

    isect->x() = p1.x() + ta * (p2.x() - p1.x());
    isect->y() = p1.y() + ta * (p2.y() - p1.y());

    return true;

}


} // namespace cvf
