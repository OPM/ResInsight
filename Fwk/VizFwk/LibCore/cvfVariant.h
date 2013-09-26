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

#include "cvfVector3.h"
#include "cvfColor3.h"
#include "cvfString.h"

#include <string>
#include <vector>

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class Variant
{
public:
    enum Type
    {
        INVALID,
        INT,
        UINT,
        DOUBLE,
        FLOAT,
        BOOL,
        VEC3D,
        COLOR3F,
        STRING,
        ARRAY
    };

public:
    Variant();
    Variant(const Variant& other);
    ~Variant();

    Variant(int val);
    Variant(uint val);
    Variant(double val);
    Variant(float val);
    Variant(bool val);
    Variant(const Vec3d& val);
    Variant(const Color3f& val);
    Variant(const String& val);
    Variant(const char* val);
    Variant(const std::vector<Variant>& arr);

    Variant&                operator=(Variant rhs);

    bool                    operator==(const Variant& rhs) const;

    Type                    type() const;
    bool                    isValid() const;

    int                     getInt() const;
    uint                    getUInt() const;
    double                  getDouble() const;
    float                   getFloat() const;
    bool                    getBool() const;
    Vec3d                   getVec3d() const;
    Color3f                 getColor3f() const;
    cvf::String             getString() const;
    std::vector<Variant>    getArray() const;

    void                    swap(Variant& other);

private:
    void                    assignData(const void* pointerToData, size_t dataSizeInBytes);

private:
    Type                    m_type;     // Type of for this variant
    std::vector<ubyte>      m_data;     // Data payload for single values
    std::vector<Variant>    m_arrayData;// Data payload when storing an array (of variants)
};

}  // namespace cvf
