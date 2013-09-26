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
#include "cvfCodeLocation.h"

#include <algorithm>
#include <cstring>

namespace cvf {


//==================================================================================================
///
/// \class cvf::CodeLocation
/// \ingroup Core
///
/// Represents a source code location. 
///
/// Typically used with logging, asserts etc. Typically initialized using built-in compiler macros 
/// such as __FILE__ and __LINE__.
///
/// Note that the strings parameters for file name and function must be a static strings with a 
/// lifetime that's longer than the lifetime of the CodeLocation object
///
//==================================================================================================

static const char* const EMPTY_STRING = "";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CodeLocation::CodeLocation()
:   m_fileName(EMPTY_STRING),
    m_functionName(EMPTY_STRING),
    m_lineNumber(-1)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CodeLocation::CodeLocation(const char* fileName, const char* functionName, int lineNumber)
:   m_fileName(fileName),
    m_functionName(functionName),
    m_lineNumber(lineNumber)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CodeLocation::CodeLocation(const CodeLocation& other)
:   m_fileName(other.m_fileName),
    m_functionName(other.m_functionName),
    m_lineNumber(other.m_lineNumber)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const CodeLocation& CodeLocation::operator=(CodeLocation rhs)
{
    // Copy-and-swap (copy already done since parameter is passed by value)
    rhs.swap(*this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char* CodeLocation::fileName() const
{
    return m_fileName ? m_fileName : EMPTY_STRING;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char* CodeLocation::shortFileName() const
{
    if (m_fileName)
    {
        const char* ptrToLastSlash = strrchr(m_fileName, '/');

#ifdef WIN32
        const char* ptrToLastBwdSlash = strrchr(m_fileName, '\\');
        if (ptrToLastBwdSlash > ptrToLastSlash)
        {
            ptrToLastSlash = ptrToLastBwdSlash;
        }
#endif

        if (ptrToLastSlash)
        {
            return ptrToLastSlash + 1;
        }
        else
        {
            return m_fileName;
        }
    }
    else
    {
        return EMPTY_STRING;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char* CodeLocation::functionName() const
{
    return m_functionName ? m_functionName : EMPTY_STRING;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int CodeLocation::lineNumber() const
{
    return m_lineNumber;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CodeLocation::swap(CodeLocation& other)
{
    std::swap(m_fileName, other.m_fileName);
    std::swap(m_functionName, other.m_functionName);
    std::swap(m_lineNumber, other.m_lineNumber);
}


} // namespace cvf

