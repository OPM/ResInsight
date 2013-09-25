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

namespace cvf {


//==================================================================================================
//
// Static wrapper class for system functions
//
//==================================================================================================
class System
{
public:
    static bool     is64Bit();
    static bool     isBigEndian();

    static bool     memcpy(void* dst, size_t dstSizeInBytes, const void* src, size_t numBytesToCopy);
    static bool     strcpy(char* strDestination, size_t maxNumElementsInDestination, const char* strSource);
    static bool     strcat(char* strDestination, size_t maxNumElementsInDestination, const char* strSource);
    static int      sprintf(char* buffer, size_t maxNumElementsInBuffer, const char* format, ...);
    static int      swprintf(wchar_t* buffer, size_t maxNumElementsInBuffer, const wchar_t* format, ...);
    static size_t   strlen(const char* str);
    static int      strcmp(const char* str1, const char* str2);
};

}
