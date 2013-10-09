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
#include "cvfCharArray.h"
#include "cvfSystem.h"

#include <sstream>
#include <iomanip>

namespace cvf {



//==================================================================================================
///
/// \class cvf::CharArray
/// \ingroup Core
///
/// 8 bit zero terminated char array.
/// 
/// Behind the scenes, the array always has a 0 termination sentinel.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CharArray::CharArray()
{
    m_data.push_back('\0');
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CharArray::CharArray(size_t size, char c)
{
    resize(size);

    size_t i;
    for (i = 0; i < size; i++)
    {
        m_data[i] = c;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CharArray::CharArray(const char* str)
{
    size_t numChars = System::strlen(str);
    resize(numChars);

    size_t i;
    for (i = 0; i < numChars; i++)
    {
        m_data[i] = str[i];
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
char& CharArray::operator[](size_t i) 
{
    CVF_TIGHT_ASSERT(i < m_data.size() - 1);

    return m_data[i];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char& CharArray::operator[](size_t i) const
{
    CVF_TIGHT_ASSERT(i < m_data.size() - 1);

    return m_data[i];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CharArray::resize(size_t size)
{
    m_data.resize(size + 1, '\0');
    m_data[size] = '\0';
}


//--------------------------------------------------------------------------------------------------
/// Add a character to the array. Array grows if necessary.
//--------------------------------------------------------------------------------------------------
void CharArray::push_back(char c)
{
    CVF_TIGHT_ASSERT(m_data.size() > 0);

    m_data[m_data.size() - 1] = c;
    m_data.push_back('\0');
}


//--------------------------------------------------------------------------------------------------
/// Get the size of the character array.
/// 
/// Note that this is not necessarily the length of the contained string. In order to get the string
/// length, you must call System::strlen().
//--------------------------------------------------------------------------------------------------
size_t CharArray::size() const
{ 
    CVF_TIGHT_ASSERT(m_data.size() > 0);

    return m_data.size() - 1;
}   



//--------------------------------------------------------------------------------------------------
/// Get a const char* pointer to the string
//--------------------------------------------------------------------------------------------------
const char* CharArray::ptr() const
{ 
    CVF_ASSERT(m_data.size() > 0);
    CVF_ASSERT(m_data[m_data.size() - 1] == '\0');
    
    return &m_data[0]; 
}


//--------------------------------------------------------------------------------------------------
/// Get a const char* pointer to the string
//--------------------------------------------------------------------------------------------------
char* CharArray::ptr()
{ 
    CVF_ASSERT(m_data.size() > 0);
    CVF_ASSERT(m_data[m_data.size() - 1] == '\0');

    return &m_data[0]; 
}


} // namespace cvf

