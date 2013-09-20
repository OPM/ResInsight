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

// Forward of base class for assert handlers
class AssertHandler;


//==================================================================================================
//
// Helper class to customize assert behavior 
//
//==================================================================================================
class Assert
{
public:
    enum ReportMode
    {
        CONSOLE,                // Report asserts to console only and then abort. (default mode for Linux)
        INTERACTIVE_DIALOG      // Show message in dialog and ask for user action (Windows only, default mode on Windows)
    };

    enum FailAction
    {
        CONTINUE,   // Continue execution
        DEBUGBREAK  // Execution should be stopped and debugger should be triggered (currently windows only)
    };

public:
    static void         setReportMode(ReportMode reportMode);
    static FailAction   reportFailedAssert(const char* fileName, int lineNumber, const char* expr, const char* msg);

private:
    static AssertHandler*   sm_handler;
};

} // cvf



// Define to trigger debug trap for use with assert macros
// Currently only useful and in action on windows
#ifdef WIN32
#define CVF_DEBUGTRAP()  __debugbreak()
#else
#define CVF_DEBUGTRAP()  ((void)0)
#endif


// Define our assert macros
#if CVF_ENABLE_ASSERTS == 1
#   define CVF_ASSERT(expr)           (void)( (!!(expr)) || (cvf::Assert::CONTINUE == cvf::Assert::reportFailedAssert(__FILE__, __LINE__, #expr, NULL))  || (CVF_DEBUGTRAP(), 0) )
#   define CVF_ASSERT_MSG(expr, msg)  (void)( (!!(expr)) || (cvf::Assert::CONTINUE == cvf::Assert::reportFailedAssert(__FILE__, __LINE__, #expr, (msg))) || (CVF_DEBUGTRAP(), 0) )
#   define CVF_FAIL_MSG(msg)          (void)( (cvf::Assert::CONTINUE == cvf::Assert::reportFailedAssert(__FILE__, __LINE__, NULL, (msg))) || (CVF_DEBUGTRAP(), 0) )
#else 
#   define CVF_ASSERT(expr)           ((void)0)
#   define CVF_ASSERT_MSG(expr, msg)  ((void)0)
#   define CVF_FAIL_MSG(msg)          ((void)0)
#endif

#if CVF_ENABLE_TIGHT_ASSERTS == 1 && CVF_ENABLE_ASSERTS == 1
#   define CVF_TIGHT_ASSERT(expr)           CVF_ASSERT(expr)
#   define CVF_TIGHT_ASSERT_MSG(expr, msg)  CVF_ASSERT_MSG(expr, msg)
#else 
#   define CVF_TIGHT_ASSERT(expr)           ((void)0)
#   define CVF_TIGHT_ASSERT_MSG(expr, msg)  ((void)0)
#endif

