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

#include "cvfDynamicUniformSet.h"
#include "cvfVector3.h"

namespace cvf {
    class UniformSet;
    class Transform;
}

namespace cvfu {


//==================================================================================================
//
// 
//
//==================================================================================================
class PointLight : public cvf::DynamicUniformSet
{
public:
    PointLight();
    ~PointLight();

    void                        setPosition(cvf::Vec3d position);
    cvf::Vec3d                  position() const;

    virtual void                update(cvf::Rendering* rendering);
    virtual cvf::UniformSet*    uniformSet();

    cvf::Transform*             markerTransform();

private:
    cvf::Vec3d                  m_position;
    cvf::ref<cvf::UniformSet>   m_uniformSet;
    cvf::ref<cvf::Transform>    m_markerTransform;
};


}


