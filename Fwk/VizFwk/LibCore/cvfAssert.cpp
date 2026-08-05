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
#include "cvfAssert.h"

#include <cstdlib>
#include <iostream>

namespace cvf {


//--------------------------------------------------------------------------------------------------
/// Report a failed assert to the console and abort execution.
///
/// abort() raises SIGABRT, which the application's signal handler turns into a full crash log
/// (message + stack trace) on both Windows and Linux. When a debugger is attached it breaks on
/// the abort as well, so no platform-specific handling is needed here.
//--------------------------------------------------------------------------------------------------
void reportFailedAssert(const char* fileName, int lineNumber, const char* expr, const char* msg)
{
    std::cerr << "Assertion failed:";

    if (expr)
    {
        std::cerr << " (" << expr << ")";
    }

    if (msg)
    {
        std::cerr << " '" << msg << "'";
    }

    if (expr || msg)
    {
        std::cerr << ",";
    }

    std::cerr << " file " << fileName << ", line " << lineNumber << std::endl;

    abort();
}


} // namespace cvf
