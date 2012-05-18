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

#include "cvfBase.h"
#include "cvfVector4.h"
#include "cvfMath.h"

namespace cvf {


template<> Vector4<double> const Vector4<double>::UNDEFINED(UNDEFINED_DOUBLE, UNDEFINED_DOUBLE, UNDEFINED_DOUBLE, UNDEFINED_DOUBLE);
template<> Vector4<float>  const Vector4<float>::UNDEFINED(UNDEFINED_FLOAT, UNDEFINED_FLOAT, UNDEFINED_FLOAT, UNDEFINED_FLOAT);
template<> Vector4<int>    const Vector4<int>::UNDEFINED(UNDEFINED_INT, UNDEFINED_INT, UNDEFINED_INT, UNDEFINED_INT);


} // namespace cvf
