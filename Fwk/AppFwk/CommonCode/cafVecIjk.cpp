//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafVecIjk.h"

#include "cafAssert.h"

#include "cvfMath.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VecIjk::VecIjk( size_t i, size_t j, size_t k )
    : cvf::Vec3st( i, j, k )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string VecIjk::toString() const
{
    return std::to_string( i() ) + ", " + std::to_string( j() ) + ", " + std::to_string( k() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VecIjk0::VecIjk0( size_t i, size_t j, size_t k )
    : VecIjk( i, j, k )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VecIjk1 VecIjk0::toOneBased() const
{
    return VecIjk1( i() + 1, j() + 1, k() + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const VecIjk0 VecIjk0::ZERO( 0, 0, 0 );

const VecIjk0 VecIjk0::UNDEFINED( cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VecIjk1::VecIjk1( size_t i, size_t j, size_t k )
    : VecIjk( i, j, k )
{
    CAF_ASSERT( i > 0 );
    CAF_ASSERT( j > 0 );
    CAF_ASSERT( k > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VecIjk0 VecIjk1::toZeroBased() const
{
    return VecIjk0( i() - 1, j() - 1, k() - 1 );
}

}; // namespace caf
