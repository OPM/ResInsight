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
#include "cvfSystem.h"

#include <cstdlib>
#include <iostream>
#include <string>

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4668)
#include <windows.h>
#pragma warning (pop)
#include <io.h>
#include <signal.h>
#include <fcntl.h>
#endif

namespace cvf {


// User actions (interactive responses)
static const int USERACTION_CONTINUE    = 0;
static const int USERACTION_DEBUGBREAK  = 1;
static const int USERACTION_ABORT       = 2;



//==================================================================================================
///
/// \class cvf::AssertHandler
/// \ingroup Core
///
/// Base class for assert handlers
///
//==================================================================================================
class AssertHandler
{
public:
    virtual ~AssertHandler() {}
    virtual Assert::FailAction  handleAssert(const char* fileName, int lineNumber, const char* expr, const char* msg) = 0;
};



//==================================================================================================
///
/// \class cvf::AssertHandlerConsole
/// \ingroup Core
///
/// Assert handler for basic assert output to console
///
//==================================================================================================
class AssertHandlerConsole : public AssertHandler
{
public:
    virtual Assert::FailAction  handleAssert(const char* fileName, int lineNumber, const char* expr, const char* msg);

private:
    static void reportToConsole(const char* fileName, int lineNumber, const char* expr, const char* msg);
    static int  askForUserActionUsingConsole();
#ifdef WIN32
    static void winCreateConsoleAndRedirectIO(bool redirectInput);
#endif
};


//--------------------------------------------------------------------------------------------------
/// Show report of a failed assert in console and abort execution
/// 
/// On Windows, a console will be created if one doesn't exist (GUI applications)
//--------------------------------------------------------------------------------------------------
Assert::FailAction AssertHandlerConsole::handleAssert(const char* fileName, int lineNumber, const char* expr, const char* msg)
{
    // Just shows assert message in console.
    // Does the job on both Windows and Linux (creates a console on Windows if one doesn't exist)
    reportToConsole(fileName, lineNumber, expr, msg);

#ifdef _MSC_VER
#if (_MSC_VER >= 1600)
    if (::IsDebuggerPresent()) 
#endif
	{
        __debugbreak();
    }
#endif

    abort();

    // Shouldn't really matter since we always abort
    return Assert::CONTINUE;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AssertHandlerConsole::reportToConsole(const char* fileName, int lineNumber, const char* expr, const char* msg)
{
#ifdef WIN32
    // Make sure we have a console (applicable to Windows GUI applications)
    winCreateConsoleAndRedirectIO(false);
#endif

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

    std::cerr <<  " file " << fileName << ", line " << lineNumber << std::endl;
}


//--------------------------------------------------------------------------------------------------
/// Ask for user action using console input
/// 
/// \return  One of the USERACTION_ constants
//--------------------------------------------------------------------------------------------------
int AssertHandlerConsole::askForUserActionUsingConsole()
{
#ifdef WIN32
    // Make sure we have a console (applicable to Windows GUI applications)
    // Also ensures that input is redirected
    winCreateConsoleAndRedirectIO(true);
#endif

    // Let abort be the default choice
#ifdef WIN32
    std::cerr << "Choose action: [A]bort, [R]etry (debug) or [I]gnore: default [A]\n";
#else
    std::cerr << "Choose action: [A]bort or [I]gnore: default [A]\n";
#endif

    // Reset failstate, just in case.
    std::cin.clear();

    std::string line;
    while (std::getline(std::cin, line))
    {
        int ch = 0;
        if (!std::cin.fail() && line.length() == 1)
        {
            ch = tolower(line[0]);
        }

        if (ch == 'i')
        {
            return USERACTION_CONTINUE;
        }
#ifdef WIN32
        else if (ch == 'r')
        {
            return USERACTION_DEBUGBREAK;
        }
#endif
        else if (ch == 'a' || line.length() == 0)
        {
            return USERACTION_ABORT;
        }
    }

    return USERACTION_ABORT;
}


//--------------------------------------------------------------------------------------------------
/// Creates a console and redirects I/O from/to it (Windows only)
/// 
/// \param redirectInput  If true, input will also be redirected.
/// 
/// Function is useful for Windows GUI applications. Allocates a console if it doesn't exist and 
/// then redirects output to the console. Also redirects input if \a redirectInput is true
//--------------------------------------------------------------------------------------------------
#ifdef WIN32
void AssertHandlerConsole::winCreateConsoleAndRedirectIO(bool redirectInput)
{
    // Allocate a new console for this app
    // Only one console can be associated with an app, so should fail if a console is already present.
    AllocConsole();


    bool redirStdOut = true;
    bool redirStdErr = true;
    bool redirStdIn = redirectInput;

    if (redirStdOut)
    {
        HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
        FILE* fp = _fdopen(fileDescriptor, "w");

        *stdout = *fp;
        setvbuf(stdout, NULL, _IONBF, 0);
    }

    if (redirStdErr)
    {
        HANDLE stdHandle = GetStdHandle(STD_ERROR_HANDLE);
        int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
        FILE* fp = _fdopen(fileDescriptor, "w");

        *stderr = *fp;
        setvbuf(stderr, NULL, _IONBF, 0);
    }

    if (redirStdIn)
    {
        HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
        int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
        FILE* fp = _fdopen(fileDescriptor, "r");

        *stdin = *fp;
        setvbuf(stdin, NULL, _IONBF, 0);
    }


    // Make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    std::ios::sync_with_stdio();
}
#endif



//==================================================================================================
///
/// \class cvf::AssertHandlerWinDialog
/// \ingroup Core
///
/// Assert handler for Windows, using a dialog with interaction
///
//==================================================================================================
#ifdef WIN32
class AssertHandlerWinDialog : public AssertHandler
{
public:
    virtual Assert::FailAction  handleAssert(const char* fileName, int lineNumber, const char* expr, const char* msg);

private:
    static int  handleUsingDialog(const char* fileName, int lineNumber, const char* expr, const char* msg);
#ifdef _DEBUG
    static int  handleUsingCrtDbgReport(const char* fileName, int lineNumber, const char* expr, const char* msg);
#endif
};
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef WIN32
Assert::FailAction AssertHandlerWinDialog::handleAssert(const char* fileName, int lineNumber, const char* expr, const char* msg)
{
    //int retVal = handleUsingCrtDbgReport(fileName, lineNumber, expr, msg);
    int retVal = handleUsingDialog(fileName, lineNumber, expr, msg);

    if (retVal == USERACTION_CONTINUE)
    {
        return Assert::CONTINUE;
    }
    else if (retVal == USERACTION_DEBUGBREAK)
    {
        return Assert::DEBUGBREAK;
    }
    else if (retVal == USERACTION_ABORT)
    {
        // From __crtMessageWindow() in response to user clicking abort button
        // Note that it is better NOT to call abort() here, because the default implementation of abort() will call Watson
        // raise abort signal
        raise(SIGABRT);

        // We usually won't get here, but it's possible that SIGABRT was ignored.  
        // So exit the program anyway. 
        _exit(3);
    }

    // Shouldn't be getting here
    abort();
    return Assert::CONTINUE;
}
#endif


//--------------------------------------------------------------------------------------------------
/// Shows message in interactive dialog and lets user choose how to proceed
/// 
/// \return Returns one of the USERACTION_ constants depending on which button the user chooses.
/// 
/// Compared to the handleUsingCrtDbgReport() function, this function will also work in release builds
/// As opposed to the handleUsingCrtDbgReport(), this function will not call abort, but will return 
/// USERACTION_ABORT instead.
/// 
/// \todo  Must add code to handle case where new assert is triggered while handling an assert
//--------------------------------------------------------------------------------------------------
#ifdef WIN32
int AssertHandlerWinDialog::handleUsingDialog(const char* fileName, int lineNumber, const char* expr, const char* msg)
{
    char szMsgBuf[2048];

    System::strcpy(szMsgBuf, sizeof(szMsgBuf), "Assertion failed\n");

    System::strcat(szMsgBuf, sizeof(szMsgBuf), "\nFile: ");
    if (fileName)
    {
        System::strcat(szMsgBuf, sizeof(szMsgBuf), fileName);
    }

    System::strcat(szMsgBuf, sizeof(szMsgBuf), "\nLine: ");
    if (lineNumber >= 0)
    {
        char szLinNumBuf[20];
        System::sprintf(szLinNumBuf, sizeof(szLinNumBuf), "%d", lineNumber);
        System::strcat(szMsgBuf, sizeof(szMsgBuf), szLinNumBuf);
    }


    if (expr)
    {
        System::strcat(szMsgBuf, sizeof(szMsgBuf), "\n\n");
        System::strcat(szMsgBuf, sizeof(szMsgBuf), "Expression: ");
        System::strcat(szMsgBuf, sizeof(szMsgBuf), expr);
    }

    if (msg)
    {
        System::strcat(szMsgBuf, sizeof(szMsgBuf), "\n\n");
        System::strcat(szMsgBuf, sizeof(szMsgBuf), msg);
    }

    System::strcat(szMsgBuf, sizeof(szMsgBuf), "\n\n(Press Retry to debug application)");


    int retVal = ::MessageBoxA(NULL, szMsgBuf, "Assertion Failed", MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

    if      (retVal == IDIGNORE)    return USERACTION_CONTINUE;
    else if (retVal == IDRETRY)     return USERACTION_DEBUGBREAK;
    else                            return USERACTION_ABORT;
}
#endif


//--------------------------------------------------------------------------------------------------
/// Wrapper function for the CRT _CrtDbgReport() function 
/// 
/// This function never returns if the user chooses 'Abort'
/// Note that the underlying function is only available in debug builds
//--------------------------------------------------------------------------------------------------
#if defined WIN32 && defined _DEBUG
int AssertHandlerWinDialog::handleUsingCrtDbgReport(const char* fileName, int lineNumber, const char* expr, const char* msg)
{
    // Create message combining expression and message
    char szMsgBuf[2048];
    szMsgBuf[0] = '\0';

    if (expr)
    {
        System::strcat(szMsgBuf, sizeof(szMsgBuf), expr);
    }

    if (msg)
    {
        System::strcat(szMsgBuf, sizeof(szMsgBuf), "\n");
        System::strcat(szMsgBuf, sizeof(szMsgBuf), msg);
    }

    int retVal = _CrtDbgReport(_CRT_ASSERT, fileName, lineNumber, NULL, szMsgBuf);

    if      (retVal == 0)   return USERACTION_CONTINUE;
    else if (retVal == 1)   return USERACTION_DEBUGBREAK;
    else                    return USERACTION_ABORT;
}
#endif



//==================================================================================================
///
/// \class cvf::Assert
/// \ingroup Core
///
/// Helper class to customize assert 
/// 
//==================================================================================================

#ifdef WIN32
AssertHandler* Assert::sm_handler = new AssertHandlerWinDialog;
#else
AssertHandler* Assert::sm_handler = new AssertHandlerConsole;
#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Assert::setReportMode(ReportMode reportMode)
{
#ifndef WIN32
    if (reportMode == INTERACTIVE_DIALOG) return;
#endif

    delete sm_handler;
    sm_handler = NULL;

    if (reportMode == CONSOLE)
    {
        sm_handler = new AssertHandlerConsole;
    }
#ifdef WIN32
    else if (reportMode == INTERACTIVE_DIALOG)
    {
        sm_handler = new AssertHandlerWinDialog;
    }
#endif
}


//--------------------------------------------------------------------------------------------------
/// Show report of a failed assert
/// 
/// If assertions are configured to be interactive, a message box will be shown on Windows and
/// the user may opt to ignore or trigger debugging. 
/// On Linux, or if interactive asserts are disabled, the application will terminate.
/// 
/// \todo  Add handling of cases where we get another assert while processing the first one AND
///        asserts from multiple threads.
//--------------------------------------------------------------------------------------------------
Assert::FailAction Assert::reportFailedAssert(const char* fileName, int lineNumber, const char* expr, const char* msg)
{
    if (sm_handler)
    {
        return sm_handler->handleAssert(fileName, lineNumber, expr, msg);
    }
    else
    {
        abort();
        return CONTINUE;
    }
}


} // namespace cvf

