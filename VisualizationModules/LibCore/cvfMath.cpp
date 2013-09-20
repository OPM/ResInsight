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
#include "cvfMath.h"

#include <cmath>

namespace cvf {



//==================================================================================================
///
/// \class cvf::Math
/// \ingroup Core
///
/// Static class providing basic math operations
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
float Math::toRadians(float degrees)
{
    return degrees*PI_F/180.0f;
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
double Math::toRadians(double degrees)
{
    return degrees*PI_D/180.0;
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
float Math::toDegrees(float radians)
{
    return radians*180.0f/PI_F;
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
double Math::toDegrees(double radians)
{
    return radians*180.0/PI_D;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::cos(double val)
{
    return ::cos(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::cos(float val)
{
    return ::cosf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::acos(double val)
{
    return ::acos(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::acos(float val)
{
    return ::acosf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::sin(double val)
{
    return ::sin(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::sin(float val)
{
    return ::sinf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::asin(double val)
{
    return ::asin(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::asin(float val)
{
    return ::asinf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::tan(double val)
{
    return ::tan(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::tan(float val)
{
    return ::tanf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::atan(double val)
{
    return ::atan(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::atan(float val)
{
    return ::atanf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::sqrt(double val)
{
    return ::sqrt(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::sqrt(float val)
{
    return ::sqrtf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::floor(double val)
{
    return ::floor(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::floor(float val)
{
    return ::floorf(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Math::ceil(double val)
{
    return ::ceil(val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Math::ceil(float val)
{
    return ::ceilf(val);
}


//--------------------------------------------------------------------------------------------------
/// Returns the floating-point remainder of x / y
//--------------------------------------------------------------------------------------------------
double Math::fmod(double x, double y)
{
    return ::fmod(x, y);
}


//--------------------------------------------------------------------------------------------------
/// Returns the floating-point remainder of x / y
//--------------------------------------------------------------------------------------------------
float Math::fmod(float x, float y)
{
    return ::fmodf(x, y);
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the number is a power of 2
///
/// The number 0 is not considered a power of 2.
//--------------------------------------------------------------------------------------------------
bool Math::isPow2(uint number)
{
    // From Bit Twiddling Hacks
    // http://graphics.stanford.edu/~seander/bithacks.html

    return (number && !(number & (number - 1)));
}

//--------------------------------------------------------------------------------------------------
/// Round up to the next highest power of 2
///
/// If the number is already a power of 2 this function will return the same value
/// If number if out of range (greater than 2147483648) we cannot represent the next power of 2 and 0 is returned
//--------------------------------------------------------------------------------------------------
uint Math::roundUpPow2(uint number)
{
    // From Bit Twiddling Hacks
    // http://graphics.stanford.edu/~seander/bithacks.html

    if (number == 0)
    {
        // Handle edge case where number is 0, algo returns 0, which isn't a power of 2;
        return 1;
    }

    if (number > 2147483648u)
    {
        // Input is too large, we cannot represent the next power of two
        return 0;
    }

    number--;
    number |= number >> 1;
    number |= number >> 2;
    number |= number >> 4;
    number |= number >> 8;
    number |= number >> 16;
    number++;

    return number;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Math::isUndefined(double val)
{
    if (val < UNDEFINED_DOUBLE_THRESHOLD)
    {
        return false;
    }
    else
    {
        return true;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Math::isUndefined(float val)
{
    if (val < UNDEFINED_FLOAT_THRESHOLD)
    {
        return false;
    }
    else
    {
        return true;
    }
}

} // namespace cvf

