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

namespace cvf {


//==================================================================================================
///
/// \class cvf::FunctorRange
/// \ingroup Core
///
/// Implements an iteration range for use with our functors.
/// The class is designed to be compatible with TBB's blocked_range class with regards to 
/// the functor's requirements
/// 
//==================================================================================================
template <typename T>
class FunctorRange
{
public:
    FunctorRange(T beginIndex, T endIndex) 
        : m_beginIndex(beginIndex), m_endIndex(endIndex) {}

    inline T begin() const { return m_beginIndex; }  ///< The first index included in the range
    inline T end() const   { return m_endIndex; }    ///< Index one past last the last index to be included in the range

private:
    T m_beginIndex;    // The first index to be included in the iteration.
    T m_endIndex;      // The index one past the last index to be included in the iteration
};


} // cvf


