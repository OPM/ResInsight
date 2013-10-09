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
#include "cvfSystem.h"
#include "cvfString.h"

#include <memory>
#include <cstring>
#include <cstdio>
#include <cstdarg>

namespace cvf {



//==================================================================================================
///
/// \class cvf::System
/// \ingroup Core
///
/// Static wrapper class for system functions
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Check if we're running 64bit 
//--------------------------------------------------------------------------------------------------
bool System::is64Bit()
{
    return (sizeof(void*) == 8) ? true : false;
}


//--------------------------------------------------------------------------------------------------
/// Check if we are running on a big endian system.
/// 
/// \return  Returns true if we're on big endian, false for little endian.
//--------------------------------------------------------------------------------------------------
bool System::isBigEndian()
{
    int iInt = 1;
    char* pcChar = reinterpret_cast<char*>(&iInt);

    return !(*pcChar);
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
bool System::memcpy(void* dst, size_t dstSizeInBytes, const void* src, size_t numBytesToCopy)
{
#if WIN32

    // Debug version asserts
    errno_t err = memcpy_s(dst, dstSizeInBytes, src, numBytesToCopy);
    if (err == 0)
    {
        return true;
    }
    else
    {
        return false;
    }

#else

    if (!dst || dstSizeInBytes == 0 || !src)
    {
        // Be consistent with the behavior of memcpy_s.
        return false;
    }

    if (numBytesToCopy > dstSizeInBytes)
    {
        // The source memory is too large to copy to the destination.  
        // To be consistent with memcpy_s, return false to indicate failure
        return false;
    }

    ::memcpy(dst, src, numBytesToCopy);

    return true;

#endif
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
bool System::strcpy(char* strDestination, size_t maxNumElementsInDestination, const char* strSource)
{
#ifdef WIN32

    // Debug version asserts
    // Note that on windows the debug versions functions first fills the buffer with 0xFD
    errno_t err = strcpy_s(strDestination, maxNumElementsInDestination, strSource);
    if (err == 0)
    {
        return true;
    }
    else
    {
        return false;
    }

#else

    CVF_UNUSED(maxNumElementsInDestination);
    ::strcpy(strDestination, strSource);

    return true;

#endif
}


//--------------------------------------------------------------------------------------------------
/// Append a string 
/// 
/// \param  strDestination               Null-terminated destination string buffer
/// \param  maxNumElementsInDestination  Total size of the destination string buffer in characters
/// \param  strSource                    Null-terminated source string buffer
//--------------------------------------------------------------------------------------------------
bool System::strcat(char* strDestination, size_t maxNumElementsInDestination, const char* strSource)
{
#ifdef WIN32

    errno_t err = strcat_s(strDestination, maxNumElementsInDestination, strSource);
    if (err == 0)
    {
        return true;
    }
    else
    {
        return false;
    }

#else

    if (!strDestination || maxNumElementsInDestination == 0 || !strSource)
    {
        // Be consistent with the behavior of strcat_s.
        return false;
    }

    size_t dstLen = strlen(strDestination);
    size_t srcLen = strlen(strSource);
    size_t sumLen = srcLen + dstLen;
    if (sumLen + 1 > maxNumElementsInDestination)
    {
        return false;
    }

    strncat(strDestination, strSource, srcLen);
    strDestination[sumLen] = 0;

    return true;

#endif
}


//--------------------------------------------------------------------------------------------------
/// Wrapper for sprintf
/// 
/// \param buffer                  The buffer to write into
/// \param maxNumElementsInBuffer  Size of the buffer in number of characters
/// \param format                  Format string
/// 
/// \return Number of character written into buffer, excluding the terminating NULL character.
///         If the function fails the return is -1
/// 
/// Not that at most maxNumElementsInBuffer - 1 characters are ever written into the buffer since
/// a termination character is always written to the last position in the buffer.
//--------------------------------------------------------------------------------------------------
int System::sprintf(char* buffer, size_t maxNumElementsInBuffer, const char* format, ...)
{
    if (!buffer || maxNumElementsInBuffer == 0 || !format)
    {
        return -1;
    }

    va_list argList;
    va_start(argList, format);

#ifdef WIN32

    int numWritten = vsnprintf_s(buffer, maxNumElementsInBuffer*sizeof(char), maxNumElementsInBuffer - 1, format, argList);

#else

    int numWritten = vsnprintf(buffer, maxNumElementsInBuffer, format, argList);
    
    // Linux will fill the buffer without adding a NULL
    // Catch this and report as error
    if (numWritten == static_cast<int>(maxNumElementsInBuffer))
    {
        numWritten = -1;
    }

#endif

    va_end(argList);

    // Ensure buffer is NULL terminated
    buffer[maxNumElementsInBuffer - 1] = '\0';

    return numWritten;
}


//--------------------------------------------------------------------------------------------------
/// Wrapper for sprintf
/// 
/// \param buffer                  The buffer to write into
/// \param maxNumElementsInBuffer  Size of the buffer in number of wide characters (not bytes)
/// \param format                  Format string
/// 
/// \return Number of wide character written into buffer, excluding the terminating NULL character.
///         If the function fails the return is -1
/// 
/// Not that at most maxNumElementsInBuffer - 1 characters are ever written into the buffer since
/// a termination character is always written to the last position in the buffer.
//--------------------------------------------------------------------------------------------------
int System::swprintf(wchar_t* buffer, size_t maxNumElementsInBuffer, const wchar_t* format, ...)
{
    if (!buffer || maxNumElementsInBuffer == 0 || !format)
    {
        return -1;
    }

    va_list argList;
    va_start(argList, format);

#ifdef WIN32
    int numWritten = _vsnwprintf_s(buffer, maxNumElementsInBuffer, maxNumElementsInBuffer - 1, format, argList);
#else
    int numWritten = vswprintf(buffer, maxNumElementsInBuffer, format, argList);
#endif

    va_end(argList);

    buffer[maxNumElementsInBuffer - 1] = L'\0';

    return numWritten;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t System::strlen(const char* str)
{
    if (str)
    {
        return ::strlen(str);
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int System::strcmp(const char* str1, const char* str2)
{
    if (str1 == NULL || str2 == NULL)
    {
        if (str1 == NULL && str2 == NULL)
        {
            return 0;
        }

        if (str1 == NULL)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return ::strcmp(str1, str2);
    }
}


} // namespace cvf

