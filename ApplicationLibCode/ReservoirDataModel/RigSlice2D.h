/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RigSlice2D
{
public:
    RigSlice2D( size_t nx, size_t ny );
    size_t nx() const;
    size_t ny() const;
    void   setValue( size_t x, size_t y, double value );
    double getValue( size_t x, size_t y ) const;

private:
    size_t getIndex( size_t x, size_t y ) const;

    std::vector<double> m_values;
    size_t              m_nx;
    size_t              m_ny;
};
