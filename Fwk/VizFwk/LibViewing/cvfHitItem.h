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

namespace cvf {

class Part;
class HitDetail;


//==================================================================================================
//
// HitItem
//
//==================================================================================================
class HitItem : public Object
{
public:
    HitItem(double distanceAlongRay, const Vec3d& intersectionPoint);
    ~HitItem();

    double              distanceAlongRay() const;
    const Vec3d&        intersectionPoint() const;

    void                setPart(const Part* part);
    const Part*         part() const;
    void                setDetail(HitDetail* detail);
    const HitDetail*    detail() const;

private:
    double          m_distanceAlongRay;     // Distance from ray origin
    Vec3d           m_intersectionPoint;
    cref<Part>      m_part;                 // Reference to the part that was hit
    ref<HitDetail>  m_detail;               // Optional detail on the intersection
};


}
