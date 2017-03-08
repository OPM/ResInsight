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

#include "cafAssert.h"


namespace caf
{

//==================================================================================================
/// A fixed array class. Used to create small fixed size index arrays typically
//==================================================================================================

template < typename T, size_t size >
class FixedArray
{
    T m_array[size];
public:

    const T* data() const { return m_array; }
    T*       data()       { return m_array; }

    FixedArray<T, size>& operator=(const T* ptr) { for (size_t i = 0; i < size ; ++i) m_array[i] = ptr[i]; return *this;} 

    template<typename IndexType>       T& operator[](const IndexType& index)       { CAF_ASSERT(static_cast<size_t>(index) < size); return m_array[index]; }
    template<typename IndexType> const T& operator[](const IndexType& index) const { CAF_ASSERT(static_cast<size_t>(index) < size); return m_array[index]; }
};

typedef FixedArray<int, 3>    IntArray3;
typedef FixedArray<int, 6>    IntArray6;
typedef FixedArray<int, 4>    IntArray4;
typedef FixedArray<int, 8>    IntArray8;
typedef FixedArray<unsigned int, 4>    UintArray4;
typedef FixedArray<unsigned int, 8>    UintArray8;
typedef FixedArray<size_t, 3> SizeTArray3;
typedef FixedArray<size_t, 4> SizeTArray4;
typedef FixedArray<size_t, 6> SizeTArray6;
typedef FixedArray<size_t, 8> SizeTArray8;

}
