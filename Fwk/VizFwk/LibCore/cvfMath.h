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

#include "cvfBase.h"

namespace cvf {


//==================================================================================================
//
// Static class providing basic math operations
//
//==================================================================================================
class Math
{
public:
    static float            toRadians(float degrees);
    static double           toRadians(double degrees);
    static float            toDegrees(float radians);
    static double           toDegrees(double radians);

    static double           cos(double val);
    static float            cos(float val);
    static double           acos(double val);
    static float            acos(float val);
    static double           sin(double val);
    static float            sin(float val);
    static double           asin(double val);
    static float            asin(float val);
    static double           tan(double val);
    static float            tan(float val);
    static double           atan(double val);
    static float            atan(float val);

    static double           sqrt(double val);
    static float            sqrt(float val);

    static double           floor(double val);
    static float            floor(float val);
    static double           ceil(double val);
    static float            ceil(float val);
    static double           fmod(double x, double y);
    static float            fmod(float x, float y);
    static bool             isPow2(uint number);
    static uint             roundUpPow2(uint number);

    static bool             isUndefined(double val);
    static bool             isUndefined(float val);

    template<typename T> static bool     valueInRange(T val, T min, T max);
    template<typename T> static T        clamp(T val, T minVal, T maxVal);

    template<typename T> static inline T abs(const T& val);
    template<typename T> static int      sign(const T& val);
};


const float         PI_F                = 3.14159265f;
const double        PI_D                = 3.14159265358979323846;
const float         ONE_THIRD_F         = 1.0f/3.0f;
const double        ONE_THIRD_D         = 1.0/3.0;
const double        SQRT2_F             = 1.41421356f;              // sqrt(2)
const double        SQRT2_D             = 1.41421356237309504880;   // sqrt(2)
const double        SQRT1_2_F           = 0.70710678f;              // 1/sqrt(2)
const double        SQRT1_2_D           = 0.70710678118654752440;   // 1/sqrt(2)

const int		    UNDEFINED_INT	            = 2147483647;
const uint			UNDEFINED_UINT	            = static_cast<uint>(-1);  // 4294967295u
const size_t        UNDEFINED_SIZE_T            = static_cast<size_t>(-1);// 18446744073709551615u
const double        UNDEFINED_DOUBLE            = 1.7976931348623158e+308;
const double	    UNDEFINED_DOUBLE_THRESHOLD	= 1.00e+308;
const float         UNDEFINED_FLOAT             = 3.402823466e+38f;
const float	        UNDEFINED_FLOAT_THRESHOLD   = 2.99e+38f;



} // namespace cvf

#include "cvfMath.inl"
