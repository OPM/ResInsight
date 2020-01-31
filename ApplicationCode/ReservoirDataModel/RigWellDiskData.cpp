/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RigWellDiskData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellDiskData::RigWellDiskData()
    : m_isSingleProperty( false )
    , m_singlePropertyValue( 0.0 )
    , m_oilValue( 0.0 )
    , m_waterValue( 0.0 )
    , m_gasValue( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellDiskData::setSinglePropertyValue( double value )
{
    m_isSingleProperty    = true;
    m_singlePropertyValue = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellDiskData::setOilGasWater( double oil, double gas, double water )
{
    m_isSingleProperty = false;

    m_oilValue   = oil;
    m_gasValue   = gas;
    m_waterValue = water;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellDiskData::total() const
{
    if ( m_isSingleProperty )
    {
        return m_singlePropertyValue;
    }
    else
    {
        return m_oilValue + m_gasValue + m_waterValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellDiskData::oil() const
{
    return m_oilValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellDiskData::gas() const
{
    return m_gasValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellDiskData::water() const
{
    return m_waterValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellDiskData::singlePropertyValue() const
{
    return m_singlePropertyValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellDiskData::isSingleProperty() const
{
    return m_isSingleProperty;
}
