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


#include "cvfLibCore.h"
#include <tchar.h>
#include <iostream>
#include <limits>
#include <windows.h>
#include <vector>
#include <iterator>

using namespace std;
using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String GetFullExecutablePath()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, sizeof(buffer));

    return String(buffer);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string ToUpper(const std::string& src)
{
	std::string result;
	std::transform( src.begin(), src.end(), std::back_inserter( result ), toupper);
	return result;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t FindStringNoCase(const std::string& str, const std::string& lookFor, size_t pos)
{
	return ToUpper(str).find(ToUpper(lookFor), pos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ReplaceAllOccurencesNoCase(std::string* str, const std::string& lookFor, const std::string& replaceWith)
{
    std::string::size_type n = 0;
    const std::string::size_type l = lookFor.length();
    for (;;)
    {
        n = FindStringNoCase(*str, lookFor, n);
        if (n != -1)
        {
            str->replace(n, l, replaceWith);
        }
        else
        {
            break;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool LauncProcessAndWait(const String& fullExecutablePath, DWORD* retExitCode)
{
    if (retExitCode) *retExitCode = EXIT_FAILURE;

    STARTUPINFO startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo;
    memset(&processInfo, 0, sizeof(processInfo));

    BOOL launchOK = CreateProcess(fullExecutablePath.c_str(), NULL, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo);
    if (!launchOK)
    {
        return false;
    }


    ::WaitForSingleObject(processInfo.hProcess, INFINITE);

    DWORD exitCode = 0;
    BOOL gotExitCode = GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    if (!gotExitCode || exitCode == STILL_ACTIVE)
    {
        return false;
    }
    if (gotExitCode && exitCode != STILL_ACTIVE)
    {
        if (retExitCode) *retExitCode = exitCode;
        return true;
    }
    else
    {
        return false;
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char **argv) 
{
	CVF_UNUSED(argc);
	CVF_UNUSED(argv);

    String myExecPath = GetFullExecutablePath();
    cout << endl;
    cout << "Running UnitTestLauncer" << endl;
    cout << "Full path: " << myExecPath.toStdString() << endl;
    cout << endl;


    // This is the list of unit tests that is to be run
    std::vector<String> unitTestList;
    unitTestList.push_back("LibCore_UnitTests");
    unitTestList.push_back("LibIo_UnitTests");
    unitTestList.push_back("LibGeometry_UnitTests");
    unitTestList.push_back("LibRender_UnitTests");
    unitTestList.push_back("LibViewing_UnitTests");
    unitTestList.push_back("LibUtilities_UnitTests");
    unitTestList.push_back("LibRegGrid2D_UnitTests");
    unitTestList.push_back("LibStructGrid_UnitTests");

    int numUnitTests = static_cast<int>(unitTestList.size());

    std::vector<bool>  launchResultList;
    std::vector<DWORD> exitCodeList;

    int i;
    for (i = 0; i < numUnitTests; i++)
    {
        launchResultList.push_back(false);
        exitCodeList.push_back(EXIT_FAILURE);
    }


    bool sawAnyProblems = false;

    for (i = 0; i < numUnitTests; i++)
    {
        const String unitTestName = unitTestList[i];

        
        // Replace all occurences of our own executable name with the name of the unit test
        String fullUnitTestPath;
        {
            std::string testPath = myExecPath.toStdString();
            ReplaceAllOccurencesNoCase(&testPath, "UnitTestLauncher", unitTestName.toStdString());
            fullUnitTestPath = testPath;
        }

        cout << endl;
        cout << endl;
        cout << "Launching: " << unitTestName.toStdString() << endl;
        cout << "  from:    " << fullUnitTestPath.toStdString() << endl;
        cout << endl;

        DWORD exitCode = EXIT_FAILURE;
        bool launchOK = LauncProcessAndWait(fullUnitTestPath, &exitCode);

        if (!launchOK || exitCode != EXIT_SUCCESS)
        {
            sawAnyProblems = true;
        }

        launchResultList[i] = launchOK;
        exitCodeList[i] = exitCode;

        //std::cin.get();
    }


    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (sawAnyProblems)
    {
        SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
    else
    {
        SetConsoleTextAttribute(stdout_handle, FOREGROUND_BLUE | FOREGROUND_GREEN| FOREGROUND_RED | FOREGROUND_INTENSITY);
    }

    cout << endl;
    cout << endl;
    cout << "SUMMARY" << endl;
    cout << "--------------------------------------------------------------" << endl;

    // Report aggregated status
    for (i = 0; i < numUnitTests; i++)
    {
        bool testOK = (launchResultList[i] && exitCodeList[i] == EXIT_SUCCESS);

        String statusStr = testOK ? " OK " : "FAIL";

        cout << "Staus: " << statusStr.toStdString() << "   " << unitTestList[i].toStdString() << endl;
    }

    cout << "--------------------------------------------------------------" << endl;
    cout << endl;

    if (sawAnyProblems)
    {
        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_SUCCESS;
    }
}
