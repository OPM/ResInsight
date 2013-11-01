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
###########################" << std::endl;
    *file << "static const char " << variableName << "[] =";

    std::vector<std::string>::iterator it;
    for (it = m_fileLines.begin(); it != m_fileLines.end(); ++it)
    {
        std::string line = *it;
        *file << std::endl << line;
    }

    *file << ";" << std::endl;
    *file << std::endl;
    *file << std::endl;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t CodeFile::maxLineLength()
{
    size_t maxLen = 0;
    std::vector<std::string>::iterator it;
    for (it = m_fileLines.begin(); it != m_fileLines.end(); ++it)
    {
        size_t len = it->size();
        if (len > maxLen)
        {
            maxLen = len;
        }
    }

    return maxLen;
}


//--------------------------------------------------------------------------------------------------
/// Trim trailing whitespace
//--------------------------------------------------------------------------------------------------
std::string CodeFile::trimRight(const std::string& str)
{
    size_t endpos = str.find_last_not_of(" \t");
    if (std::string::npos != endpos)
    {
        std::string retString = str.substr(0, endpos + 1);
        return retString;
    }
    else
    {
        return str;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string CodeFile::replaceAllOccurences(std::string str, const std::string& lookFor, const std::string& replaceWith)
{
    std::string::size_type n = 0;
    const std::string::size_type l = lookFor.length();
    for (;;)
    {
        n = str.find(lookFor, n);
        if (n != -1)
        {
            str.replace(n, l, replaceWith);
        }
        else
        {
            break;
        }
    }

    return str;
}


