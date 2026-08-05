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

// Must be included so the assert macros see the default value of CVF_ENABLE_ASSERTS.
// Without this, a translation unit that reaches this header before cvfConfigCore.h would
// have CVF_ENABLE_ASSERTS undefined, silently compiling all asserts to no-ops.
#include "cvfConfigCore.h"

namespace cvf {

// Report a failed assert to the console and abort execution.
// On Windows GUI applications a console is created if one does not already exist.
[[noreturn]] void reportFailedAssert(const char* fileName, int lineNumber, const char* expr, const char* msg);

} // namespace cvf


// Define our assert macros
#if !defined(CVF_ENABLE_ASSERTS) || CVF_ENABLE_ASSERTS == 1
#   define CVF_ASSERT(expr)    (void)( (!!(expr)) || (cvf::reportFailedAssert(__FILE__, __LINE__, #expr, nullptr), 0) ) /* NOLINT */
#   define CVF_FAIL_MSG(msg)   cvf::reportFailedAssert(__FILE__, __LINE__, nullptr, (msg))
#else
#   define CVF_ASSERT(expr)    ((void)0)
#   define CVF_FAIL_MSG(msg)   ((void)0)
#endif

#if CVF_ENABLE_TIGHT_ASSERTS == 1 && ( !defined(CVF_ENABLE_ASSERTS) || CVF_ENABLE_ASSERTS == 1 )
#   define CVF_TIGHT_ASSERT(expr)  CVF_ASSERT(expr)
#else
#   define CVF_TIGHT_ASSERT(expr)  ((void)0)
#endif
