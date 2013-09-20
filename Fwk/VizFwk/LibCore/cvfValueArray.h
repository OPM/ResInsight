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

#include "cvfVector4.h"
#include "cvfColor4.h"

namespace cvf {


//==================================================================================================
//
// Abstract base class for templated read-only array that returns elements by value
//
//==================================================================================================
template <typename T>
class ValueArray
{
public:
    virtual         ~ValueArray() {}
    virtual T       val(size_t index) const = 0;
    virtual size_t  size() const = 0;
};


typedef ValueArray<int>         IntValueArray;
typedef ValueArray<uint>	    UIntValueArray;
typedef ValueArray<ushort>	    UShortValueArray;
typedef ValueArray<ubyte>	    UByteValueArray;
typedef ValueArray<float>       FloatValueArray;
typedef ValueArray<double>      DoubleValueArray;
typedef ValueArray<Vec2f>       Vec2fValueArray;
typedef ValueArray<Vec3f>       Vec3fValueArray;
typedef ValueArray<Vec3d>       Vec3dValueArray;
typedef ValueArray<Color3ub>    Color3ubValueArray;
typedef ValueArray<Color3f>     Color3fValueArray;
typedef ValueArray<Color4ub>    Color4ubValueArray;

}


