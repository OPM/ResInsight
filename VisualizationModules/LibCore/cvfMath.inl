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


namespace cvf {


//--------------------------------------------------------------------------------------------------
/// Check if a value is within the specified range, inclusive.
/// 
/// \return  Returns true if \a val >= \a min and \a val <= \a max.
//--------------------------------------------------------------------------------------------------
template<typename T> 
bool Math::valueInRange(T val, T minVal, T maxVal)
{
    CVF_ASSERT(minVal <= maxVal);

    if (val >= minVal && val <= maxVal)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Clamp a value within an inclusive range. 
/// 
/// \return The clamped value
//--------------------------------------------------------------------------------------------------
template<typename T> 
T Math::clamp(T val, T minVal, T maxVal)
{
    CVF_ASSERT(minVal <= maxVal);

    if (val >= minVal && val <= maxVal)
    {
        return val;
    }
    else if (val > maxVal)
    {
        return maxVal;
    }
    else
    {
        return minVal;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the absolute value of val
//--------------------------------------------------------------------------------------------------
template<typename T> 
T Math::abs(const T& val)
{ 
    return val >= 0 ? val : -val; 
}


//--------------------------------------------------------------------------------------------------
/// Returns the sign of val
//--------------------------------------------------------------------------------------------------
template<typename T> 
int Math::sign(const T& val)
{ 
    return val < 0 ? -1 : 1; 
}


}  // namespace cvf
