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
#include "cvfValueArray.h"

#include <vector>

namespace cvf {


//==================================================================================================
//
// Templated array designed for high performance and no overhead over C arrays.
//
//==================================================================================================
template <typename T>
class Array : public Object, public ValueArray<T>
{
public:
    Array();
    Array(const Array& other);
    Array(const T* data, size_t size);
    explicit Array(size_t size);
    explicit Array(const ValueArray<T>& other);
    explicit Array(const std::vector<T>& other);
    ~Array();

    inline const T& operator[](size_t index) const;
    inline T&       operator[](size_t index);
    Array&          operator=(Array rhs);

    void                    assign(const T* data, size_t size);
    void                    assign(const std::vector<T>& data);
    void                    resize(size_t size);
    void                    clear();
    inline virtual size_t   size() const;

    inline void             set(size_t index, const T& val);
    inline void             setAll(const T& val);
    inline void             setConsecutive(const T& startVal);
    inline const T&         get(size_t index) const;
    inline virtual T        val(size_t index) const;

    inline const T* ptr() const;
    inline T*       ptr();
    inline const T* ptr(size_t index) const;
    inline T*       ptr(size_t index);

    void            setSharedPtr(T* data, size_t size);
    void            setPtr(T* data, size_t size);   

    void            copyData(const T* source, size_t numElementsToCopy, size_t destIndex);
    void            copyData(const Array<T>& source, size_t numElementsToCopy, size_t destIndex, size_t sourceIndex);

    template<typename U> 
    void            copyConvertedData(const Array<U>& source, size_t numElementsToCopy, size_t destIndex, size_t sourceIndex);

    template<typename U> 
    ref<Array<T> >  extractElements(const Array<U>& elementIndices) const;

    void            toStdVector(std::vector<T>* vec) const;

    size_t          capacity() const;        
    void            reserve(size_t capacity);
    void            squeeze();
    void            setSizeZero();
    inline void     add(const T& val);

    T               min(size_t* index = 0) const;
    T               max(size_t* index = 0) const;

    void            swap(Array& other);

    // Basic iterator support to be able to utilize STL algorithms
public:
    typedef T*          iterator;
    typedef const T*    const_iterator;

    inline iterator begin()             { return m_data; }
    inline iterator end()               { return m_data + m_size; }
    inline const_iterator begin() const { return m_data; }
    inline const_iterator end() const   { return m_data + m_size; }

private:
    void ground();

private:
    size_t  m_size;                 ///< Number of elements/items in array
    size_t  m_capacity;             ///< Size of the allocated buffer
    T*      m_data;                 ///< Array holding the actual data
    bool    m_sharedData;           ///< True if the data member is shared with another object and cannot be changed                
};


// Only works for simple types that don't require copy operators.
typedef Array<int>              IntArray;
typedef Array<uint>				UIntArray;
typedef Array<ushort>			UShortArray;
typedef Array<ubyte>			UByteArray;
typedef Array<float>            FloatArray;
typedef Array<double>           DoubleArray;
typedef Array<Vec2f>            Vec2fArray;
typedef Array<Vec2d>            Vec2dArray;
typedef Array<Vec3f>            Vec3fArray;
typedef Array<Vec3d>            Vec3dArray;
typedef Array<Vec4f>            Vec4fArray;
typedef Array<Vec4d>            Vec4dArray;
typedef Array<Color3ub>         Color3ubArray;
typedef Array<Color3f>          Color3fArray;
typedef Array<Color4ub>         Color4ubArray;

}

#include "cvfArray.inl"
