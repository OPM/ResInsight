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

#if !defined(WIN32) && !defined(CVF_LINUX) && !defined(CVF_IOS) && !defined(CVF_OSX) && !defined(CVF_ANDROID)
#error No platform defined
#endif

//  Global include file with definitions useful for all library files

// Disable some annoying warnings so we can compile with warning level Wall
#ifdef _MSC_VER
// 4512  'class' : assignment operator could not be generated : Due to problems with classes with reference member variables (e.g. VertexCompactor)
// 4514  unreferenced inline/local function has been removed
// 4625  copy constructor could not be generated because a base class copy constructor is inaccessible
// 4626  assignment operator could not be generated because a base class assignment operator is inaccessible
// 4640  'staticInstance' : construction of local static object is not thread-safe -> used by singletons. To be revisited.
// 4710  function 'func_name' not inlined
// 4711  function 'func_name' selected for automatic inline expansion
// 4738  storing 32-bit float result in memory, possible loss of performance
// 4820  'bytes' bytes padding added after construct 'member_name'
#pragma warning (disable: 4512 4514 4625 4626 4640 4710 4711 4738 4820)

#if (_MSC_VER >= 1600)
// VS2010 and newer
// 4986  'operator new[]': exception specification does not match previous declaration
#pragma warning (disable: 4986)
#endif

#endif


// Makes it easier to check on the current GCC version
#ifdef __GNUC__
// 40302 means version 4.3.2.
# define CVF_GCC_VER  (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__)
#endif  

// Helper macro to disable (ignore) compiler warnings on GCC
// The needed pragma is only available in GCC for versions 4.2.x and above
#if defined(__GNUC__) && (CVF_GCC_VER >= 40200)
#define CVF_DO_PRAGMA(x) _Pragma(#x)
#define CVF_GCC_DIAGNOSTIC_IGNORE(OPTION_STRING) CVF_DO_PRAGMA(GCC diagnostic ignored OPTION_STRING)
#else
#define CVF_GCC_DIAGNOSTIC_IGNORE(OPTION_STRING) 
#endif


#if defined(CVF_LINUX) || defined(CVF_IOS) || defined(CVF_OSX) || defined(CVF_ANDROID)
// Used by int64_t on *nix below
#include <cstdint>     
#endif

// Brings in size_t and definition of NULL
#include <cstddef>

// Added due to conflict between std::min/max and define in Windows.h
#define CVF_MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define CVF_MAX(X, Y) ((X) > (Y) ? (X) : (Y))

// Macro for avoiding "unused parameter" warnings
// The bottom one is the best alternative, but unfortunately doesn't work on VS2010
#ifdef WIN32
#define CVF_UNUSED(EXPR) (void)(EXPR);
#else
#define CVF_UNUSED(EXPR) (void)sizeof(EXPR); 
#endif


// Macro to disable the copy constructor and assignment operator
// Should be used in the private section of a class
#define CVF_DISABLE_COPY_AND_ASSIGN(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME&);              \
    void operator=(const CLASS_NAME&)


/// Ceetron Visualization Framework namespace
namespace cvf {

typedef unsigned char    ubyte;
typedef unsigned short   ushort;
typedef unsigned int     uint;

// 64bit integer support via the int64 type
#ifdef WIN32
typedef __int64 int64;  
#elif defined(CVF_LINUX) || defined(CVF_IOS) || defined(CVF_OSX) || defined(CVF_ANDROID)
typedef int64_t int64;  
#endif 

}

#include "cvfConfigCore.h"
#include "cvfVersion.h"
#include "cvfAssert.h"
