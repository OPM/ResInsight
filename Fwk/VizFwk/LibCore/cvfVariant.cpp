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
#include "cvfVariant.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::Variant
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant()
:   m_type(INVALID)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(const Variant& other)
:   m_type(other.m_type),
    m_data(other.m_data),
    m_arrayData(other.m_arrayData)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::~Variant()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant& Variant::operator=(Variant rhs)
{
    // Copy-and-swap (copy already done since parameter is passed by value)
    rhs.swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Compares two variants and returns true if they are equal
//--------------------------------------------------------------------------------------------------
bool Variant::operator==(const Variant& rhs) const
{
    if (m_type != rhs.m_type)
    {
        return false;
    }

    switch (m_type)
    {
        case INVALID:   return true;
        case INT:       return getInt() == rhs.getInt();
        case UINT:      return getUInt() == rhs.getUInt();
        case DOUBLE:    return getDouble() == rhs.getDouble();
        case FLOAT:     return getFloat() == rhs.getFloat();
        case BOOL:      return getBool() == rhs.getBool();
        case VEC3D:     return getVec3d() == rhs.getVec3d();
        case COLOR3F:   return getColor3f() == rhs.getColor3f();
        case STRING:    return getString() == rhs.getString();
        case ARRAY:     return getArray() == rhs.getArray();
    }

    CVF_FAIL_MSG("Unhandled variant type");
    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Type Variant::type() const
{
    return m_type;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Variant::isValid() const
{
    return (m_type != INVALID);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(int val)
:   m_type(INT)
{
    assignData(&val, sizeof(val));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int Variant::getInt() const
{
    CVF_ASSERT(m_type == INT);
    CVF_ASSERT(m_data.size() == sizeof(int));
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    if (rawPtr)
    {
        const int* valPtr = reinterpret_cast<const int*>(rawPtr);
        return *valPtr;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(uint val)
:   m_type(UINT)
{
    assignData(&val, sizeof(val));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint Variant::getUInt() const
{
    CVF_ASSERT(m_type == UINT);
    CVF_ASSERT(m_data.size() == sizeof(uint));
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    if (rawPtr)
    {
        const uint* valPtr = reinterpret_cast<const uint*>(rawPtr);
        return *valPtr;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(double val)
:   m_type(DOUBLE)
{
    assignData(&val, sizeof(val));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Variant::getDouble() const
{
    CVF_ASSERT(m_type == DOUBLE);
    CVF_ASSERT(m_data.size() == sizeof(double));
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    if (rawPtr)
    {
        const double* valPtr = reinterpret_cast<const double*>(rawPtr);
        return *valPtr;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(float val)
:   m_type(FLOAT)
{
    assignData(&val, sizeof(val));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Variant::getFloat() const
{
    CVF_ASSERT(m_type == FLOAT);
    CVF_ASSERT(m_data.size() == sizeof(float));
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    if (rawPtr)
    {
        const float* valPtr = reinterpret_cast<const float*>(rawPtr);
        return *valPtr;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(bool val)
:   m_type(BOOL)
{
    ubyte tmpVal = val ? 1u : 0;
    assignData(&tmpVal, 1);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Variant::getBool() const
{
    CVF_ASSERT(m_type == BOOL);
    CVF_ASSERT(m_data.size() == 1);
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    if (rawPtr)
    {
        if (*rawPtr == 1u)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(const Vec3d& val)
:   m_type(VEC3D)
{
    assignData(val.ptr(), 3*sizeof(double));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d Variant::getVec3d() const
{
    CVF_ASSERT(m_type == VEC3D);
    CVF_ASSERT(m_data.size() == 3*sizeof(double));
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    if (rawPtr)
    {
        const double* valPtr = reinterpret_cast<const double*>(rawPtr);
        return Vec3d(valPtr[0], valPtr[1], valPtr[2]);
    }
    else
    {
        return Vec3d(0, 0, 0);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(const Color3f& val)
:   m_type(COLOR3F)
{
    assignData(val.ptr(), 3*sizeof(float));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3f Variant::getColor3f() const
{
    CVF_ASSERT(m_type == COLOR3F);
    CVF_ASSERT(m_data.size() == 3*sizeof(float));
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    if (rawPtr)
    {
        const float* valPtr = reinterpret_cast<const float*>(rawPtr);
        return Color3f(valPtr[0], valPtr[1], valPtr[2]);
    }
    else
    {
        return Color3f(0, 0, 0);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(const String& val)
:   m_type(STRING)
{
    size_t strSize = val.size();
    if (strSize > 0)
    {
        assignData(val.c_str(), strSize*sizeof(wchar_t));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(const char* val)
:   m_type(STRING)
{
    String strVal(val);
    size_t strSize = strVal.size();
    if (strSize > 0)
    {
        assignData(strVal.c_str(), strSize*sizeof(wchar_t));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant::Variant(const std::vector<Variant>& arr)
:   m_type(ARRAY),
    m_arrayData(arr)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<Variant> Variant::getArray() const
{
    CVF_ASSERT(m_type == ARRAY);
    return m_arrayData;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Variant::getString() const
{
    CVF_ASSERT(m_type == STRING);

    // Array must contain at least one wchar_t
    const ubyte* rawPtr = m_data.empty() ? 0 : &m_data.front();
    const size_t numWideCars = m_data.size()/sizeof(wchar_t);
    if (rawPtr && numWideCars > 0)
    {
        const wchar_t* valPtr = reinterpret_cast<const wchar_t*>(rawPtr);
        std::wstring tmpWideString(valPtr, numWideCars);
        return String(tmpWideString);
    }
    else
    {
        return String();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Variant::swap(Variant& other)
{
    std::swap(m_type, other.m_type);    
    m_data.swap(other.m_data);
    m_arrayData.swap(other.m_arrayData);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Variant::assignData(const void* pointerToData, size_t dataSizeInBytes)
{
    CVF_ASSERT(m_arrayData.size() == 0);

    m_data.assign(reinterpret_cast<const ubyte*>(pointerToData), reinterpret_cast<const ubyte*>(pointerToData) + dataSizeInBytes);
    CVF_ASSERT(m_data.size() == dataSizeInBytes);
}


}  // namespace gc
