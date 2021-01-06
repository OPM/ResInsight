/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafAssert.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
class RiaMedianCalculator
{
public:
    RiaMedianCalculator() = default;

    void   add( T value );
    double median() const;
    bool   valid() const;

private:
    std::vector<T> m_values;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
void RiaMedianCalculator<T>::add( T value )
{
    m_values.insert( std::lower_bound( m_values.begin(), m_values.end(), value ), value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
double RiaMedianCalculator<T>::median() const
{
    CAF_ASSERT( valid() && "You must ensure the result is valid before calling this" );

    auto count = m_values.size();
    if ( count == 1u )
        return m_values.front();
    else if ( count % 2 == 0 )
    {
        return ( m_values[count / 2 - 1] + m_values[count / 2] ) * 0.5;
    }
    else
    {
        return m_values[count / 2];
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <class T>
bool RiaMedianCalculator<T>::valid() const
{
    return !m_values.empty();
}
