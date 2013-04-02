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

#include "cvfVector4.h"
#include "cvfMatrix4.h"
#include "cvfArray.h"
#include "cvfCharArray.h"

namespace cvf {



//==================================================================================================
//
// 
//
//==================================================================================================
class Uniform : public Object
{
public:
    enum Type
    {
        UNDEFINED,
        INT,
        FLOAT, 
        FLOAT_VEC2, 
        FLOAT_VEC3, 
        FLOAT_VEC4, 
        FLOAT_MAT4 
    };

public:
    const char*             name() const;
    virtual Type            type() const;
    virtual int             valueCount() const = 0;
    virtual const float*    floatPtr() const;
    virtual const int*      intPtr() const;

protected:
    Uniform(const char* name);

protected:
    Type        m_type;

private:
    CharArray   m_name;
    CVF_DISABLE_COPY_AND_ASSIGN(Uniform);
};


//==================================================================================================
//
// 
//
//==================================================================================================
class UniformInt: public Uniform
{
public:
    UniformInt(const char* name);
    UniformInt(const char* name, int value);

    void                    set(int value);

    void                    setArray(const IntArray& values);

    virtual int             valueCount() const;
    virtual const int*      intPtr() const;

private:
    IntArray  m_data;
    CVF_DISABLE_COPY_AND_ASSIGN(UniformInt);
};


//==================================================================================================
//
// 
//
//==================================================================================================
class UniformFloat : public Uniform
{
public:
    UniformFloat(const char* name);
    UniformFloat(const char* name, float value);
    UniformFloat(const char* name, const Vec2f& value);
    UniformFloat(const char* name, const Vec3f& value);
    UniformFloat(const char* name, const Vec4f& value);
    UniformFloat(const char* name, const Color3f& value);
    UniformFloat(const char* name, const Color4f& value);

    void                    set(float value);
    void                    set(const Vec2f& value);
    void                    set(const Vec3f& value);
    void                    set(const Vec4f& value);
    void                    set(const Color3f& value);
    void                    set(const Color4f& value);

    void                    setArray(const FloatArray& values);
    void                    setArray(const Vec3fArray& values);
    void                    setArray(const Vec4fArray& values);

    virtual int             valueCount() const;
    virtual const float*    floatPtr() const;

private:
    FloatArray  m_data;
    CVF_DISABLE_COPY_AND_ASSIGN(UniformFloat);
};



//==================================================================================================
//
// 
//
//==================================================================================================
class UniformMatrixf : public Uniform
{
public:
    UniformMatrixf(const char* name);
    UniformMatrixf(const char* name, const Mat4f& value);

    void                    set(const Mat4f& value);

    virtual int             valueCount() const;
    virtual const float*    floatPtr() const;

private:
    FloatArray  m_data;
    CVF_DISABLE_COPY_AND_ASSIGN(UniformMatrixf);
};


}

