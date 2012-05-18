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

#include "cvfString.h"
#include "cvfArray.h"


namespace cvf {

//==================================================================================================
//
// Base64 encoding and decoding for representing binary data in an ASCII string format by 
// translating it into a radix-64 representation.
//
//==================================================================================================
class Base64
{
public:
    static std::string encode(const cvf::UByteArray& data);
    static cvf::ref<cvf::UByteArray> decode(const std::string& encodedData);

};

}
