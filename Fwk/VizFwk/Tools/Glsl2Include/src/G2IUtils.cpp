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


#include "G2IUtils.h"

#include <windows.h>
#include <fstream>
#include <string>
#include <algorithm>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string G2IUtils::extractBaseName(const std::string& fileName)
{
    std::string baseName = fileName;
    size_t dotPos = baseName.rfind(".");
    if (dotPos != std::string::npos)
    {
        baseName.erase(dotPos, std::string::npos);
    }

    size_t slashPos = baseName.find_last_of("\\/");
    if (slashPos != std::string::npos)
    {
        baseName = baseName.substr(slashPos + 1, std::string::npos);
    }

    return baseName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> G2IUtils::getFilesInDirectorySorted(const std::string& directory, const std::string& filter)
{
    std::vector<std::string> fileList;

    std::string fileMask = directory + filter;

    WIN32_FIND_DATAA findData;
    HANDLE hFind = ::FindFirstFile(fileMask.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            fileList.push_back(std::string(findData.cFileName));
        }
        while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }

    std::sort(fileList.begin(), fileList.end(), compareStringsNoCase);

    return fileList;
}


//--------------------------------------------------------------------------------------------------
/// comparison, not case sensitive.
//--------------------------------------------------------------------------------------------------
bool G2IUtils::compareStringsNoCase(const std::string& first, const std::string& second)
{
    unsigned int i = 0;
    while ( (i < first.length()) && (i < second.length()) )
    {
        if      (tolower(first[i]) < tolower(second[i])) return true;
        else if (tolower(first[i]) > tolower(second[i])) return false;
        ++i;
    }
    
    if (first.length() < second.length()) return true;
    else                                  return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool G2IUtils::fileContensEqual(const std::string& fileName1, const std::string& fileName2)
{
    std::ifstream file1(fileName1);
    std::ifstream file2(fileName2);
    if (!file1.is_open() || !file2.is_open())
    {
        return false;
    }

    std::string line1;
    std::string line2;
    while (file1.good() && file2.good())
    {
        getline(file1, line1);
        getline(file2, line2);

        if (line1 != line2)
        {
            return false;
        }
    }

    if (file1.eof() && file2.eof())
    {
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
bool G2IUtils::copyFile(const std::string& srcFileName, const std::string& destFileName)
{
    char ch;

    std::ifstream srcFile(srcFileName);
    std::ofstream dstFile(destFileName);

    if (!srcFile || !dstFile)
    {
        return false;
    }


    while (srcFile && srcFile.get(ch)) 
    {
        dstFile.put(ch);
    }

    return true;
}