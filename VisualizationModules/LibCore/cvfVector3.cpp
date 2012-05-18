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
#include "cvfVector3.h"

namespace cvf {


template<> Vector3<double> const Vector3<double>::UNDEFINED(UNDEFINED_DOUBLE, UNDEFINED_DOUBLE, UNDEFINED_DOUBLE);
template<> Vector3<float>  const Vector3<float>::UNDEFINED(UNDEFINED_FLOAT, UNDEFINED_FLOAT, UNDEFINED_FLOAT);
template<> Vector3<int>    const Vector3<int>::UNDEFINED(UNDEFINED_INT, UNDEFINED_INT, UNDEFINED_INT);
template<> Vector3<size_t> const Vector3<size_t>::UNDEFINED(UNDEFINED_SIZE_T, UNDEFINED_SIZE_T, UNDEFINED_SIZE_T);


} // namespace cvf
