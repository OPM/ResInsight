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

#include <tchar.h>
#include <iostream>
#include <limits>
#include <windows.h>
#include <vector>



//==================================================================================================
//
// 
//
//==================================================================================================
class CodeFile
{
public:
    CodeFile();

    bool    readFile(const std::string& fileName);
    bool    writeFile(const std::string& fileName, const std::string& variableName);
    bool    appendToOpenFile(std::ofstream* file, const std::string& variableName);

    void    trimAndConvertToSpaces();
    void    removeBottomEmptyLines();
    void    padAndQuote();

private:
    size_t              maxLineLength();
    static std::string  trimRight(const std::string& str);
    static std::string  replaceAllOccurences(std::string str, const std::string& lookFor, const std::string& replaceWith);

private:
    std::vector<std::string> m_fileLines;
};


