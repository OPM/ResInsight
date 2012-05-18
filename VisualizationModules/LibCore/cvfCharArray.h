//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include <vector>

namespace cvf {


//==================================================================================================
//
// 8 bit zero terminated char array
//
//==================================================================================================
class CharArray
{
public:
    CharArray();
    CharArray(size_t size, char c);
    explicit CharArray(const char* str);
    
    char&       operator[](size_t i);
    const char& operator[](size_t i) const;

    void        resize(size_t size);
    void        push_back(char c);
    size_t      size() const;

    const char* ptr() const;
    char*       ptr();

private:
    std::vector<char> m_data;
};

}
