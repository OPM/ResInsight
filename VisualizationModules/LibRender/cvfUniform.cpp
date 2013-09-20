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
#include "cvfAssert.h"
#include "cvfUniform.h"
#include "cvfString.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Uniform
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Uniform::Uniform(const char* name)
{
    m_type = UNDEFINED;

    CVF_ASSERT(name);
    
    String s(name);
    CVF_ASSERT(!s.isEmpty());

    m_name = s.toAscii();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char* Uniform::name() const
{
    return m_name.ptr();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Uniform::Type Uniform::type() const
{
    return m_type;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const float* Uniform::floatPtr() const
{
    CVF_FAIL_MSG("Must be implemented in derived class");
    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const int* Uniform::intPtr() const
{
    CVF_FAIL_MSG("Must be implemented in derived class");
    return NULL;
}


//==================================================================================================
///
/// \class cvf::UniformInt
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformInt::UniformInt(const char* name)
:   Uniform(name)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformInt::UniformInt(const char* name, int value)
:   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformInt::set(int value)
{
    m_type = INT;
    m_data.resize(1);
    m_data[0] = value;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformInt::setArray(const IntArray& values)
{
    CVF_ASSERT(values.size() > 0);

    m_type = INT;
    m_data = values;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UniformInt::valueCount() const
{
    if (m_type == UNDEFINED)
    {
        return 0;
    }

    CVF_ASSERT(m_type == INT);

    return static_cast<int>(m_data.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const int* UniformInt::intPtr() const
{
    return m_data.ptr();
}



//==================================================================================================
///
/// \class cvf::UniformFloat
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformFloat::UniformFloat(const char* name)
:   Uniform(name)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformFloat::UniformFloat(const char* name, float value)
:   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformFloat::UniformFloat(const char* name, const Vec2f& value)
:   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformFloat::UniformFloat(const char* name, const Vec3f& value)
    :   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformFloat::UniformFloat(const char* name, const Vec4f& value)
    :   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformFloat::UniformFloat(const char* name, const Color3f& value)
    :   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformFloat::UniformFloat(const char* name, const Color4f& value)
    :   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::set(float value)
{
    m_type = FLOAT;
    m_data.resize(1);
    m_data[0] = value;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::set(const Vec2f& value)
{
    m_type = FLOAT_VEC2;
    m_data.resize(2);
    m_data[0] = value.x();
    m_data[1] = value.y();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::set(const Vec3f& value)
{
    m_type = FLOAT_VEC3;
    m_data.resize(3);
    m_data[0] = value.x();
    m_data[1] = value.y();
    m_data[2] = value.z();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::set(const Vec4f& value)
{
    m_type = FLOAT_VEC4;
    m_data.resize(4);
    m_data[0] = value.x();
    m_data[1] = value.y();
    m_data[2] = value.z();
    m_data[3] = value.w();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::set(const Color3f& value)
{
    m_type = FLOAT_VEC3;
    m_data.resize(3);
    m_data[0] = value.r();
    m_data[1] = value.g();
    m_data[2] = value.b();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::set(const Color4f& value)
{
    m_type = FLOAT_VEC4;
    m_data.resize(4);
    m_data[0] = value.r();
    m_data[1] = value.g();
    m_data[2] = value.b();
    m_data[3] = value.a();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::setArray(const FloatArray& values)
{
    CVF_ASSERT(values.size() > 0);

    m_type = FLOAT;
    m_data = values;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::setArray(const Vec3fArray& values)
{
    size_t numValues = values.size();
    CVF_ASSERT(numValues > 0);

    m_type = FLOAT_VEC3;
    m_data.resize(3*numValues);
    m_data.copyData(values.ptr()->ptr(), 3*numValues, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformFloat::setArray(const Vec4fArray& values)
{
    size_t numValues = values.size();
    CVF_ASSERT(numValues > 0);

    m_type = FLOAT_VEC4;
    m_data.resize(4*numValues);
    m_data.copyData(values.ptr()->ptr(), 4*numValues, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UniformFloat::valueCount() const
{
    switch (m_type)
    {
        case FLOAT:         return static_cast<int>(m_data.size());
        case FLOAT_VEC2:    return static_cast<int>(m_data.size())/2;
        case FLOAT_VEC3:    return static_cast<int>(m_data.size())/3;
        case FLOAT_VEC4:    return static_cast<int>(m_data.size())/4;

        case UNDEFINED:    
        case INT:    
        case FLOAT_MAT4:    break;
    }

    return 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const float* UniformFloat::floatPtr() const
{
    return m_data.ptr();
}



//==================================================================================================
///
/// \class cvf::UniformMatrixf
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformMatrixf::UniformMatrixf(const char* name)
:   Uniform(name)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
UniformMatrixf::UniformMatrixf(const char* name, const Mat4f& value)
:   Uniform(name)
{
    set(value);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void UniformMatrixf::set(const Mat4f& value)
{
    m_type = FLOAT_MAT4;
    m_data.resize(16);
    m_data.copyData(value.ptr(), 16, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int UniformMatrixf::valueCount() const
{
    if (m_type != FLOAT_MAT4)
    {
        return 0;
    }

    return static_cast<int>(m_data.size()/16);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const float* UniformMatrixf::floatPtr() const
{
    return m_data.ptr();
}


} // namespace cvf

