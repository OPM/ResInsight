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

#include "cvfPlane.h"
#include "cvfUniform.h"
#include "cvfDynamicUniformSet.h"

#include <vector>

namespace cvf {

//==================================================================================================
//
// ClipPlaneSet
//
//==================================================================================================
class ClipPlaneSet : public DynamicUniformSet
{
public:
    ClipPlaneSet();
    ~ClipPlaneSet();

    size_t              planeCount() const;
    void                addPlane(const Plane& plane);
    void                clear();

    virtual void        update(Rendering* rendering);
    virtual UniformSet* uniformSet();

private:
    std::vector<Plane>  m_wcPlanes;             // Planes in world coordinates
    ref<UniformInt>     m_planeCountUniform;    // Uniform holding the plane count
    ref<UniformFloat>   m_ecPlanesUniformArray; // Uniform array holding one vec4 for each plane
    ref<UniformSet>     m_combinedUniformSet;   // Uniform set containing the two uniforms
};

}
