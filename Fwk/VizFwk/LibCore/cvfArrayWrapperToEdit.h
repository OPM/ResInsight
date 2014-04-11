//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

namespace cvf {

//--------------------------------------------------------------------------------------------------
/// 
/// \class cvf::ArrayWrapperToEdit
/// \ingroup Core
///
/// A wrapper class to make it possible to use different array types with 
/// different element types in the same algorithms. 
///
/// The implementation has a specialization for bare pointer arrays.
/// The reason for the bare pointer specialization is the [] access implementation 
/// which is different. (*array)[] vs array[]
///
/// The convenience functions wrapArrayToEdit() are available to simplify wrapping of your data making it 
/// possible to do: 
///    myFunction (wrapArrayToEdit(myNodeArray), wrapArrayToEdit(myIndexArray), ...); 
/// when calling a template function using ArrayWrapperToEdit's as input.
/// 
//--------------------------------------------------------------------------------------------------

template < typename ArrayType, typename ElmType >
class ArrayWrapperToEdit
{
public:
    ArrayWrapperToEdit(ArrayType* array, size_t size) : m_array(array), m_size(size) { }

    inline size_t size() const { return m_size; } 

    inline ElmType&        operator[](const size_t index)       { return (*m_array)[index]; }
    inline const ElmType&  operator[](const size_t index) const { return (*m_array)[index]; }

private:
    ArrayType * m_array;
    size_t m_size;
};

//--------------------------------------------------------------------------------------------------
///  Bare-pointer array wrapper specialization
//--------------------------------------------------------------------------------------------------

template < typename ElmType >
class ArrayWrapperToEdit <ElmType*, ElmType>
{
public:
    ArrayWrapperToEdit(ElmType* array, size_t size) : m_array(array), m_size(size) { }

    inline size_t size() const { return m_size; } 

    inline ElmType&        operator[](const size_t index)       { return m_array[index]; }
    inline const ElmType&  operator[](const size_t index) const { return m_array[index]; }

private:
    ElmType * m_array;
    size_t m_size;
};


#include "cvfArray.h"
#include <vector>

//--------------------------------------------------------------------------------------------------
/// cvf::Array specialization
//--------------------------------------------------------------------------------------------------
template <typename ElmType>
inline ArrayWrapperToEdit< cvf::Array<ElmType>, ElmType > wrapArrayToEdit(cvf::Array<ElmType>* array )
{
    ArrayWrapperToEdit<cvf::Array<ElmType>, ElmType> warr(array, array->size());
    return warr;
}

//--------------------------------------------------------------------------------------------------
/// std::vector specialization
//--------------------------------------------------------------------------------------------------

template <typename ElmType>
inline ArrayWrapperToEdit< std::vector<ElmType>, ElmType > wrapArrayToEdit(std::vector<ElmType>* array )
{
    ArrayWrapperToEdit<std::vector<ElmType>, ElmType> warr(array, array->size());
    return warr;
}

//--------------------------------------------------------------------------------------------------
/// Bare-pointer specialization
//--------------------------------------------------------------------------------------------------

template <typename ElmType>
inline ArrayWrapperToEdit< ElmType*, ElmType > wrapArrayToEdit(ElmType* array, size_t size )
{
    ArrayWrapperToEdit<ElmType*, ElmType> warr(array, size);
    return warr;
}

}

